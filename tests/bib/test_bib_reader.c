#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "atarix/bib_builder.h"
#include "atarix/bib_reader.h"

#define BUFFER_SIZE 256u

static int fail(const char *name, long long actual, long long expected) {
    if (actual != expected) {
        fprintf(stderr, "%s: expected %lld, got %lld\n", name, expected, actual);
        return 1;
    }
    return 0;
}

static int build(uint8_t *buffer, uint32_t *length) {
    atarix_bib_builder_t builder;
    int failures = 0;

    failures += fail("init",
        atarix_bib_builder_init(&builder, buffer, BUFFER_SIZE,
            ATARIX_BIB_FLAG_DIAGNOSTIC_BOOT,
            UINT64_C(0x1122334455667788),
            UINT64_C(0x8877665544332211), 7u),
        ATARIX_BIB_BUILDER_OK);
    failures += fail("service",
        atarix_bib_builder_add_reference(&builder,
            ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE,
            ATARIX_BIB_TLV_FLAG_MANDATORY,
            UINT64_C(0x1001), UINT64_C(0x100000), UINT64_C(0x2000),
            1u, 0u),
        ATARIX_BIB_BUILDER_OK);
    failures += fail("memory",
        atarix_bib_builder_add_reference(&builder,
            ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE,
            ATARIX_BIB_TLV_FLAG_MANDATORY,
            UINT64_C(0x1002), UINT64_C(0x200000), UINT64_C(0x3000),
            1u, 0u),
        ATARIX_BIB_BUILDER_OK);
    failures += fail("seal",
        atarix_bib_builder_seal(&builder, length),
        ATARIX_BIB_BUILDER_OK);
    return failures;
}

int main(void) {
    uint8_t buffer[BUFFER_SIZE];
    uint8_t corrupt[BUFFER_SIZE];
    uint32_t length = 0;
    atarix_bib_view_t view;
    atarix_bib_record_view_t record;
    atarix_bib_reference_view_t reference;
    int failures = build(buffer, &length);

    failures += fail("serialized length", length, 176u);
    failures += fail("open", atarix_bib_reader_open(&view, buffer, length), ATARIX_BIB_OK);
    failures += fail("flags", view.flags, ATARIX_BIB_FLAG_DIAGNOSTIC_BOOT);
    failures += fail("boot id", (long long)view.boot_id, (long long)UINT64_C(0x1122334455667788));
    failures += fail("producer id", (long long)view.producer_id, (long long)UINT64_C(0x8877665544332211));
    failures += fail("counter", (long long)view.created_counter, 7);
    failures += fail("record count", view.tlv_count, 2);

    failures += fail("record zero", atarix_bib_reader_record(&view, 0u, &record), ATARIX_BIB_OK);
    failures += fail("record zero type", record.type, ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE);
    failures += fail("record zero payload", record.payload_length, ATARIX_BIB_REFERENCE_FIXED_SIZE_V1);
    failures += fail("record out of range", atarix_bib_reader_record(&view, 2u, &record), ATARIX_BIB_E_TLV_BOUNDS);

    failures += fail("find service",
        atarix_bib_reader_find_reference(&view,
            ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE, &reference),
        ATARIX_BIB_OK);
    failures += fail("service object", (long long)reference.object_id, 0x1001);
    failures += fail("service address", (long long)reference.address, 0x100000);
    failures += fail("service length", (long long)reference.length, 0x2000);
    failures += fail("service format", reference.format_major, 1);
    failures += fail("service integrity length", reference.integrity_length, 0);

    failures += fail("find memory",
        atarix_bib_reader_find_reference(&view,
            ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE, &reference),
        ATARIX_BIB_OK);
    failures += fail("memory object", (long long)reference.object_id, 0x1002);
    failures += fail("memory address", (long long)reference.address, 0x200000);
    failures += fail("memory length", (long long)reference.length, 0x3000);

    failures += fail("truncated", atarix_bib_reader_open(&view, buffer, length - 1u), ATARIX_BIB_E_TOTAL_LENGTH);

    memcpy(corrupt, buffer, length);
    corrupt[100] ^= 1u;
    failures += fail("corrupt CRC", atarix_bib_reader_open(&view, corrupt, length), ATARIX_BIB_E_CRC);

    memcpy(corrupt, buffer, length);
    corrupt[64] = 0x00u;
    corrupt[65] = 0x70u;
    corrupt[20] = corrupt[21] = corrupt[22] = corrupt[23] = 0u;
    failures += fail("changed record invalidates CRC",
        atarix_bib_reader_open(&view, corrupt, length), ATARIX_BIB_E_CRC);

    return failures == 0 ? 0 : 1;
}
