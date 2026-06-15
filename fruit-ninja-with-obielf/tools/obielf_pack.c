#include "obielf/resource_pack.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MANIFEST_ENTRIES 256u
#define MAX_PATH_LENGTH 1024u

typedef struct source_entry {
    char name[OBIELF_RESOURCE_NAME_CAPACITY];
    char path[MAX_PATH_LENGTH];
    uint8_t *data;
    size_t size;
    uint32_t checksum;
    uint64_t offset;
    uint16_t kind;
} source_entry_t;

static void write_u16_le(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value & UINT16_C(0xff));
    bytes[1] = (uint8_t)(value >> 8u);
}

static void write_u32_le(uint8_t *bytes, uint32_t value)
{
    unsigned int index;
    for (index = 0u; index < 4u; ++index) {
        bytes[index] = (uint8_t)(value >> (index * 8u));
    }
}

static void write_u64_le(uint8_t *bytes, uint64_t value)
{
    unsigned int index;
    for (index = 0u; index < 8u; ++index) {
        bytes[index] = (uint8_t)(value >> (index * 8u));
    }
}

static uint64_t align16(uint64_t value)
{
    return (value + UINT64_C(15)) & ~UINT64_C(15);
}

static char *trim(char *text)
{
    char *end;

    while (*text != '\0' && isspace((unsigned char)*text) != 0) {
        ++text;
    }
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1]) != 0) {
        --end;
    }
    *end = '\0';
    return text;
}

static int valid_name(const char *name)
{
    size_t index;
    const size_t length = strlen(name);

    if (length == 0u || length >= OBIELF_RESOURCE_NAME_CAPACITY ||
        name[0] == '/' || name[0] == '\\') {
        return 0;
    }
    for (index = 0u; index < length; ++index) {
        if (name[index] == '\\' ||
            (index + 1u < length && name[index] == '.' &&
             name[index + 1u] == '.')) {
            return 0;
        }
    }
    return 1;
}

static uint16_t resource_kind(const char *path)
{
    const char *extension = strrchr(path, '.');
    if (extension != NULL &&
        (strcmp(extension, ".png") == 0 || strcmp(extension, ".PNG") == 0)) {
        return OBIELF_RESOURCE_PNG;
    }
    return OBIELF_RESOURCE_BINARY;
}

static int load_file(const char *path, uint8_t **data, size_t *size)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *buffer;

    if (file == NULL || fseek(file, 0L, SEEK_END) != 0) {
        if (file != NULL) {
            fclose(file);
        }
        return 0;
    }
    length = ftell(file);
    if (length < 0L || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    buffer = (uint8_t *)malloc((size_t)length);
    if (buffer == NULL && length != 0L) {
        fclose(file);
        return 0;
    }
    if (fread(buffer, 1u, (size_t)length, file) != (size_t)length) {
        free(buffer);
        fclose(file);
        return 0;
    }
    fclose(file);
    *data = buffer;
    *size = (size_t)length;
    return 1;
}

static int compare_entries(const void *left, const void *right)
{
    const source_entry_t *a = (const source_entry_t *)left;
    const source_entry_t *b = (const source_entry_t *)right;
    return strcmp(a->name, b->name);
}

static void free_entries(source_entry_t *entries, size_t count)
{
    size_t index;
    for (index = 0u; index < count; ++index) {
        free(entries[index].data);
    }
}

static int read_manifest(
    const char *root,
    const char *manifest_path,
    source_entry_t *entries,
    size_t *entry_count)
{
    FILE *manifest = fopen(manifest_path, "r");
    char line[2048];
    size_t count = 0u;

    if (manifest == NULL) {
        fprintf(stderr, "cannot open manifest: %s\n", manifest_path);
        return 0;
    }
    while (fgets(line, (int)sizeof(line), manifest) != NULL) {
        char *content = trim(line);
        char *separator;
        char *name;
        char *relative;
        source_entry_t *entry;
        int path_length;

        if (*content == '\0' || *content == '#') {
            continue;
        }
        separator = strchr(content, '=');
        if (separator == NULL || count >= MAX_MANIFEST_ENTRIES) {
            fprintf(stderr, "invalid manifest line: %s\n", content);
            fclose(manifest);
            free_entries(entries, count);
            return 0;
        }
        *separator = '\0';
        name = trim(content);
        relative = trim(separator + 1);
        if (!valid_name(name) || !valid_name(relative)) {
            fprintf(stderr, "unsafe resource name: %s=%s\n", name, relative);
            fclose(manifest);
            free_entries(entries, count);
            return 0;
        }

        entry = &entries[count];
        memset(entry, 0, sizeof(*entry));
        memcpy(entry->name, name, strlen(name) + 1u);
        path_length = snprintf(
            entry->path, sizeof(entry->path), "%s/%s", root, relative);
        if (path_length < 0 ||
            (size_t)path_length >= sizeof(entry->path) ||
            !load_file(entry->path, &entry->data, &entry->size)) {
            fprintf(stderr, "cannot load resource: %s\n", entry->path);
            fclose(manifest);
            free_entries(entries, count);
            return 0;
        }
        entry->checksum = obielf_crc32(entry->data, entry->size);
        entry->kind = resource_kind(entry->path);
        ++count;
    }
    fclose(manifest);

    qsort(entries, count, sizeof(entries[0]), compare_entries);
    if (count == 0u) {
        fprintf(stderr, "manifest contains no resources\n");
        return 0;
    }
    {
        size_t index;
        for (index = 1u; index < count; ++index) {
            if (strcmp(entries[index - 1u].name, entries[index].name) == 0) {
                fprintf(stderr, "duplicate logical name: %s\n", entries[index].name);
                free_entries(entries, count);
                return 0;
            }
        }
    }
    *entry_count = count;
    return 1;
}

