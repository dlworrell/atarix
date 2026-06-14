#include <stdio.h>

#include "atarix/sim_node.h"

int main(void) {
    atarix_sim_node_t node;
    int failures = 0;

    atarix_sim_node_init(&node, ATARIX_SIM_NODE_SUPERVISOR, "supervisor");

    if (node.node_id != ATARIX_SIM_NODE_SUPERVISOR) {
        printf("node_id got %u expected %u\n", node.node_id, ATARIX_SIM_NODE_SUPERVISOR);
        failures++;
    }

    if (node.name == 0 || node.name[0] != 's') {
        printf("node name not initialized\n");
        failures++;
    }

    if (node.tx_count != 0u || node.rx_count != 0u) {
        printf("counters not initialized to zero\n");
        failures++;
    }

    atarix_sim_node_record_tx(&node);
    atarix_sim_node_record_rx(&node);
    atarix_sim_node_record_rx(&node);

    if (node.tx_count != 1u) {
        printf("tx_count got %u expected 1\n", node.tx_count);
        failures++;
    }

    if (node.rx_count != 2u) {
        printf("rx_count got %u expected 2\n", node.rx_count);
        failures++;
    }

    atarix_sim_node_init(0, ATARIX_SIM_NODE_TEST_BASE, "null");
    atarix_sim_node_record_tx(0);
    atarix_sim_node_record_rx(0);

    return failures ? 1 : 0;
}
