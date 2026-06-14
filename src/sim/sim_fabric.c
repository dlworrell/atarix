#include <string.h>

#include "atarix/sim_fabric.h"

void atarix_sim_fabric_init(
    atarix_sim_fabric_t *fabric) {
    if (!fabric) {
        return;
    }

    memset(fabric, 0, sizeof(*fabric));
}

atarix_sim_fabric_status_t atarix_sim_fabric_attach(
    atarix_sim_fabric_t *fabric,
    atarix_sim_node_t *node) {
    size_t i;

    if (!fabric || !node) {
        return ATARIX_SIM_FABRIC_STATUS_NULL;
    }

    for (i = 0u; i < fabric->node_count; ++i) {
        if (fabric->slots[i].node_id == node->node_id) {
            return ATARIX_SIM_FABRIC_STATUS_DUPLICATE_NODE;
        }
    }

    if (fabric->node_count >= ATARIX_SIM_FABRIC_MAX_NODES) {
        return ATARIX_SIM_FABRIC_STATUS_FULL;
    }

    fabric->slots[fabric->node_count].node_id = node->node_id;
    fabric->slots[fabric->node_count].node = node;
    fabric->node_count++;

    return ATARIX_SIM_FABRIC_STATUS_OK;
}

atarix_sim_node_t *atarix_sim_fabric_lookup(
    atarix_sim_fabric_t *fabric,
    uint32_t node_id) {
    size_t i;

    if (!fabric) {
        return 0;
    }

    for (i = 0u; i < fabric->node_count; ++i) {
        if (fabric->slots[i].node_id == node_id) {
            return fabric->slots[i].node;
        }
    }

    return 0;
}

size_t atarix_sim_fabric_node_count(
    const atarix_sim_fabric_t *fabric) {
    if (!fabric) {
        return 0u;
    }

    return fabric->node_count;
}