static int build_pack(
    source_entry_t *entries,
    size_t count,
    uint8_t **pack_bytes,
    size_t *pack_size)
{
    const uint64_t directory_offset = OBIELF_RESOURCE_HEADER_SIZE;
    const uint64_t directory_size =
        (uint64_t)count * OBIELF_RESOURCE_ENTRY_SIZE;
    const uint64_t data_offset = align16(directory_offset + directory_size);
    uint64_t cursor = data_offset;
    uint8_t *bytes;
    size_t index;

    for (index = 0u; index < count; ++index) {
        entries[index].offset = cursor;
        if ((uint64_t)entries[index].size > UINT64_MAX - cursor) {
            return 0;
        }
        cursor = align16(cursor + (uint64_t)entries[index].size);
    }
    if (cursor > (uint64_t)SIZE_MAX) {
        return 0;
    }
    bytes = (uint8_t *)calloc((size_t)cursor, 1u);
    if (bytes == NULL) {
        return 0;
    }

    memcpy(bytes, OBIELF_RESOURCE_MAGIC, 8u);
    write_u16_le(bytes + 8u, OBIELF_RESOURCE_VERSION);
    write_u16_le(bytes + 10u, OBIELF_RESOURCE_HEADER_SIZE);
    write_u16_le(bytes + 12u, OBIELF_RESOURCE_ENTRY_SIZE);
    write_u16_le(bytes + 14u, 0u);
    write_u32_le(bytes + 16u, (uint32_t)count);
    write_u64_le(bytes + 20u, directory_offset);
    write_u64_le(bytes + 28u, data_offset);
    write_u64_le(bytes + 36u, cursor);

    for (index = 0u; index < count; ++index) {
        uint8_t *record =
            bytes + (size_t)directory_offset +
            (index * OBIELF_RESOURCE_ENTRY_SIZE);
        memcpy(record, entries[index].name, strlen(entries[index].name) + 1u);
        write_u64_le(record + 64u, entries[index].offset);
        write_u64_le(record + 72u, (uint64_t)entries[index].size);
        write_u32_le(record + 80u, entries[index].checksum);
        write_u16_le(record + 84u, entries[index].kind);
        write_u16_le(record + 86u, 0u);
        memcpy(
            bytes + (size_t)entries[index].offset,
            entries[index].data,
            entries[index].size);
    }

    write_u32_le(
        bytes + 44u,
        obielf_crc32(
            bytes + (size_t)directory_offset, (size_t)directory_size));
    write_u32_le(bytes + 48u, 0u);
    write_u32_le(
        bytes + 48u,
        obielf_crc32(bytes, OBIELF_RESOURCE_HEADER_SIZE));
    *pack_bytes = bytes;
    *pack_size = (size_t)cursor;
    return 1;
}

static int write_binary(const char *path, const uint8_t *bytes, size_t size)
{
    FILE *output = fopen(path, "wb");
    if (output == NULL) {
        return 0;
    }
    if (fwrite(bytes, 1u, size, output) != size) {
        fclose(output);
        return 0;
    }
    return fclose(output) == 0;
}

static int write_header(const char *path, const char *symbol)
{
    FILE *output = fopen(path, "w");
    if (output == NULL) {
        return 0;
    }
    fprintf(output,
            "#ifndef FRUIT_ASSETS_EMBED_H\n"
            "#define FRUIT_ASSETS_EMBED_H\n\n"
            "#include <stddef.h>\n\n"
            "extern const unsigned char %s[];\n"
            "extern const size_t %s_size;\n\n"
            "#endif\n",
            symbol,
            symbol);
    return fclose(output) == 0;
}

