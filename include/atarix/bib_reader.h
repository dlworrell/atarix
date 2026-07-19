#ifndef ATARIX_BIB_READER_H
#define ATARIX_BIB_READER_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/bib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct atarix_bib_view {
    const uint8_t *buffer;
    uint32_t total_length;
    uint32_t tlv_offset;
    uint32_t tlv_count;
    uint16_t flags;
    uint64_t boot_id;
    uint64_t producer_id;
    uint64_t created_counter;
} atarix_bib_view_t;

typedef struct atarix_bib_record_view {
    uint16_t type;
    uint16_t flags;
    const uint8_t *payload;
    uint32_t payload_length;
    uint32_t serialized_length;
} atarix_bib_record_view_t;

typedef struct atarix_bib_reference_view {
    uint64_t object_id;
    uint64_t address;
    uint64_t length;
    uint32_t format_major;
    uint32_t format_minor;
    uint32_t integrity_kind;
    const uint8_t *integrity;
    uint32_t integrity_length;
} atarix_bib_reference_view_t;

atarix_bib_error_t atarix_bib_reader_open(
    atarix_bib_view_t *view,
    const void *buffer,
    size_t window_length);

atarix_bib_error_t atarix_bib_reader_record(
    const atarix_bib_view_t *view,
    uint32_t index,
    atarix_bib_record_view_t *record);

atarix_bib_error_t atarix_bib_reader_find_reference(
    const atarix_bib_view_t *view,
    uint16_t type,
    atarix_bib_reference_view_t *reference);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_BIB_READER_H */
