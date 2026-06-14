#include <stdio.h>
#include <string.h>

#include "atarix/mailbox.h"

int main(void) {
    atarix_mailbox_sequence_window_t window;
    atarix_mailbox_status_t status;
    int failures = 0;

    memset(&window, 0, sizeof(window));

    status = atarix_mailbox_sequence_accept(&window, 100u);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("initial sequence rejected: %d\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_sequence_accept(&window, 99u);
    if (status != ATARIX_MAILBOX_STATUS_OUT_OF_ORDER) {
        printf("old sequence got %d expected out-of-order\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_sequence_accept(&window, 102u);
    if (status != ATARIX_MAILBOX_STATUS_SEQUENCE_ERROR) {
        printf("sequence gap got %d expected sequence error\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_sequence_accept(&window, 101u);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("next sequence after gap rejected: %d\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
