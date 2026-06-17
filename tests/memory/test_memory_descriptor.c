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

    descriptor.magic = ATARIX_MEMORY_DESCRIPTOR_MAGIC_V1;
    descriptor.version = ATARIX_MEMORY_DESCRIPTOR_VERSION_V1;
    descriptor.reserved = 0u;
    descriptor.flags = ATARIX_MEMORY_MOVE_COPY;
    descriptor.source_object = 0x1000u;
    descriptor.source_offset = 0u;
    descriptor.destination_object = 0x2000u;
    descriptor.destination_offset = 0u;
    descriptor.length = 64u;
    descriptor.capability_id = 0x3000u;
    descriptor.generation = 1u;
    descriptor.audit_hint = 0u;

    return descriptor;
}

int main(void)
{
    int failures = 0;
    atarix_memory_descriptor_v1_t descriptor;

    failures += expect_status(
        "null_descriptor",
        atarix_memory_descriptor_validate(NULL),
        ATARIX_MEMORY_STATUS_NULL_DESCRIPTOR);

    descriptor = valid_descriptor();
    failures += expect_status(
        "valid_copy_descriptor",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_OK);

    descriptor = valid_descriptor();
    descriptor.magic = 0u;
    failures += expect_status(
        "bad_magic",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_MAGIC);

    descriptor = valid_descriptor();
    descriptor.version = 0u;
    failures += expect_status(
        "bad_version",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_VERSION);

    descriptor = valid_descriptor();
    descriptor.length = 0u;
    failures += expect_status(
        "zero_length",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_ZERO_LENGTH);

    descriptor = valid_descriptor();
    descriptor.capability_id = 0u;
    failures += expect_status(
        "missing_capability",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_MISSING_CAPABILITY);

    descriptor = valid_descriptor();
    descriptor.source_object = 0u;
    failures += expect_status(
        "missing_source",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_MISSING_SOURCE);

    descriptor = valid_descriptor();
    descriptor.destination_object = 0u;
    failures += expect_status(
        "missing_destination",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_MISSING_DESTINATION);

    descriptor = valid_descriptor();
    descriptor.flags = ATARIX_MEMORY_MOVE_COPY | ATARIX_MEMORY_MOVE_MOVE;
    failures += expect_status(
        "multiple_move_forms",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_FLAGS);

    descriptor = valid_descriptor();
    descriptor.flags = 0x80000000u;
    failures += expect_status(
        "unknown_flags",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_BAD_FLAGS);

    descriptor = valid_descriptor();
    descriptor.length = ATARIX_MEMORY_DESCRIPTOR_MAX_LENGTH_V1 + 1u;
    failures += expect_status(
        "length_exceeded",
        atarix_memory_descriptor_validate(&descriptor),
        ATARIX_MEMORY_STATUS_LENGTH_EXCEEDED);

    if (failures != 0) {
        fprintf(stderr, "ATX-SPEC-021 memory descriptor tests failed: %d\n", failures);
        return 1;
    }

    printf("ATX-SPEC-021 memory descriptor tests passed\n");
    return 0;
}
