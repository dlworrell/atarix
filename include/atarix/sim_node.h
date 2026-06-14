#ifndef ATARIX_SIM_NODE_H
#define ATARIX_SIM_NODE_H

#include <stdint.h>

#include "atarix/sim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct atarix_sim_node {
    uint32_t node_id;
    const char *name;
    uint32_t tx_count;
    uint32_t rx_count;
} atarix_sim_node_t;

void atarix_sim_node_init(
    atarix_sim_node_t *node,
    uint32_t node_id,
    const char *name);

void atarix_sim_node_record_tx(
    atarix_sim_node_t *node);

void atarix_sim_node_record_rx(
    atarix_sim_node_t *node);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_SIM_NODE_H */
