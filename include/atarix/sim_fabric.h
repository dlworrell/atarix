#ifndef ATARIX_SIM_FABRIC_H
#define ATARIX_SIM_FABRIC_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/sim_node.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_SIM_FABRIC_MAX_NODES 256u

typedef enum atarix_sim_fabric_status {
    ATARIX_SIM_FABRIC_STATUS_OK = 0,
    ATARIX_SIM_FABRIC_STATUS_NULL,
    ATARIX_SIM_FABRIC_STATUS_DUPLICATE_NODE,
    ATARIX_SIM_FABRIC_STATUS_FULL,
    ATARIX_SIM_FABRIC_STATUS_NOT_FOUND
} atarix_sim_fabric_status_t;

typedef struct atarix_sim_fabric_slot {
    uint32_t node_id;
    atarix_sim_node_t *node;
} atarix_sim_fabric_slot_t;

typedef struct atarix_sim_fabric {
    atarix_sim_fabric_slot_t slots[ATARIX_SIM_FABRIC_MAX_NODES];
    size_t node_count;
} atarix_sim_fabric_t;

void atarix_sim_fabric_init(
    atarix_sim_fabric_t *fabric);

atarix_sim_fabric_status_t atarix_sim_fabric_attach(
    atarix_sim_fabric_t *fabric,
    atarix_sim_node_t *node);

atarix_sim_node_t *atarix_sim_fabric_lookup(
    atarix_sim_fabric_t *fabric,
    uint32_t node_id);

size_t atarix_sim_fabric_node_count(
    const atarix_sim_fabric_t *fabric);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_SIM_FABRIC_H */
