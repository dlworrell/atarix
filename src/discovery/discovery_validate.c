#include <stdint.h>
#include <string.h>
#include "atarix/discovery_parser.h"

static int atarix_offset_aligned8(size_t offset) {
    return (offset & 7u) == 0u;
}

static uint32_t atarix_crc32_iso_hdlc(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;

    for (i = 0; i < length; ++i) {
        int bit;
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

static int atarix_discovery_crc_valid(const void *image, size_t image_size) {
    uint8_t header_copy[ATARIX_DISCOVERY_HEADER_SIZE_V1];
    uint8_t image_copy_stack[512];
    uint8_t *image_copy;
    const atarix_discovery_header_v1_t *header;
    atarix_discovery_header_v1_t *mutable_header;
    uint32_t expected_header_crc;
    uint32_t expected_image_crc;
    uint32_t actual_header_crc;
    uint32_t actual_image_crc;

    header = (const atarix_discovery_header_v1_t *)image;
    expected_header_crc = header->header_crc32;
    expected_image_crc = header->image_crc32;

    memcpy(header_copy, image, ATARIX_DISCOVERY_HEADER_SIZE_V1);
    mutable_header = (atarix_discovery_header_v1_t *)header_copy;
    mutable_header->header_crc32 = 0;
    mutable_header->image_crc32 = 0;
    actual_header_crc = atarix_crc32_iso_hdlc(header_copy, ATARIX_DISCOVERY_HEADER_SIZE_V1);

    if (actual_header_crc != expected_header_crc) {
        return 0;
    }

    if (image_size > sizeof(image_copy_stack)) {
        return 1;
    }

    image_copy = image_copy_stack;
    memcpy(image_copy, image, image_size);
    mutable_header = (atarix_discovery_header_v1_t *)image_copy;
    mutable_header->image_crc32 = 0;
    actual_image_crc = atarix_crc32_iso_hdlc(image_copy, header->total_length);

    return actual_image_crc == expected_image_crc;
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

    if (!atarix_discovery_crc_valid(image, image_size)) {
        return ATARIX_DISCOVERY_ERROR_BAD_CRC;
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

    return ATARIX_DISCOVERY_OK;
}
