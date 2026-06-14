#include <stdio.h>

#include "atarix/sim_mailbox.h"

static void expect_status(const char *label,
                          atarix_mailbox_status_t got,
                          atarix_mailbox_status_t expected,
                          int *failures) {
    if (got != expected) {
        printf("%s got %d expected %d\n", label, (int)got, (int)expected);
        (*failures)++;
    }
}

int main(void) {
    atarix_sim_node_t supervisor;
    atarix_sim_node_t fabric;
    atarix_sim_mailbox_endpoint_t fabric_endpoint;
    atarix_sim_mailbox_packet_t packet;
    const unsigned char payload[] = { 1u, 2u, 3u, 4u };
    atarix_mailbox_status_t status;
    int failures = 0;

    atarix_sim_node_init(&supervisor, ATARIX_SIM_NODE_SUPERVISOR, "supervisor");
    atarix_sim_node_init(&fabric, ATARIX_SIM_NODE_FABRIC_BASE, "fabric");
    atarix_sim_mailbox_endpoint_init(&fabric_endpoint, &fabric);

    status = atarix_sim_mailbox_send(&supervisor,
                                     &packet,
                                     ATARIX_MAILBOX_MESSAGE_PING,
                                     1u,
                                     payload,
                                     (uint16_t)sizeof(payload));
    expect_status("valid send", status, ATARIX_MAILBOX_STATUS_OK, &failures);

    packet.header.crc32 ^= 1u;
    status = atarix_sim_mailbox_receive(&fabric_endpoint, &packet);
    expect_status("bad crc", status, ATARIX_MAILBOX_STATUS_CRC_MISMATCH, &failures);

    status = atarix_sim_mailbox_send(&supervisor,
                                     &packet,
                                     ATARIX_MAILBOX_MESSAGE_PING,
                                     1u,
                                     payload,
                                     (uint16_t)sizeof(payload));
    expect_status("valid resend", status, ATARIX_MAILBOX_STATUS_OK, &failures);

    packet.header.type = 0x7FFFu;
    status = atarix_sim_mailbox_receive(&fabric_endpoint, &packet);
    expect_status("unknown type", status, ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE, &failures);

    status = atarix_sim_mailbox_send(&supervisor,
                                     &packet,
                                     ATARIX_MAILBOX_MESSAGE_PING,
                                     1u,
                                     payload,
                                     (uint16_t)sizeof(payload));
    expect_status("valid send before duplicate", status, ATARIX_MAILBOX_STATUS_OK, &failures);

    status = atarix_sim_mailbox_receive(&fabric_endpoint, &packet);
    expect_status("first receive", status, ATARIX_MAILBOX_STATUS_OK, &failures);

    status = atarix_sim_mailbox_receive(&fabric_endpoint, &packet);
    expect_status("duplicate receive", status, ATARIX_MAILBOX_STATUS_DUPLICATE, &failures);

    status = atarix_sim_mailbox_send(&supervisor,
                                     &packet,
                                     ATARIX_MAILBOX_MESSAGE_PING,
                                     0u,
                                     payload,
                                     (uint16_t)sizeof(payload));
    expect_status("valid old send", status, ATARIX_MAILBOX_STATUS_OK, &failures);

    status = atarix_sim_mailbox_receive(&fabric_endpoint, &packet);
    expect_status("out of order receive", status, ATARIX_MAILBOX_STATUS_OUT_OF_ORDER, &failures);

    status = atarix_sim_mailbox_send(&supervisor,
                                     &packet,
                                     ATARIX_MAILBOX_MESSAGE_PING,
                                     2u,
                                     payload,
                                     (uint16_t)sizeof(payload));
    expect_status("valid send before invalid length", status, ATARIX_MAILBOX_STATUS_OK, &failures);

    packet.header.length = ATARIX_MAILBOX_MAX_PAYLOAD_V1 + 1u;
    status = atarix_sim_mailbox_receive(&fabric_endpoint, &packet);
    expect_status("invalid length", status, ATARIX_MAILBOX_STATUS_INVALID_LENGTH, &failures);

    if (fabric.rx_count != 1u) {
        printf("fabric rx_count got %u expected 1\n", fabric.rx_count);
        failures++;
    }

    return failures ? 1 : 0;
}
