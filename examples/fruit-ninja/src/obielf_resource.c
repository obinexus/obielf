#include "obielf/resource_pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t read_u16_le(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8u));
}

static uint32_t read_u32_le(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8u) |
           ((uint32_t)bytes[2] << 16u) | ((uint32_t)bytes[3] << 24u);
}

static uint64_t read_u64_le(const uint8_t *bytes)
{
    uint64_t value = 0u;
    unsigned int index;

    for (index = 0u; index < 8u; ++index) {
        value |= (uint64_t)bytes[index] << (index * 8u);
    }
    return value;
}

static int range_is_valid(uint64_t offset, uint64_t length, size_t total)
{
    const uint64_t total64 = (uint64_t)total;
    return offset <= total64 && length <= total64 - offset;
}

static int resource_name_is_valid(const uint8_t *name)
{
    size_t length = 0u;
    size_t index;

    while (length < OBIELF_RESOURCE_NAME_CAPACITY && name[length] != 0u) {
        ++length;
    }
    if (length == 0u || length == OBIELF_RESOURCE_NAME_CAPACITY) {
        return 0;
    }
    if (name[0] == (uint8_t)'/' || name[0] == (uint8_t)'\\') {
        return 0;
    }
    for (index = 0u; index < length; ++index) {
        if (name[index] == (uint8_t)'\\') {
            return 0;
        }
        if (index + 1u < length && name[index] == (uint8_t)'.' &&
            name[index + 1u] == (uint8_t)'.') {
            return 0;
        }
    }
    return 1;
}

uint32_t obielf_crc32(const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc = UINT32_MAX;
    size_t byte_index;

    if (data == NULL && size != 0u) {
        return 0u;
    }

    for (byte_index = 0u; byte_index < size; ++byte_index) {
        unsigned int bit_index;
        crc ^= bytes[byte_index];
        for (bit_index = 0u; bit_index < 8u; ++bit_index) {
            const uint32_t mask = (uint32_t)(0u - (crc & 1u));
            crc = (crc >> 1u) ^ (UINT32_C(0xedb88320) & mask);
        }
    }
    return ~crc;
}

static obielf_status_t validate_entries(const obielf_resource_pack_t *pack)
{
    uint32_t index;
    char previous[OBIELF_RESOURCE_NAME_CAPACITY + 1u] = {0};

    for (index = 0u; index < pack->entry_count; ++index) {
        const uint64_t record_offset =
            pack->directory_offset +
            ((uint64_t)index * OBIELF_RESOURCE_ENTRY_SIZE);
        const uint8_t *record = pack->bytes + (size_t)record_offset;
        const uint64_t payload_offset = read_u64_le(record + 64u);
        const uint64_t payload_size = read_u64_le(record + 72u);
        char current[OBIELF_RESOURCE_NAME_CAPACITY + 1u] = {0};

        if (!resource_name_is_valid(record)) {
            return OBIELF_ERROR_FORMAT;
        }
        memcpy(current, record, OBIELF_RESOURCE_NAME_CAPACITY);
        if (index > 0u && strcmp(previous, current) >= 0) {
            return OBIELF_ERROR_FORMAT;
        }
        memcpy(previous, current, sizeof(previous));

        if (payload_offset < pack->data_offset ||
            !range_is_valid(payload_offset, payload_size, pack->size)) {
            return OBIELF_ERROR_BOUNDS;
        }
    }
    return OBIELF_OK;
}

