#include <stdio.h>
#include <string.h>

#include "atarix/mailbox.h"

int main(void) {
    atarix_mailbox_sequence_window_t window;
    atarix_mailbox_status_t status;
    int failures = 0;

    memset(&window, 0, sizeof(window));

    status = atarix_mailbox_sequence_accept(&window, 7u);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("initial sequence rejected: %d\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_sequence_accept(&window, 7u);
    if (status != ATARIX_MAILBOX_STATUS_DUPLICATE) {
        printf("duplicate sequence got %d expected duplicate\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
