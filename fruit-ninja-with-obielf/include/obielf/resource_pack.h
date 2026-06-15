#ifndef OBIELF_RESOURCE_PACK_H
#define OBIELF_RESOURCE_PACK_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OBIELF_RESOURCE_MAGIC "OBIRPACK"
#define OBIELF_RESOURCE_VERSION 1u
#define OBIELF_RESOURCE_HEADER_SIZE 64u
#define OBIELF_RESOURCE_ENTRY_SIZE 96u
#define OBIELF_RESOURCE_NAME_CAPACITY 64u

typedef enum obielf_status {
    OBIELF_OK = 0,
    OBIELF_ERROR_ARGUMENT,
    OBIELF_ERROR_IO,
    OBIELF_ERROR_MEMORY,
    OBIELF_ERROR_MAGIC,
    OBIELF_ERROR_VERSION,
    OBIELF_ERROR_FORMAT,
    OBIELF_ERROR_BOUNDS,
    OBIELF_ERROR_CHECKSUM,
    OBIELF_ERROR_NOT_FOUND
} obielf_status_t;

typedef enum obielf_resource_kind {
    OBIELF_RESOURCE_BINARY = 0,
    OBIELF_RESOURCE_PNG = 1
} obielf_resource_kind_t;

typedef struct obielf_resource_pack {
    const uint8_t *bytes;
    size_t size;
    uint8_t *owned_bytes;
    uint32_t entry_count;
    uint64_t directory_offset;
    uint64_t data_offset;
} obielf_resource_pack_t;

typedef struct obielf_resource_view {
    char name[OBIELF_RESOURCE_NAME_CAPACITY + 1u];
    const uint8_t *data;
    size_t size;
    uint32_t checksum;
    uint16_t kind;
    uint16_t flags;
} obielf_resource_view_t;

uint32_t obielf_crc32(const void *data, size_t size);

obielf_status_t obielf_resource_pack_open_memory(
    obielf_resource_pack_t *pack,
    const void *bytes,
    size_t size);

obielf_status_t obielf_resource_pack_open_file(
    obielf_resource_pack_t *pack,
    const char *path);

void obielf_resource_pack_close(obielf_resource_pack_t *pack);

obielf_status_t obielf_resource_pack_find(
    const obielf_resource_pack_t *pack,
    const char *name,
    obielf_resource_view_t *view);

obielf_status_t obielf_resource_pack_entry(
    const obielf_resource_pack_t *pack,
    uint32_t index,
    obielf_resource_view_t *view);

obielf_status_t obielf_resource_pack_verify(
    const obielf_resource_pack_t *pack);

const char *obielf_status_string(obielf_status_t status);

#ifdef __cplusplus
}
#endif

#endif

