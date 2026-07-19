#include <limits.h>
#include <string.h>

#include "atarix/bib_reader.h"

static uint16_t read_le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_le32(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint64_t read_le64(const uint8_t *p) {
    return (uint64_t)read_le32(p) | ((uint64_t)read_le32(p + 4) << 32);
}

static uint32_t crc32c_with_zeroed_field(const uint8_t *data, uint32_t length) {
    uint32_t crc = UINT32_MAX;
    uint32_t i;

    for (i = 0; i < length; ++i) {
        uint8_t byte = (i >= 20u && i < 24u) ? 0u : data[i];
        unsigned bit;
        crc ^= byte;
        for (bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ ((crc & 1u) ? 0x82F63B78u : 0u);
        }
    }
    return crc ^ UINT32_MAX;
}

static int known_type(uint16_t type) {
    return type >= ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE &&
           type <= ATARIX_BIB_TLV_ENTROPY_SEED_REFERENCE;
}

static atarix_bib_error_t record_at_offset(const atarix_bib_view_t *view,
                                            uint32_t offset,
                                            atarix_bib_record_view_t *record) {
    const uint8_t *p;
    uint32_t length;
    uint32_t payload_length;

    if (offset > view->total_length ||
        view->total_length - offset < ATARIX_BIB_TLV_HEADER_SIZE_V1) {
        return ATARIX_BIB_E_TLV_BOUNDS;
    }
    p = view->buffer + offset;
    length = read_le32(p + 4);
    payload_length = read_le32(p + 8);
    if (length < ATARIX_BIB_TLV_HEADER_SIZE_V1 ||
        !atarix_bib_length_is_aligned(length) ||
        length > view->total_length - offset ||
        payload_length > length - ATARIX_BIB_TLV_HEADER_SIZE_V1) {
        return ATARIX_BIB_E_TLV_BOUNDS;
    }
    if (!atarix_bib_tlv_flags_are_known(read_le16(p + 2)) ||
        read_le32(p + 12) != 0u) {
        return ATARIX_BIB_E_TLV_RESERVED;
    }
    record->type = read_le16(p);
    record->flags = read_le16(p + 2);
    record->payload = p + ATARIX_BIB_TLV_HEADER_SIZE_V1;
    record->payload_length = payload_length;
    record->serialized_length = length;
    return ATARIX_BIB_OK;
}

atarix_bib_error_t atarix_bib_reader_open(atarix_bib_view_t *view,
                                           const void *buffer,
                                           size_t window_length) {
    const uint8_t *p = (const uint8_t *)buffer;
    uint32_t total_length;
    uint32_t offset;
    uint32_t index;
    unsigned service_count = 0;
    unsigned memory_count = 0;

    if (view == NULL || p == NULL || window_length < ATARIX_BIB_HEADER_SIZE_V1) {
        return ATARIX_BIB_E_WINDOW;
    }
    memset(view, 0, sizeof(*view));
    if (!atarix_bib_magic_is_valid(p)) {
        return ATARIX_BIB_E_MAGIC;
    }
    if (read_le16(p + 8) != ATARIX_BIB_VERSION_MAJOR_V1) {
        return ATARIX_BIB_E_VERSION;
    }
    if (read_le16(p + 12) != ATARIX_BIB_HEADER_SIZE_V1) {
        return ATARIX_BIB_E_HEADER_LENGTH;
    }
    total_length = read_le32(p + 16);
    if (total_length < ATARIX_BIB_HEADER_SIZE_V1 ||
        total_length > window_length ||
        total_length > ATARIX_BIB_RECOMMENDED_MAX_SIZE_V1 ||
        !atarix_bib_length_is_aligned(total_length)) {
        return ATARIX_BIB_E_TOTAL_LENGTH;
    }
    if (read_le32(p + 20) != crc32c_with_zeroed_field(p, total_length)) {
        return ATARIX_BIB_E_CRC;
    }
    if (!atarix_bib_header_flags_are_known(read_le16(p + 14)) ||
        read_le64(p + 56) != 0u) {
        return ATARIX_BIB_E_HEADER_RESERVED;
    }

    view->buffer = p;
    view->total_length = total_length;
    view->tlv_offset = read_le32(p + 48);
    view->tlv_count = read_le32(p + 52);
    view->flags = read_le16(p + 14);
    view->boot_id = read_le64(p + 24);
    view->producer_id = read_le64(p + 32);
    view->created_counter = read_le64(p + 40);

    if (view->tlv_offset < ATARIX_BIB_HEADER_SIZE_V1 ||
        view->tlv_offset > total_length ||
        !atarix_bib_length_is_aligned(view->tlv_offset)) {
        memset(view, 0, sizeof(*view));
        return ATARIX_BIB_E_TLV_BOUNDS;
    }

    offset = view->tlv_offset;
    for (index = 0; index < view->tlv_count; ++index) {
        atarix_bib_record_view_t record;
        atarix_bib_error_t error = record_at_offset(view, offset, &record);
        if (error != ATARIX_BIB_OK) {
            memset(view, 0, sizeof(*view));
            return error;
        }
        if (!known_type(record.type) &&
            (record.flags & ATARIX_BIB_TLV_FLAG_MANDATORY) != 0u) {
            memset(view, 0, sizeof(*view));
            return ATARIX_BIB_E_UNKNOWN_MANDATORY;
        }
        if (record.type == ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE ||
            record.type == ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE) {
            uint64_t address;
            uint64_t length;
            if (record.payload_length < ATARIX_BIB_REFERENCE_FIXED_SIZE_V1) {
                memset(view, 0, sizeof(*view));
                return ATARIX_BIB_E_TLV_BOUNDS;
            }
            address = read_le64(record.payload + 8);
            length = read_le64(record.payload + 16);
            if (address > UINT64_MAX - length) {
                memset(view, 0, sizeof(*view));
                return ATARIX_BIB_E_REFERENCE_OVERFLOW;
            }
            service_count += record.type == ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE;
            memory_count += record.type == ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE;
        }
        offset += record.serialized_length;
    }
    if (offset != total_length) {
        memset(view, 0, sizeof(*view));
        return ATARIX_BIB_E_TLV_BOUNDS;
    }
    if (service_count != 1u || memory_count != 1u) {
        memset(view, 0, sizeof(*view));
        return ATARIX_BIB_E_REQUIRED_RECORD;
    }
    return ATARIX_BIB_OK;
}

atarix_bib_error_t atarix_bib_reader_record(const atarix_bib_view_t *view,
                                             uint32_t index,
                                             atarix_bib_record_view_t *record) {
    uint32_t offset;
    uint32_t current;
    if (view == NULL || view->buffer == NULL || record == NULL || index >= view->tlv_count) {
        return ATARIX_BIB_E_TLV_BOUNDS;
    }
    offset = view->tlv_offset;
    for (current = 0; current < index; ++current) {
        atarix_bib_record_view_t skip;
        atarix_bib_error_t error = record_at_offset(view, offset, &skip);
        if (error != ATARIX_BIB_OK) {
            return error;
        }
        offset += skip.serialized_length;
    }
    return record_at_offset(view, offset, record);
}

atarix_bib_error_t atarix_bib_reader_find_reference(
    const atarix_bib_view_t *view,
    uint16_t type,
    atarix_bib_reference_view_t *reference) {
    uint32_t index;
    if (view == NULL || reference == NULL) {
        return ATARIX_BIB_E_WINDOW;
    }
    for (index = 0; index < view->tlv_count; ++index) {
        atarix_bib_record_view_t record;
        atarix_bib_error_t error = atarix_bib_reader_record(view, index, &record);
        if (error != ATARIX_BIB_OK) {
            return error;
        }
        if (record.type == type) {
            uint32_t integrity_length;
            if (record.payload_length < ATARIX_BIB_REFERENCE_FIXED_SIZE_V1) {
                return ATARIX_BIB_E_TLV_BOUNDS;
            }
            integrity_length = read_le32(record.payload + 36);
            if (integrity_length > record.payload_length - ATARIX_BIB_REFERENCE_FIXED_SIZE_V1) {
                return ATARIX_BIB_E_REFERENCE_INTEGRITY;
            }
            reference->object_id = read_le64(record.payload + 0);
            reference->address = read_le64(record.payload + 8);
            reference->length = read_le64(record.payload + 16);
            reference->format_major = read_le32(record.payload + 24);
            reference->format_minor = read_le32(record.payload + 28);
            reference->integrity_kind = read_le32(record.payload + 32);
            reference->integrity_length = integrity_length;
            reference->integrity = record.payload + ATARIX_BIB_REFERENCE_FIXED_SIZE_V1;
            return ATARIX_BIB_OK;
        }
    }
    return ATARIX_BIB_E_REQUIRED_RECORD;
}
