#include <stdio.h>

#include "atarix/sim_fabric.h"

int main(void) {
    atarix_sim_fabric_t fabric;
    atarix_sim_node_t nodes[ATARIX_SIM_FABRIC_MAX_NODES + 1u];
    atarix_sim_fabric_status_t status;
    size_t i;
    int failures = 0;

    atarix_sim_fabric_init(&fabric);

    for (i = 0u; i < ATARIX_SIM_FABRIC_MAX_NODES; ++i) {
        atarix_sim_node_init(&nodes[i], ATARIX_SIM_NODE_TEST_BASE + (uint32_t)i, "node");
        status = atarix_sim_fabric_attach(&fabric, &nodes[i]);
        if (status != ATARIX_SIM_FABRIC_STATUS_OK) {
            printf("attach %lu got %d expected ok\n", (unsigned long)i, (int)status);
            failures++;
            break;
        }
    }

    if (atarix_sim_fabric_node_count(&fabric) != ATARIX_SIM_FABRIC_MAX_NODES) {
        printf("node count got %lu expected %u\n",
               (unsigned long)atarix_sim_fabric_node_count(&fabric),
               (unsigned int)ATARIX_SIM_FABRIC_MAX_NODES);
        failures++;
    }

    atarix_sim_node_init(&nodes[ATARIX_SIM_FABRIC_MAX_NODES],
                         ATARIX_SIM_NODE_TEST_BASE + ATARIX_SIM_FABRIC_MAX_NODES,
                         "overflow");
    status = atarix_sim_fabric_attach(&fabric, &nodes[ATARIX_SIM_FABRIC_MAX_NODES]);
    if (status != ATARIX_SIM_FABRIC_STATUS_FULL) {
        printf("overflow attach got %d expected full\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
