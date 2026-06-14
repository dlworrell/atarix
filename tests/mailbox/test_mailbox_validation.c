#include <stdio.h>

#include "atarix/mailbox.h"

int main(void) {
    atarix_mailbox_header_v1_t header;
    const unsigned char payload[] = { 1u, 2u, 3u, 4u };
    atarix_mailbox_status_t status;
    int failures = 0;

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_STATUS_QUERY,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("valid build failed: %d\n", (int)status);
        failures++;
    }

    header.magic = 0u;
    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_INVALID_HEADER) {
        printf("bad magic got %d expected invalid header\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_STATUS_QUERY,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    header.type = ATARIX_MAILBOX_MESSAGE_INVALID;
    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE) {
        printf("bad type got %d expected unknown type\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_STATUS_QUERY,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    header.length = (uint16_t)(sizeof(payload) + 1u);
    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_INVALID_LENGTH) {
        printf("bad length got %d expected invalid length\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_INVALID,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE) {
        printf("unknown type build got %d expected unknown type\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
