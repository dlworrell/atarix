#include <stdint.h>
#include "atarix/discovery_parser.h"

static int atarix_is_aligned8(uintptr_t value) {
    return (value & 7u) == 0u;
}

const atarix_discovery_record_header_v1_t *atarix_discovery_first_record(
    const atarix_discovery_header_v1_t *header,
    size_t image_size) {
    const uint8_t *base;

    if (!header || image_size < ATARIX_DISCOVERY_HEADER_SIZE_V1) {
        return 0;
    }

    base = (const uint8_t *)header;
    return (const atarix_discovery_record_header_v1_t *)(base + ATARIX_DISCOVERY_HEADER_SIZE_V1);
}

const atarix_discovery_record_header_v1_t *atarix_discovery_next_record(
    const atarix_discovery_header_v1_t *header,
    size_t image_size,
    const atarix_discovery_record_header_v1_t *record) {
    const uint8_t *base;
    const uint8_t *end;
    const uint8_t *next;

    if (!header || !record) {
        return 0;
    }

    base = (const uint8_t *)header;
    end = base + image_size;

    if (record->length < ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1) {
        return 0;
    }

    next = ((const uint8_t *)record) + record->length;

    if (next >= end) {
        return 0;
    }

    if (!atarix_is_aligned8((uintptr_t)(next - base))) {
        return 0;
    }

    return (const atarix_discovery_record_header_v1_t *)next;
}
