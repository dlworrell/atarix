#include <stdio.h>
#include <string.h>

#include "atarix/mailbox.h"

int main(void) {
    atarix_mailbox_header_v1_t header;
    unsigned char payload[ATARIX_MAILBOX_MAX_PAYLOAD_V1];
    atarix_mailbox_status_t status;
    int failures = 0;

    memset(payload, 0xA5, sizeof(payload));

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_PING,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("max payload build failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("max payload validate failed: %d\n", (int)status);
        failures++;
    }

    header.length = ATARIX_MAILBOX_MAX_PAYLOAD_V1 + 1u;
    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_INVALID_LENGTH) {
        printf("oversized header got %d expected invalid length\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
