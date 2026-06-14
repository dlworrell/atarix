#include <stdio.h>

#include "atarix/sim_fabric.h"

int main(void) {
    atarix_sim_fabric_t fabric;
    atarix_sim_node_t node_a;
    atarix_sim_node_t node_b;
    atarix_sim_fabric_status_t status;
    int failures = 0;

    atarix_sim_fabric_init(&fabric);
    atarix_sim_node_init(&node_a, ATARIX_SIM_NODE_FABRIC_BASE, "fabric-a");
    atarix_sim_node_init(&node_b, ATARIX_SIM_NODE_FABRIC_BASE, "fabric-b");

    status = atarix_sim_fabric_attach(&fabric, &node_a);
    if (status != ATARIX_SIM_FABRIC_STATUS_OK) {
        printf("first attach got %d expected ok\n", (int)status);
        failures++;
    }

    status = atarix_sim_fabric_attach(&fabric, &node_b);
    if (status != ATARIX_SIM_FABRIC_STATUS_DUPLICATE_NODE) {
        printf("duplicate attach got %d expected duplicate\n", (int)status);
        failures++;
    }

    if (atarix_sim_fabric_node_count(&fabric) != 1u) {
        printf("node count got %lu expected 1\n", (unsigned long)atarix_sim_fabric_node_count(&fabric));
        failures++;
    }

    return failures ? 1 : 0;
}
