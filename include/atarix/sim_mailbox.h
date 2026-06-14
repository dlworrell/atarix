#ifndef ATARIX_SIM_MAILBOX_H
#define ATARIX_SIM_MAILBOX_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/mailbox.h"
#include "atarix/sim_node.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct atarix_sim_mailbox_packet {
    atarix_mailbox_header_v1_t header;
    const void *payload;
    size_t payload_length;
} atarix_sim_mailbox_packet_t;

typedef struct atarix_sim_mailbox_endpoint {
    atarix_sim_node_t *node;
    atarix_mailbox_sequence_window_t rx_window;
} atarix_sim_mailbox_endpoint_t;

void atarix_sim_mailbox_endpoint_init(
    atarix_sim_mailbox_endpoint_t *endpoint,
    atarix_sim_node_t *node);

atarix_mailbox_status_t atarix_sim_mailbox_send(
    atarix_sim_node_t *source,
    atarix_sim_mailbox_packet_t *packet,
    uint16_t type,
    uint32_t sequence,
    const void *payload,
    uint16_t payload_length);

atarix_mailbox_status_t atarix_sim_mailbox_receive(
    atarix_sim_mailbox_endpoint_t *destination,
    const atarix_sim_mailbox_packet_t *packet);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_SIM_MAILBOX_H */