static int write_c_source(
    const char *path,
    const char *header_path,
    const char *symbol,
    const uint8_t *bytes,
    size_t size)
{
    FILE *output = fopen(path, "w");
    const char *header_name;
    size_t index;

    if (output == NULL) {
        return 0;
    }
    header_name = strrchr(header_path, '/');
    if (header_name == NULL) {
        header_name = strrchr(header_path, '\\');
    }
    header_name = header_name == NULL ? header_path : header_name + 1;

    fprintf(output,
            "#include \"%s\"\n\n"
            "#if defined(__GNUC__) || defined(__clang__)\n"
            "#define OBIELF_RESOURCE_SECTION "
            "__attribute__((aligned(16), used, section(\".obi.resources\")))\n"
            "#else\n"
            "#define OBIELF_RESOURCE_SECTION\n"
            "#endif\n\n"
            "OBIELF_RESOURCE_SECTION\n"
            "const unsigned char %s[] = {\n",
            header_name,
            symbol);

    for (index = 0u; index < size; ++index) {
        if (index % 12u == 0u) {
            fputs("    ", output);
        }
        fprintf(output, "0x%02x", (unsigned int)bytes[index]);
        if (index + 1u != size) {
            fputs(", ", output);
        }
        if (index % 12u == 11u || index + 1u == size) {
            fputc('\n', output);
        }
    }
    fprintf(output,
            "};\n\n"
            "const size_t %s_size = sizeof(%s);\n",
            symbol,
            symbol);
    return fclose(output) == 0;
}

static int inspect_pack(const char *path)
{
    obielf_resource_pack_t pack;
    obielf_status_t status =
        obielf_resource_pack_open_file(&pack, path);
    uint32_t index;

    if (status != OBIELF_OK) {
        fprintf(stderr, "%s: %s\n", path, obielf_status_string(status));
        return 0;
    }
    status = obielf_resource_pack_verify(&pack);
    if (status != OBIELF_OK) {
        fprintf(stderr, "%s: %s\n", path, obielf_status_string(status));
        obielf_resource_pack_close(&pack);
        return 0;
    }
    printf("OBIRPACK v1: %u resources, %zu bytes\n",
           pack.entry_count,
           pack.size);
    for (index = 0u; index < pack.entry_count; ++index) {
        obielf_resource_view_t view;
        status = obielf_resource_pack_entry(&pack, index, &view);
        if (status != OBIELF_OK) {
            obielf_resource_pack_close(&pack);
            return 0;
        }
        printf("  %-32s %8zu bytes crc32=%08x\n",
               view.name,
               view.size,
               (unsigned int)view.checksum);
    }
    obielf_resource_pack_close(&pack);
    return 1;
}

static const char *argument_value(
    int argc,
    char **argv,
    const char *name)
{
    int index;
    for (index = 1; index + 1 < argc; ++index) {
        if (strcmp(argv[index], name) == 0) {
            return argv[index + 1];
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    const char *root;
    const char *manifest;
    const char *output_path;
    const char *c_output;
    const char *header_output;
    const char *symbol;
    source_entry_t entries[MAX_MANIFEST_ENTRIES];
    size_t entry_count = 0u;
    uint8_t *pack_bytes = NULL;
    size_t pack_size = 0u;
    int success;

    if (argc == 3 && strcmp(argv[1], "--inspect") == 0) {
        return inspect_pack(argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    root = argument_value(argc, argv, "--root");
    manifest = argument_value(argc, argv, "--manifest");
    output_path = argument_value(argc, argv, "--output");
    c_output = argument_value(argc, argv, "--c-output");
    header_output = argument_value(argc, argv, "--header-output");
    symbol = argument_value(argc, argv, "--symbol");
    if (root == NULL || manifest == NULL || output_path == NULL ||
        c_output == NULL || header_output == NULL || symbol == NULL) {
        fprintf(stderr,
                "usage: obielf-pack --root DIR --manifest FILE "
                "--output FILE --c-output FILE --header-output FILE "
                "--symbol IDENTIFIER\n"
                "       obielf-pack --inspect FILE\n");
        return EXIT_FAILURE;
    }

    memset(entries, 0, sizeof(entries));
    if (!read_manifest(
            root, manifest, entries, &entry_count) ||
        !build_pack(entries, entry_count, &pack_bytes, &pack_size)) {
        free_entries(entries, entry_count);
        free(pack_bytes);
        return EXIT_FAILURE;
    }

    success = write_binary(output_path, pack_bytes, pack_size) &&
              write_header(header_output, symbol) &&
              write_c_source(
                  c_output,
                  header_output,
                  symbol,
                  pack_bytes,
                  pack_size);
    if (!success) {
        fprintf(stderr, "failed to write generated resources: %s\n",
                strerror(errno));
    } else {
        printf("packed %zu resources into %s (%zu bytes)\n",
               entry_count,
               output_path,
               pack_size);
    }
    free_entries(entries, entry_count);
    free(pack_bytes);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

