#ifndef ATARIX_SIM_BIB_H
#define ATARIX_SIM_BIB_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/bib_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_SIM_BIB_CAPTURE_SIZE 176u

typedef struct atarix_sim_bib_capture_config {
    uint16_t header_flags;
    uint64_t boot_id;
    uint64_t producer_id;
    uint64_t created_counter;
    uint64_t service_object_id;
    uint64_t service_address;
    uint64_t service_length;
    uint64_t memory_object_id;
    uint64_t memory_address;
    uint64_t memory_length;
} atarix_sim_bib_capture_config_t;

atarix_bib_builder_result_t atarix_sim_bib_capture(
    const atarix_sim_bib_capture_config_t *config,
    void *buffer,
    size_t capacity,
    uint32_t *serialized_length);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_SIM_BIB_H */
