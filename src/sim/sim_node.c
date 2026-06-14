#include "atarix/sim_node.h"

void atarix_sim_node_init(
    atarix_sim_node_t *node,
    uint32_t node_id,
    const char *name) {
    if (!node) {
        return;
    }

    node->node_id = node_id;
    node->name = name;
    node->tx_count = 0u;
    node->rx_count = 0u;
}

void atarix_sim_node_record_tx(
    atarix_sim_node_t *node) {
    if (!node) {
        return;
    }

    node->tx_count++;
}

void atarix_sim_node_record_rx(
    atarix_sim_node_t *node) {
    if (!node) {
        return;
    }

    node->rx_count++;
}
