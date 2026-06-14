#include <stdint.h>
#include <stdio.h>

#include "atarix/sim_mailbox.h"

int main(void) {
    atarix_sim_node_t supervisor;
    atarix_sim_node_t fabric;
    atarix_sim_mailbox_endpoint_t supervisor_endpoint;
    atarix_sim_mailbox_endpoint_t fabric_endpoint;
    atarix_sim_mailbox_packet_t query_packet;
    atarix_sim_mailbox_packet_t reply_packet;
    const unsigned char version_reply[] = { 0u, 1u, 0u, 0u };
    atarix_mailbox_status_t status;
    int failures = 0;

    atarix_sim_node_init(&supervisor, ATARIX_SIM_NODE_SUPERVISOR, "supervisor");
    atarix_sim_node_init(&fabric, ATARIX_SIM_NODE_FABRIC_BASE, "fabric");
    atarix_sim_mailbox_endpoint_init(&supervisor_endpoint, &supervisor);
    atarix_sim_mailbox_endpoint_init(&fabric_endpoint, &fabric);

    status = atarix_sim_mailbox_send(&supervisor,
                                     &query_packet,
                                     ATARIX_MAILBOX_MESSAGE_VERSION_QUERY,
                                     2u,
                                     0,
                                     0u);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("version query send failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_sim_mailbox_receive(&fabric_endpoint, &query_packet);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("version query receive failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_sim_mailbox_send(&fabric,
                                     &reply_packet,
                                     ATARIX_MAILBOX_MESSAGE_VERSION_REPLY,
                                     2u,
                                     version_reply,
                                     (uint16_t)sizeof(version_reply));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("version reply send failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_sim_mailbox_receive(&supervisor_endpoint, &reply_packet);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("version reply receive failed: %d\n", (int)status);
        failures++;
    }

    if (query_packet.header.type != ATARIX_MAILBOX_MESSAGE_VERSION_QUERY) {
        printf("query type not preserved\n");
        failures++;
    }

    if (reply_packet.header.type != ATARIX_MAILBOX_MESSAGE_VERSION_REPLY) {
        printf("reply type not preserved\n");
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

    return failures ? 1 : 0;
}
