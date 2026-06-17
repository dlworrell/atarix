#include <stdint.h>
#include <stdio.h>

#include "atarix/memory.h"

static int expect_status(
    const char *name,
    atarix_memory_status_t actual,
    atarix_memory_status_t expected)
{
    if (actual != expected) {
        fprintf(stderr,
                "FAIL %s expected=%d actual=%d\n",
                name,
                (int)expected,
                (int)actual);
        return 1;
    }

    printf("PASS %s status=%d\n", name, (int)actual);
    return 0;
}

static atarix_memory_descriptor_v1_t valid_descriptor(void)
{
    atarix_memory_descriptor_v1_t descriptor;

    descriptor.magic = ATARIX_MEMORY_DESCRIPTOR_MAGIC;
    descriptor.version = ATARIX_MEMORY_DESCRIPTOR_VER_1;
    descriptor.reserved = 0u;
    descriptor.flags = ATARIX_MEMORY_MOVE_COPY;
    descriptor.source_object = 0x1001u;
    descriptor.source_offset = 0u;
    descriptor.destination_object = 0x2002u;
    descriptor.destination_offset = 0u;
    descriptor.length = 512u;
    descriptor.capability_id = 0x7777u;
    descriptor.generation = 1u;
    descriptor.audit_hint = 0u;

    return descriptor;
}

int main(void)
{
    int failures = 0;
    atarix_memory_descriptor_v1_t descriptor;

    printf("[+] ATX-SPEC-021: Executing Memory Descriptor Validation Suite\n");

    failures += expect_status(
        "null_descriptor",
        atarix_validate_descriptor(NULL),
        ATARIX_MEMORY_STATUS_NULL_DESCRIPTOR);

    descriptor = valid_descriptor();
    failures += expect_status(
        "valid_copy_descriptor_alias",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_OK);

    descriptor = valid_descriptor();
    failures += expect_status(
        "valid_copy_descriptor_canonical",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_OK);

    descriptor = valid_descriptor();
    descriptor.magic = 0xDEADBEEFu;
    failures += expect_status(
        "bad_magic",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_MAGIC);

    descriptor = valid_descriptor();
    descriptor.version = 99u;
    failures += expect_status(
        "bad_version",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_VERSION);

    descriptor = valid_descriptor();
    descriptor.length = 0u;
    failures += expect_status(
        "zero_length",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_ZERO_LENGTH);

    descriptor = valid_descriptor();
    descriptor.length = ATARIX_MEMORY_DESCRIPTOR_MAX_LENGTH_V1 + 1u;
    failures += expect_status(
        "length_exceeded",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_LENGTH_EXCEEDED);

    descriptor = valid_descriptor();
    descriptor.capability_id = 0u;
    failures += expect_status(
        "missing_capability",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_MISSING_CAPABILITY);

    descriptor = valid_descriptor();
    descriptor.source_object = 0u;
    failures += expect_status(
        "missing_source",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_MISSING_SOURCE);

    descriptor = valid_descriptor();
    descriptor.destination_object = 0u;
    failures += expect_status(
        "missing_destination",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_MISSING_DESTINATION);

    descriptor = valid_descriptor();
    descriptor.flags = 0u;
    failures += expect_status(
        "no_move_form",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_FLAGS);

    descriptor = valid_descriptor();
    descriptor.flags = ATARIX_MEMORY_MOVE_COPY | ATARIX_MEMORY_MOVE_MOVE;
    failures += expect_status(
        "multiple_move_forms",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_FLAGS);

    descriptor = valid_descriptor();
    descriptor.flags = 0x80u;
    failures += expect_status(
        "unknown_flags",
        atarix_validate_descriptor(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_FLAGS);

    if (failures != 0) {
        fprintf(stderr, "ATX-SPEC-021 memory descriptor tests failed: %d\n", failures);
        return 1;
    }

    printf("[+] SUCCESS: ATX-SPEC-021 validation vectors passed successfully.\n");
    return 0;
}
