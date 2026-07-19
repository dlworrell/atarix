#include <stdint.h>
#include <stdio.h>

#include "atarix/bib_reader.h"
#include "atarix/sim_bib.h"

static int fail(const char *name, long long actual, long long expected) {
    if (actual != expected) {
        fprintf(stderr, "%s: expected %lld, got %lld\n", name, expected, actual);
        return 1;
    }
    return 0;
}

int main(void) {
    uint8_t capture[ATARIX_SIM_BIB_CAPTURE_SIZE];
    uint32_t length = 0u;
    atarix_bib_view_t view;
    atarix_bib_reference_view_t reference;
    const atarix_sim_bib_capture_config_t config = {
        ATARIX_BIB_FLAG_DIAGNOSTIC_BOOT,
        UINT64_C(0x454D554C41544F52),
        UINT64_C(0x4154415249580001),
        UINT64_C(1),
        UINT64_C(0x1001),
        UINT64_C(0x00100000),
        UINT64_C(0x00002000),
        UINT64_C(0x1002),
        UINT64_C(0x00200000),
        UINT64_C(0x00003000)
    };
    int failures = 0;

    failures += fail("capture",
        atarix_sim_bib_capture(&config, capture, sizeof(capture), &length),
        ATARIX_BIB_BUILDER_OK);
    failures += fail("capture length", length, ATARIX_SIM_BIB_CAPTURE_SIZE);
    failures += fail("decode",
        atarix_bib_reader_open(&view, capture, length),
        ATARIX_BIB_OK);
    failures += fail("boot id", (long long)view.boot_id, (long long)config.boot_id);
    failures += fail("producer id", (long long)view.producer_id, (long long)config.producer_id);
    failures += fail("counter", (long long)view.created_counter, 1);
    failures += fail("records", view.tlv_count, 2);

    failures += fail("service",
        atarix_bib_reader_find_reference(
            &view, ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE, &reference),
        ATARIX_BIB_OK);
    failures += fail("service object", (long long)reference.object_id, 0x1001);
    failures += fail("service address", (long long)reference.address, 0x00100000);

    failures += fail("memory",
        atarix_bib_reader_find_reference(
            &view, ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE, &reference),
        ATARIX_BIB_OK);
    failures += fail("memory object", (long long)reference.object_id, 0x1002);
    failures += fail("memory address", (long long)reference.address, 0x00200000);

    return failures == 0 ? 0 : 1;
}