obielf_status_t obielf_resource_pack_open_memory(
    obielf_resource_pack_t *pack,
    const void *bytes,
    size_t size)
{
    const uint8_t *raw = (const uint8_t *)bytes;
    uint8_t header[OBIELF_RESOURCE_HEADER_SIZE];
    uint64_t directory_size;
    uint64_t file_size;
    uint32_t expected_crc;
    obielf_status_t status;

    if (pack == NULL || bytes == NULL) {
        return OBIELF_ERROR_ARGUMENT;
    }
    memset(pack, 0, sizeof(*pack));
    if (size < OBIELF_RESOURCE_HEADER_SIZE) {
        return OBIELF_ERROR_FORMAT;
    }
    if (memcmp(raw, OBIELF_RESOURCE_MAGIC, 8u) != 0) {
        return OBIELF_ERROR_MAGIC;
    }
    if (read_u16_le(raw + 8u) != OBIELF_RESOURCE_VERSION) {
        return OBIELF_ERROR_VERSION;
    }
    if (read_u16_le(raw + 10u) != OBIELF_RESOURCE_HEADER_SIZE ||
        read_u16_le(raw + 12u) != OBIELF_RESOURCE_ENTRY_SIZE) {
        return OBIELF_ERROR_FORMAT;
    }

    memcpy(header, raw, sizeof(header));
    expected_crc = read_u32_le(header + 48u);
    memset(header + 48u, 0, 4u);
    if (obielf_crc32(header, sizeof(header)) != expected_crc) {
        return OBIELF_ERROR_CHECKSUM;
    }

    pack->bytes = raw;
    pack->size = size;
    pack->entry_count = read_u32_le(raw + 16u);
    pack->directory_offset = read_u64_le(raw + 20u);
    pack->data_offset = read_u64_le(raw + 28u);
    file_size = read_u64_le(raw + 36u);

    if (file_size != (uint64_t)size ||
        pack->directory_offset < OBIELF_RESOURCE_HEADER_SIZE) {
        return OBIELF_ERROR_FORMAT;
    }
    directory_size =
        (uint64_t)pack->entry_count * OBIELF_RESOURCE_ENTRY_SIZE;
    if (pack->entry_count != 0u &&
        directory_size / OBIELF_RESOURCE_ENTRY_SIZE != pack->entry_count) {
        return OBIELF_ERROR_BOUNDS;
    }
    if (!range_is_valid(
            pack->directory_offset, directory_size, pack->size) ||
        pack->data_offset < pack->directory_offset + directory_size ||
        pack->data_offset > (uint64_t)pack->size) {
        return OBIELF_ERROR_BOUNDS;
    }
    if (obielf_crc32(
            raw + (size_t)pack->directory_offset,
            (size_t)directory_size) != read_u32_le(raw + 44u)) {
        return OBIELF_ERROR_CHECKSUM;
    }

    status = validate_entries(pack);
    if (status != OBIELF_OK) {
        memset(pack, 0, sizeof(*pack));
    }
    return status;
}

obielf_status_t obielf_resource_pack_open_file(
    obielf_resource_pack_t *pack,
    const char *path)
{
    FILE *file;
    long file_size;
    uint8_t *bytes;
    obielf_status_t status;

    if (pack == NULL || path == NULL) {
        return OBIELF_ERROR_ARGUMENT;
    }
    file = fopen(path, "rb");
    if (file == NULL) {
        return OBIELF_ERROR_IO;
    }
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return OBIELF_ERROR_IO;
    }
    file_size = ftell(file);
    if (file_size < 0L || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return OBIELF_ERROR_IO;
    }
    bytes = (uint8_t *)malloc((size_t)file_size);
    if (bytes == NULL && file_size != 0L) {
        fclose(file);
        return OBIELF_ERROR_MEMORY;
    }
    if (fread(bytes, 1u, (size_t)file_size, file) != (size_t)file_size) {
        free(bytes);
        fclose(file);
        return OBIELF_ERROR_IO;
    }
    fclose(file);

    status = obielf_resource_pack_open_memory(
        pack, bytes, (size_t)file_size);
    if (status != OBIELF_OK) {
        free(bytes);
        return status;
    }
    pack->owned_bytes = bytes;
    return OBIELF_OK;
}

void obielf_resource_pack_close(obielf_resource_pack_t *pack)
{
    if (pack != NULL) {
        free(pack->owned_bytes);
        memset(pack, 0, sizeof(*pack));
    }
}

