#include <stdint.h>
#include "atarix/discovery_parser.h"

static int atarix_offset_aligned8(size_t offset) {
    return (offset & 7u) == 0u;
}

int atarix_discovery_validate(const void *image, size_t image_size) {
    const atarix_discovery_header_v1_t *header;
    const atarix_discovery_record_header_v1_t *record;
    const uint8_t *base;
    size_t offset;
    uint32_t records_seen;
    int saw_end;

    if (!image) {
        return ATARIX_DISCOVERY_ERROR_NULL;
    }

    if (image_size < ATARIX_DISCOVERY_HEADER_SIZE_V1) {
        return ATARIX_DISCOVERY_ERROR_TOO_SMALL;
    }

    header = (const atarix_discovery_header_v1_t *)image;
    base = (const uint8_t *)image;

    if (!atarix_discovery_magic_is_valid(header->magic)) {
        return ATARIX_DISCOVERY_ERROR_BAD_MAGIC;
    }

    if (header->version_major != ATARIX_DISCOVERY_VERSION_MAJOR_V1) {
        return ATARIX_DISCOVERY_ERROR_UNSUPPORTED_VERSION;
    }

    if (header->total_length < ATARIX_DISCOVERY_HEADER_SIZE_V1 ||
        header->total_length > image_size ||
        !atarix_offset_aligned8(header->total_length)) {
        return ATARIX_DISCOVERY_ERROR_BAD_LENGTH;
    }

    offset = ATARIX_DISCOVERY_HEADER_SIZE_V1;
    records_seen = 0;
    saw_end = 0;

    while (offset < header->total_length) {
        if (!atarix_offset_aligned8(offset)) {
            return ATARIX_DISCOVERY_ERROR_BAD_ALIGNMENT;
        }

        if (header->total_length - offset < ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1) {
            return ATARIX_DISCOVERY_ERROR_TRUNCATED_RECORD;
        }

        record = (const atarix_discovery_record_header_v1_t *)(base + offset);

        if (record->length < ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1 ||
            !atarix_offset_aligned8(record->length)) {
            return ATARIX_DISCOVERY_ERROR_BAD_LENGTH;
        }

        if (record->length > header->total_length - offset) {
            return ATARIX_DISCOVERY_ERROR_TRUNCATED_RECORD;
        }

        ++records_seen;

        if (record->type == ATARIX_DISCOVERY_RECORD_END) {
            saw_end = 1;
            break;
        }

        offset += record->length;
    }

    if (!saw_end) {
        return ATARIX_DISCOVERY_ERROR_MISSING_END;
    }

    if (header->record_count != 0 && records_seen != header->record_count) {
        return ATARIX_DISCOVERY_ERROR_BAD_LENGTH;
    }

    /* CRC validation is intentionally left for the CRC utility implementation. */
    return ATARIX_DISCOVERY_OK;
}
