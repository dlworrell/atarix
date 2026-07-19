#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "atarix/bib.h"

#define VECTOR_SIZE 176u
#define TLV_RECORD_SIZE 56u
#define EXPECTED_CRC32C 0x25F8A83Au

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

static void seal_crc(uint8_t *bib, size_t length) {
    write_le32(bib + 20, 0u);
    write_le32(bib + 20, crc32c(bib, length));
}

static void write_reference_tlv(uint8_t *p,
                                uint16_t type,
                                uint64_t object_id,
                                uint64_t address,
                                uint64_t length) {
    write_le16(p + 0, type);
    write_le16(p + 2, ATARIX_BIB_TLV_FLAG_MANDATORY);
    write_le32(p + 4, TLV_RECORD_SIZE);
    write_le32(p + 8, ATARIX_BIB_REFERENCE_FIXED_SIZE_V1);
    write_le32(p + 12, 0u);
    write_le64(p + 16, object_id);
    write_le64(p + 24, address);
    write_le64(p + 32, length);
    write_le32(p + 40, 1u);
    write_le32(p + 44, 0u);
    write_le32(p + 48, 0u);
    write_le32(p + 52, 0u);
}

static void make_golden_vector(uint8_t bib[VECTOR_SIZE]) {
    memset(bib, 0, VECTOR_SIZE);
    memcpy(bib, "ATARXBIB", 8);
    write_le16(bib + 8, ATARIX_BIB_VERSION_MAJOR_V1);
    write_le16(bib + 10, ATARIX_BIB_VERSION_MINOR_V0);
    write_le16(bib + 12, ATARIX_BIB_HEADER_SIZE_V1);
    write_le16(bib + 14, 0u);
    write_le32(bib + 16, VECTOR_SIZE);
    write_le64(bib + 24, UINT64_C(0x1122334455667788));
    write_le64(bib + 32, UINT64_C(0x8877665544332211));
    write_le64(bib + 40, 1u);
    write_le32(bib + 48, ATARIX_BIB_HEADER_SIZE_V1);
    write_le32(bib + 52, 2u);

    write_reference_tlv(bib + 64,
                        ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE,
                        UINT64_C(0x1001), UINT64_C(0x100000), UINT64_C(0x2000));
    write_reference_tlv(bib + 120,
                        ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE,
                        UINT64_C(0x1002), UINT64_C(0x200000), UINT64_C(0x3000));
    seal_crc(bib, VECTOR_SIZE);
}

static int known_tlv_type(uint16_t type) {
    return type >= ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE &&
           type <= ATARIX_BIB_TLV_ENTROPY_SEED_REFERENCE;
}

static atarix_bib_error_t validate_vector(const uint8_t *bib, size_t window) {
    uint32_t total_length;
    uint32_t tlv_offset;
    uint32_t tlv_count;
    uint32_t stored_crc;
    uint32_t computed_crc;
    uint32_t offset;
    uint32_t index;
    unsigned service_count = 0;
    unsigned memory_count = 0;
    uint8_t copy[VECTOR_SIZE];

    if (bib == NULL || window < ATARIX_BIB_HEADER_SIZE_V1) {
        return ATARIX_BIB_E_WINDOW;
    }
    if (!atarix_bib_magic_is_valid(bib)) {
        return ATARIX_BIB_E_MAGIC;
    }
    if (read_le16(bib + 8) != ATARIX_BIB_VERSION_MAJOR_V1) {
        return ATARIX_BIB_E_VERSION;
    }
    if (read_le16(bib + 12) != ATARIX_BIB_HEADER_SIZE_V1) {
        return ATARIX_BIB_E_HEADER_LENGTH;
    }

    total_length = read_le32(bib + 16);
    if (total_length < ATARIX_BIB_HEADER_SIZE_V1 ||
        total_length > window ||
        total_length > VECTOR_SIZE ||
        !atarix_bib_length_is_aligned(total_length)) {
        return ATARIX_BIB_E_TOTAL_LENGTH;
    }

    memcpy(copy, bib, total_length);
    stored_crc = read_le32(copy + 20);
    write_le32(copy + 20, 0u);
    computed_crc = crc32c(copy, total_length);
    if (stored_crc != computed_crc) {
        return ATARIX_BIB_E_CRC;
    }

    if (!atarix_bib_header_flags_are_known(read_le16(bib + 14)) ||
        read_le64(bib + 56) != 0u) {
        return ATARIX_BIB_E_HEADER_RESERVED;
    }

    tlv_offset = read_le32(bib + 48);
    tlv_count = read_le32(bib + 52);
    if (tlv_offset < ATARIX_BIB_HEADER_SIZE_V1 ||
        tlv_offset > total_length ||
        !atarix_bib_length_is_aligned(tlv_offset)) {
        return ATARIX_BIB_E_TLV_BOUNDS;
    }

    offset = tlv_offset;
    for (index = 0; index < tlv_count; ++index) {
        uint16_t type;
        uint16_t flags;
        uint32_t length;
        uint32_t payload_length;

        if (offset > total_length || total_length - offset < ATARIX_BIB_TLV_HEADER_SIZE_V1) {
            return ATARIX_BIB_E_TLV_BOUNDS;
        }

        type = read_le16(bib + offset);
        flags = read_le16(bib + offset + 2);
        length = read_le32(bib + offset + 4);
        payload_length = read_le32(bib + offset + 8);

        if (length < ATARIX_BIB_TLV_HEADER_SIZE_V1 ||
            !atarix_bib_length_is_aligned(length) ||
            length > total_length - offset ||
            payload_length > length - ATARIX_BIB_TLV_HEADER_SIZE_V1) {
            return ATARIX_BIB_E_TLV_BOUNDS;
        }
        if (!atarix_bib_tlv_flags_are_known(flags) || read_le32(bib + offset + 12) != 0u) {
            return ATARIX_BIB_E_TLV_RESERVED;
        }
        if (!known_tlv_type(type) && (flags & ATARIX_BIB_TLV_FLAG_MANDATORY) != 0u) {
            return ATARIX_BIB_E_UNKNOWN_MANDATORY;
        }

        if (type == ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE ||
            type == ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE) {
            uint64_t address;
            uint64_t referenced_length;
            if (payload_length < ATARIX_BIB_REFERENCE_FIXED_SIZE_V1) {
                return ATARIX_BIB_E_TLV_BOUNDS;
            }
            address = read_le64(bib + offset + 24);
            referenced_length = read_le64(bib + offset + 32);
            if (address > UINT64_MAX - referenced_length) {
                return ATARIX_BIB_E_REFERENCE_OVERFLOW;
            }
            service_count += type == ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE;
            memory_count += type == ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE;
        }

        offset += length;
    }

    if (offset != total_length) {
        return ATARIX_BIB_E_TLV_BOUNDS;
    }
    if (service_count != 1u || memory_count != 1u) {
        return ATARIX_BIB_E_REQUIRED_RECORD;
    }

    return ATARIX_BIB_OK;
}