obielf_status_t obielf_resource_pack_entry(
    const obielf_resource_pack_t *pack,
    uint32_t index,
    obielf_resource_view_t *view)
{
    uint64_t record_offset;
    const uint8_t *record;
    uint64_t payload_offset;
    uint64_t payload_size;

    if (pack == NULL || view == NULL || pack->bytes == NULL) {
        return OBIELF_ERROR_ARGUMENT;
    }
    if (index >= pack->entry_count) {
        return OBIELF_ERROR_NOT_FOUND;
    }
    record_offset =
        pack->directory_offset +
        ((uint64_t)index * OBIELF_RESOURCE_ENTRY_SIZE);
    record = pack->bytes + (size_t)record_offset;
    payload_offset = read_u64_le(record + 64u);
    payload_size = read_u64_le(record + 72u);
    if (payload_size > (uint64_t)SIZE_MAX ||
        !range_is_valid(payload_offset, payload_size, pack->size)) {
        return OBIELF_ERROR_BOUNDS;
    }

    memset(view, 0, sizeof(*view));
    memcpy(view->name, record, OBIELF_RESOURCE_NAME_CAPACITY);
    view->data = pack->bytes + (size_t)payload_offset;
    view->size = (size_t)payload_size;
    view->checksum = read_u32_le(record + 80u);
    view->kind = read_u16_le(record + 84u);
    view->flags = read_u16_le(record + 86u);
    return OBIELF_OK;
}

obielf_status_t obielf_resource_pack_find(
    const obielf_resource_pack_t *pack,
    const char *name,
    obielf_resource_view_t *view)
{
    uint32_t low = 0u;
    uint32_t high;

    if (pack == NULL || name == NULL || view == NULL) {
        return OBIELF_ERROR_ARGUMENT;
    }
    high = pack->entry_count;
    while (low < high) {
        const uint32_t middle = low + ((high - low) / 2u);
        obielf_resource_view_t candidate;
        const obielf_status_t status =
            obielf_resource_pack_entry(pack, middle, &candidate);
        int comparison;

        if (status != OBIELF_OK) {
            return status;
        }
        comparison = strcmp(name, candidate.name);
        if (comparison == 0) {
            *view = candidate;
            return OBIELF_OK;
        }
        if (comparison < 0) {
            high = middle;
        } else {
            low = middle + 1u;
        }
    }
    return OBIELF_ERROR_NOT_FOUND;
}

obielf_status_t obielf_resource_pack_verify(
    const obielf_resource_pack_t *pack)
{
    uint32_t index;

    if (pack == NULL || pack->bytes == NULL) {
        return OBIELF_ERROR_ARGUMENT;
    }
    for (index = 0u; index < pack->entry_count; ++index) {
        obielf_resource_view_t view;
        obielf_status_t status =
            obielf_resource_pack_entry(pack, index, &view);
        if (status != OBIELF_OK) {
            return status;
        }
        if (obielf_crc32(view.data, view.size) != view.checksum) {
            return OBIELF_ERROR_CHECKSUM;
        }
    }
    return OBIELF_OK;
}

const char *obielf_status_string(obielf_status_t status)
{
    switch (status) {
    case OBIELF_OK:
        return "ok";
    case OBIELF_ERROR_ARGUMENT:
        return "invalid argument";
    case OBIELF_ERROR_IO:
        return "I/O failure";
    case OBIELF_ERROR_MEMORY:
        return "out of memory";
    case OBIELF_ERROR_MAGIC:
        return "invalid magic";
    case OBIELF_ERROR_VERSION:
        return "unsupported version";
    case OBIELF_ERROR_FORMAT:
        return "invalid format";
    case OBIELF_ERROR_BOUNDS:
        return "out-of-bounds record";
    case OBIELF_ERROR_CHECKSUM:
        return "checksum mismatch";
    case OBIELF_ERROR_NOT_FOUND:
        return "resource not found";
    default:
        return "unknown error";
    }
}

