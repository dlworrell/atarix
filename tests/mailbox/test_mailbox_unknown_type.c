#include <stdio.h>

#include "atarix/mailbox.h"

int main(void) {
    atarix_mailbox_header_v1_t header;
    const unsigned char payload[] = { 0u };
    atarix_mailbox_status_t status;
    int failures = 0;

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_INVALID,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE) {
        printf("invalid type build got %d expected unknown type\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_PING,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("valid build failed: %d\n", (int)status);
        failures++;
    }

    header.type = 0x7FFFu;
    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE) {
        printf("unknown type validate got %d expected unknown type\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
