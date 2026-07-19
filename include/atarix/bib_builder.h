#ifndef ATARIX_BIB_BUILDER_H
#define ATARIX_BIB_BUILDER_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/bib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum atarix_bib_builder_result {
    ATARIX_BIB_BUILDER_OK = 0,
    ATARIX_BIB_BUILDER_E_ARGUMENT,
    ATARIX_BIB_BUILDER_E_CAPACITY,
    ATARIX_BIB_BUILDER_E_STATE,
    ATARIX_BIB_BUILDER_E_FLAGS,
    ATARIX_BIB_BUILDER_E_LENGTH
} atarix_bib_builder_result_t;

typedef struct atarix_bib_builder {
    uint8_t *buffer;
    size_t capacity;
    uint32_t length;
    uint32_t tlv_count;
    int sealed;
} atarix_bib_builder_t;

atarix_bib_builder_result_t atarix_bib_builder_init(
    atarix_bib_builder_t *builder,
    void *buffer,
    size_t capacity,
    uint16_t header_flags,
    uint64_t boot_id,
    uint64_t producer_id,
    uint64_t created_counter);

atarix_bib_builder_result_t atarix_bib_builder_add_tlv(
    atarix_bib_builder_t *builder,
    uint16_t type,
    uint16_t flags,
    const void *payload,
    uint32_t payload_length);

/* Adds the fixed 40-byte reference payload with no following integrity data. */
atarix_bib_builder_result_t atarix_bib_builder_add_reference(
    atarix_bib_builder_t *builder,
    uint16_t type,
    uint16_t flags,
    uint64_t object_id,
    uint64_t address,
    uint64_t length,
    uint32_t format_major,
    uint32_t format_minor);

atarix_bib_builder_result_t atarix_bib_builder_seal(
    atarix_bib_builder_t *builder,
    uint32_t *serialized_length);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_BIB_BUILDER_H */
