#include "obielf/resource_pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    obielf_resource_pack_t pack;
    obielf_resource_view_t apple;
    obielf_resource_view_t missing;
    obielf_status_t status;
    uint8_t *corrupted;
    size_t apple_offset;
    int passed = 1;

    if (argc != 2) {
        fprintf(stderr, "usage: resource-pack-test FILE\n");
        return EXIT_FAILURE;
    }

    status = obielf_resource_pack_open_file(&pack, argv[1]);
    passed &= require(status == OBIELF_OK, "pack opens");
    if (status != OBIELF_OK) {
        return EXIT_FAILURE;
    }
    passed &= require(pack.entry_count == 25u, "expected 25 resources");
    passed &= require(
        obielf_resource_pack_verify(&pack) == OBIELF_OK,
        "all payload checksums verify");
    passed &= require(
        obielf_resource_pack_find(
            &pack, "apple_small.png", &apple) == OBIELF_OK,
        "apple resource found");
    passed &= require(
        apple.size > 8u && memcmp(apple.data, "\x89PNG\r\n\x1a\n", 8u) == 0,
        "apple payload is PNG");
    passed &= require(
        obielf_resource_pack_find(
            &pack, "not-present.png", &missing) ==
            OBIELF_ERROR_NOT_FOUND,
        "missing resource returns typed error");

    apple_offset = (size_t)(apple.data - pack.bytes);
    corrupted = (uint8_t *)malloc(pack.size);
    passed &= require(corrupted != NULL, "corruption buffer allocated");
    if (corrupted != NULL) {
        obielf_resource_pack_t corrupted_pack;
        memcpy(corrupted, pack.bytes, pack.size);
        corrupted[apple_offset] ^= UINT8_C(0xff);
        status = obielf_resource_pack_open_memory(
            &corrupted_pack, corrupted, pack.size);
        passed &= require(
            status == OBIELF_OK,
            "payload corruption leaves structural metadata readable");
        if (status == OBIELF_OK) {
            passed &= require(
                obielf_resource_pack_verify(&corrupted_pack) ==
                    OBIELF_ERROR_CHECKSUM,
                "payload corruption is detected");
            obielf_resource_pack_close(&corrupted_pack);
        }
        free(corrupted);
    }

    obielf_resource_pack_close(&pack);
    if (passed) {
        puts("resource pack conformance: PASS");
    }
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}

