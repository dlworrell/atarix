#include <stdio.h>

#include "atarix/sim_fabric.h"

int main(void) {
    atarix_sim_fabric_t fabric;
    atarix_sim_node_t supervisor;
    int failures = 0;

    atarix_sim_fabric_init(&fabric);
    atarix_sim_node_init(&supervisor, ATARIX_SIM_NODE_SUPERVISOR, "supervisor");

    if (atarix_sim_fabric_lookup(&fabric, ATARIX_SIM_NODE_FABRIC_BASE) != 0) {
        printf("empty fabric lookup should be null\n");
        failures++;
    }

    if (atarix_sim_fabric_attach(&fabric, &supervisor) != ATARIX_SIM_FABRIC_STATUS_OK) {
        printf("attach supervisor failed\n");
        failures++;
    }

    if (atarix_sim_fabric_lookup(&fabric, ATARIX_SIM_NODE_FABRIC_BASE) != 0) {
        printf("missing fabric node lookup should be null\n");
        failures++;
    }

    return failures ? 1 : 0;
}
