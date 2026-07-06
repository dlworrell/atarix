#include <stdio.h>
#include <string.h>

#include "atarix/mailbox.h"

int main(void) {
    const char payload[] = "ATARIX";
    atarix_mailbox_header_v1_t header;
    atarix_mailbox_status_t status;
    char corrupt[sizeof(payload)];
    int failures = 0;

    status = atarix_mailbox_build_header(&header,
                                         ATARIX_MAILBOX_MESSAGE_PING,
                                         1u,
                                         payload,
                                         (uint16_t)sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("build header failed: %d\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("valid payload rejected: %d\n", (int)status);
        failures++;
    }

    memcpy(corrupt, payload, sizeof(payload)); /* aes-sec-001: allow test fixture copy between same-sized local arrays */
    corrupt[0] ^= 0x01;
    status = atarix_mailbox_validate_header(&header, corrupt, sizeof(corrupt));
    if (status != ATARIX_MAILBOX_STATUS_CRC_MISMATCH) {
        printf("corrupt payload status %d expected CRC mismatch\n", (int)status);
        failures++;
    }

    header.crc32 ^= 0x01u;
    status = atarix_mailbox_validate_header(&header, payload, sizeof(payload));
    if (status != ATARIX_MAILBOX_STATUS_CRC_MISMATCH) {
        printf("corrupt crc status %d expected CRC mismatch\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
