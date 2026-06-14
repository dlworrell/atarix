#include <stdio.h>

#include "atarix/sim_fabric.h"

int main(void) {
    atarix_sim_fabric_t fabric;
    atarix_sim_node_t supervisor;
    atarix_sim_fabric_status_t status;
    atarix_sim_node_t *found;
    int failures = 0;

    atarix_sim_fabric_init(&fabric);
    atarix_sim_node_init(&supervisor, ATARIX_SIM_NODE_SUPERVISOR, "supervisor");

    if (atarix_sim_fabric_node_count(&fabric) != 0u) {
        printf("initial node count not zero\n");
        failures++;
    }

    status = atarix_sim_fabric_attach(&fabric, &supervisor);
    if (status != ATARIX_SIM_FABRIC_STATUS_OK) {
        printf("attach got %d expected ok\n", (int)status);
        failures++;
    }

    if (atarix_sim_fabric_node_count(&fabric) != 1u) {
        printf("node count got %lu expected 1\n", (unsigned long)atarix_sim_fabric_node_count(&fabric));
        failures++;
    }

    found = atarix_sim_fabric_lookup(&fabric, ATARIX_SIM_NODE_SUPERVISOR);
    if (found != &supervisor) {
        printf("lookup did not return attached supervisor\n");
        failures++;
    }

    return failures ? 1 : 0;
}
