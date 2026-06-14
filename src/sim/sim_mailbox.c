#include <string.h>

#include "atarix/sim_mailbox.h"

void atarix_sim_mailbox_endpoint_init(
    atarix_sim_mailbox_endpoint_t *endpoint,
    atarix_sim_node_t *node) {
    if (!endpoint) {
        return;
    }

    endpoint->node = node;
    memset(&endpoint->rx_window, 0, sizeof(endpoint->rx_window));
}

atarix_mailbox_status_t atarix_sim_mailbox_send(
    atarix_sim_node_t *source,
    atarix_sim_mailbox_packet_t *packet,
    uint16_t type,
    uint32_t sequence,
    const void *payload,
    uint16_t payload_length) {
    atarix_mailbox_status_t status;

    if (!source || !packet || (!payload && payload_length != 0u)) {
        return ATARIX_MAILBOX_STATUS_NULL;
    }

    status = atarix_mailbox_build_header(&packet->header,
                                         type,
                                         sequence,
                                         payload,
                                         payload_length);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        return status;
    }

    packet->payload = payload;
    packet->payload_length = payload_length;
    atarix_sim_node_record_tx(source);

    return ATARIX_MAILBOX_STATUS_OK;
}

atarix_mailbox_status_t atarix_sim_mailbox_receive(
    atarix_sim_mailbox_endpoint_t *destination,
    const atarix_sim_mailbox_packet_t *packet) {
    atarix_mailbox_status_t status;

    if (!destination || !destination->node || !packet) {
        return ATARIX_MAILBOX_STATUS_NULL;
    }

    status = atarix_mailbox_validate_header(&packet->header,
                                            packet->payload,
                                            packet->payload_length);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        return status;
    }

    status = atarix_mailbox_sequence_accept(&destination->rx_window,
                                            packet->header.sequence);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        return status;
    }

    atarix_sim_node_record_rx(destination->node);
    return ATARIX_MAILBOX_STATUS_OK;
}