static int expect(const char *name, atarix_bib_error_t actual, atarix_bib_error_t expected) {
    if (actual != expected) {
        fprintf(stderr, "%s: expected %d, got %d\n", name, (int)expected, (int)actual);
        return 1;
    }
    return 0;
}

int main(void) {
    uint8_t bib[VECTOR_SIZE];
    int failures = 0;

    make_golden_vector(bib);
    if (read_le32(bib + 20) != EXPECTED_CRC32C) {
        fprintf(stderr, "golden CRC changed: expected %08X, got %08X\n",
                EXPECTED_CRC32C, read_le32(bib + 20));
        ++failures;
    }
    failures += expect("golden", validate_vector(bib, sizeof(bib)), ATARIX_BIB_OK);
    failures += expect("truncated window", validate_vector(bib, 63u), ATARIX_BIB_E_WINDOW);

    make_golden_vector(bib);
    bib[0] ^= 1u;
    failures += expect("magic", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_MAGIC);

    make_golden_vector(bib);
    write_le16(bib + 8, 2u);
    seal_crc(bib, sizeof(bib));
    failures += expect("version", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_VERSION);

    make_golden_vector(bib);
    write_le32(bib + 16, 175u);
    failures += expect("unaligned total length", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_TOTAL_LENGTH);

    make_golden_vector(bib);
    bib[100] ^= 1u;
    failures += expect("CRC", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_CRC);

    make_golden_vector(bib);
    write_le32(bib + 68, 64u);
    seal_crc(bib, sizeof(bib));
    failures += expect("TLV overrun", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_TLV_BOUNDS);

    make_golden_vector(bib);
    write_le16(bib + 64, 0x7000u);
    seal_crc(bib, sizeof(bib));
    failures += expect("unknown mandatory", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_UNKNOWN_MANDATORY);

    make_golden_vector(bib);
    write_le16(bib + 64, ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE);
    seal_crc(bib, sizeof(bib));
    failures += expect("missing service", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_REQUIRED_RECORD);

    make_golden_vector(bib);
    write_le64(bib + 88, UINT64_MAX - 15u);
    write_le64(bib + 96, 32u);
    seal_crc(bib, sizeof(bib));
    failures += expect("reference overflow", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_REFERENCE_OVERFLOW);

    make_golden_vector(bib);
    write_le16(bib + 64, 0x7000u);
    write_le16(bib + 66, 0u);
    seal_crc(bib, sizeof(bib));
    failures += expect("unknown optional skipped", validate_vector(bib, sizeof(bib)), ATARIX_BIB_E_REQUIRED_RECORD);

    return failures == 0 ? 0 : 1;
}
