#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "atarix/bib_builder.h"

#define VECTOR_SIZE 176u
#define EXPECTED_CRC32C 0x25F8A83Au

static uint32_t read_le32(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static int expect_result(const char *name,
                         atarix_bib_builder_result_t actual,
                         atarix_bib_builder_result_t expected) {
    if (actual != expected) {
        fprintf(stderr, "%s: expected %d, got %d\n",
                name, (int)expected, (int)actual);
        return 1;
    }
    return 0;
}

int main(void) {
    uint8_t bib[VECTOR_SIZE];
    atarix_bib_builder_t builder;
    uint32_t serialized_length = 0;
    int failures = 0;

    failures += expect_result(
        "init",
        atarix_bib_builder_init(&builder, bib, sizeof(bib), 0u,
                                UINT64_C(0x1122334455667788),
                                UINT64_C(0x8877665544332211), 1u),
        ATARIX_BIB_BUILDER_OK);

    failures += expect_result(
        "service reference",
        atarix_bib_builder_add_reference(
            &builder, ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE,
            ATARIX_BIB_TLV_FLAG_MANDATORY,
            UINT64_C(0x1001), UINT64_C(0x100000), UINT64_C(0x2000), 1u, 0u),
        ATARIX_BIB_BUILDER_OK);

    failures += expect_result(
        "memory reference",
        atarix_bib_builder_add_reference(
            &builder, ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE,
            ATARIX_BIB_TLV_FLAG_MANDATORY,
            UINT64_C(0x1002), UINT64_C(0x200000), UINT64_C(0x3000), 1u, 0u),
        ATARIX_BIB_BUILDER_OK);

    failures += expect_result(
        "seal",
        atarix_bib_builder_seal(&builder, &serialized_length),
        ATARIX_BIB_BUILDER_OK);

    if (serialized_length != VECTOR_SIZE) {
        fprintf(stderr, "length: expected %u, got %u\n",
                VECTOR_SIZE, serialized_length);
        ++failures;
    }
    if (read_le32(bib + 20) != EXPECTED_CRC32C) {
        fprintf(stderr, "CRC: expected %08X, got %08X\n",
                EXPECTED_CRC32C, read_le32(bib + 20));
        ++failures;
    }
    if (memcmp(bib, "ATARXBIB", 8) != 0 || read_le32(bib + 16) != VECTOR_SIZE ||
        read_le32(bib + 52) != 2u) {
        fprintf(stderr, "sealed header fields do not match the v1 vector\n");
        ++failures;
    }

    failures += expect_result(
        "sealed mutation",
        atarix_bib_builder_add_tlv(&builder, ATARIX_BIB_TLV_BOOT_ARGUMENTS,
                                   0u, "x", 1u),
        ATARIX_BIB_BUILDER_E_STATE);
    failures += expect_result(
        "double seal",
        atarix_bib_builder_seal(&builder, NULL),
        ATARIX_BIB_BUILDER_E_STATE);

    failures += expect_result(
        "unknown header flags",
        atarix_bib_builder_init(&builder, bib, sizeof(bib), 0x8000u,
                                0u, 0u, 0u),
        ATARIX_BIB_BUILDER_E_FLAGS);

    failures += expect_result(
        "small capacity init",
        atarix_bib_builder_init(&builder, bib, 63u, 0u, 0u, 0u, 0u),
        ATARIX_BIB_BUILDER_E_ARGUMENT);

    failures += expect_result(
        "capacity init",
        atarix_bib_builder_init(&builder, bib, 120u, 0u, 0u, 0u, 0u),
        ATARIX_BIB_BUILDER_OK);
    failures += expect_result(
        "capacity exhaustion",
        atarix_bib_builder_add_reference(
            &builder, ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE,
            ATARIX_BIB_TLV_FLAG_MANDATORY, 1u, 1u, 1u, 1u, 0u),
        ATARIX_BIB_BUILDER_OK);
    failures += expect_result(
        "second capacity exhaustion",
        atarix_bib_builder_add_reference(
            &builder, ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE,
            ATARIX_BIB_TLV_FLAG_MANDATORY, 2u, 2u, 2u, 1u, 0u),
        ATARIX_BIB_BUILDER_E_CAPACITY);

    return failures == 0 ? 0 : 1;
}
