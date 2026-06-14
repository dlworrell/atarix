#include <stdio.h>

#include "atarix/sim_mailbox.h"

int main(void) {
    atarix_sim_node_t supervisor;
    atarix_sim_node_t fabric;
    atarix_sim_mailbox_endpoint_t supervisor_endpoint;
    atarix_sim_mailbox_endpoint_t fabric_endpoint;
    atarix_sim_mailbox_packet_t request_packet;
    atarix_sim_mailbox_packet_t reply_packet;
    const unsigned char request_payload[] = { 1u, 2u, 3u, 4u };
    const unsigned char reply_payload[] = { 5u, 6u, 7u, 8u };
    atarix_mailbox_status_t status;
    int failures = 0;

    atarix_sim_node_init(&supervisor, ATARIX_SIM_NODE_SUPERVISOR, "supervisor");
    atarix_sim_node_init(&fabric, ATARIX_SIM_NODE_FABRIC_BASE, "fabric");
    atarix_sim_mailbox_endpoint_init(&supervisor_endpoint, &supervisor);
    atarix_sim_mailbox_endpoint_init(&fabric_endpoint, &fabric);

    status = atarix_sim_mailbox_send(&supervisor,
                                     &request_packet,
                                     ATARIX_MAILBOX_MESSAGE_PING,
                                     1u,
                                     request_payload,
                                     (uint16_t)sizeof(request_payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("supervisor send failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_sim_mailbox_receive(&fabric_endpoint, &request_packet);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("fabric receive failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_sim_mailbox_send(&fabric,
                                     &reply_packet,
                                     ATARIX_MAILBOX_MESSAGE_PONG,
                                     1u,
                                     reply_payload,
                                     (uint16_t)sizeof(reply_payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("fabric send failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_sim_mailbox_receive(&supervisor_endpoint, &reply_packet);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("supervisor receive failed: %d\n", (int)status);
        failures++;
    }

    if (supervisor.tx_count != 1u || supervisor.rx_count != 1u) {
        printf("supervisor counters tx=%u rx=%u expected 1/1\n",
               supervisor.tx_count,
               supervisor.rx_count);
        failures++;
    }

    if (fabric.tx_count != 1u || fabric.rx_count != 1u) {
        printf("fabric counters tx=%u rx=%u expected 1/1\n",
               fabric.tx_count,
               fabric.rx_count);
        failures++;
    }

    status = atarix_sim_mailbox_receive(&fabric_endpoint, &request_packet);
    if (status != ATARIX_MAILBOX_STATUS_DUPLICATE) {
        printf("duplicate receive got %d expected duplicate\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
