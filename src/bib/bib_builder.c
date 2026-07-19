#include <limits.h>
#include <string.h>

#include "atarix/bib_builder.h"

static void write_le16(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

static void write_le32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static void write_le64(uint8_t *p, uint64_t value) {
    write_le32(p, (uint32_t)value);
    write_le32(p + 4, (uint32_t)(value >> 32));
}

static uint32_t crc32c(const uint8_t *data, size_t length) {
    uint32_t crc = UINT32_MAX;
    size_t i;

    for (i = 0; i < length; ++i) {
        unsigned bit;
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ ((crc & 1u) ? 0x82F63B78u : 0u);
        }
    }
    return crc ^ UINT32_MAX;
}

static int align8_u32(uint32_t value, uint32_t *aligned) {
    if (value > UINT32_MAX - 7u) {
        return 0;
    }
    *aligned = (value + 7u) & ~7u;
    return 1;
}

atarix_bib_builder_result_t atarix_bib_builder_init(
    atarix_bib_builder_t *builder,
    void *buffer,
    size_t capacity,
    uint16_t header_flags,
    uint64_t boot_id,
    uint64_t producer_id,
    uint64_t created_counter) {
    uint8_t *p = (uint8_t *)buffer;

    if (builder == NULL || p == NULL || capacity < ATARIX_BIB_HEADER_SIZE_V1 ||
        capacity > UINT32_MAX) {
        return ATARIX_BIB_BUILDER_E_ARGUMENT;
    }
    if (!atarix_bib_header_flags_are_known(header_flags)) {
        return ATARIX_BIB_BUILDER_E_FLAGS;
    }

    memset(p, 0, capacity);
    memcpy(p, "ATARXBIB", 8);
    write_le16(p + 8, ATARIX_BIB_VERSION_MAJOR_V1);
    write_le16(p + 10, ATARIX_BIB_VERSION_MINOR_V0);
    write_le16(p + 12, ATARIX_BIB_HEADER_SIZE_V1);
    write_le16(p + 14, header_flags);
    write_le64(p + 24, boot_id);
    write_le64(p + 32, producer_id);
    write_le64(p + 40, created_counter);
    write_le32(p + 48, ATARIX_BIB_HEADER_SIZE_V1);

    builder->buffer = p;
    builder->capacity = capacity;
    builder->length = ATARIX_BIB_HEADER_SIZE_V1;
    builder->tlv_count = 0;
    builder->sealed = 0;
    return ATARIX_BIB_BUILDER_OK;
}

atarix_bib_builder_result_t atarix_bib_builder_add_tlv(
    atarix_bib_builder_t *builder,
    uint16_t type,
    uint16_t flags,
    const void *payload,
    uint32_t payload_length) {
    uint32_t unaligned_length;
    uint32_t record_length;
    uint8_t *record;

    if (builder == NULL || builder->buffer == NULL ||
        (payload_length != 0u && payload == NULL)) {
        return ATARIX_BIB_BUILDER_E_ARGUMENT;
    }
    if (builder->sealed) {
        return ATARIX_BIB_BUILDER_E_STATE;
    }
    if (!atarix_bib_tlv_flags_are_known(flags)) {
        return ATARIX_BIB_BUILDER_E_FLAGS;
    }
    if (payload_length > UINT32_MAX - ATARIX_BIB_TLV_HEADER_SIZE_V1) {
        return ATARIX_BIB_BUILDER_E_LENGTH;
    }

    unaligned_length = ATARIX_BIB_TLV_HEADER_SIZE_V1 + payload_length;
    if (!align8_u32(unaligned_length, &record_length)) {
        return ATARIX_BIB_BUILDER_E_LENGTH;
    }
    if (record_length > builder->capacity - builder->length) {
        return ATARIX_BIB_BUILDER_E_CAPACITY;
    }

    record = builder->buffer + builder->length;
    memset(record, 0, record_length);
    write_le16(record + 0, type);
    write_le16(record + 2, flags);
    write_le32(record + 4, record_length);
    write_le32(record + 8, payload_length);
    if (payload_length != 0u) {
        memcpy(record + ATARIX_BIB_TLV_HEADER_SIZE_V1, payload, payload_length);
    }

    builder->length += record_length;
    builder->tlv_count += 1u;
    return ATARIX_BIB_BUILDER_OK;
}

atarix_bib_builder_result_t atarix_bib_builder_add_reference(
    atarix_bib_builder_t *builder,
    uint16_t type,
    uint16_t flags,
    uint64_t object_id,
    uint64_t address,
    uint64_t length,
    uint32_t format_major,
    uint32_t format_minor,
    uint32_t integrity_kind,
    const void *integrity,
    uint32_t integrity_length) {
    uint8_t payload[ATARIX_BIB_REFERENCE_FIXED_SIZE_V1];

    if ((integrity_length != 0u && integrity == NULL) ||
        integrity_length != 0u) {
        return integrity_length != 0u ? ATARIX_BIB_BUILDER_E_LENGTH
                                      : ATARIX_BIB_BUILDER_E_ARGUMENT;
    }

    memset(payload, 0, sizeof(payload));
    write_le64(payload + 0, object_id);
    write_le64(payload + 8, address);
    write_le64(payload + 16, length);
    write_le32(payload + 24, format_major);
    write_le32(payload + 28, format_minor);
    write_le32(payload + 32, integrity_kind);
    write_le32(payload + 36, integrity_length);

    return atarix_bib_builder_add_tlv(builder, type, flags, payload, sizeof(payload));
}

atarix_bib_builder_result_t atarix_bib_builder_seal(
    atarix_bib_builder_t *builder,
    uint32_t *serialized_length) {
    uint32_t crc;

    if (builder == NULL || builder->buffer == NULL) {
        return ATARIX_BIB_BUILDER_E_ARGUMENT;
    }
    if (builder->sealed) {
        return ATARIX_BIB_BUILDER_E_STATE;
    }

    write_le32(builder->buffer + 16, builder->length);
    write_le32(builder->buffer + 20, 0u);
    write_le32(builder->buffer + 52, builder->tlv_count);
    crc = crc32c(builder->buffer, builder->length);
    write_le32(builder->buffer + 20, crc);
    builder->sealed = 1;
    if (serialized_length != NULL) {
        *serialized_length = builder->length;
    }
    return ATARIX_BIB_BUILDER_OK;
}
