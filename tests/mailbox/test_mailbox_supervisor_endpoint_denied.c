#include <stdio.h>

#include "atarix/mailbox.h"

int main(void) {
    atarix_mailbox_status_t status;
    int failures = 0;

    status = atarix_mailbox_ring_authorize(ATARIX_RING_APPLICATION,
                                           ATARIX_RING_SUPERVISOR,
                                           ATARIX_MAILBOX_MESSAGE_STATUS_QUERY);
    if (status != ATARIX_MAILBOX_STATUS_RING_DENIED) {
        printf("application to supervisor mailbox got %d expected ring denied\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_ring_authorize(ATARIX_RING_DRIVER,
                                           ATARIX_RING_SUPERVISOR,
                                           ATARIX_MAILBOX_MESSAGE_STATUS_QUERY);
    if (status != ATARIX_MAILBOX_STATUS_RING_DENIED) {
        printf("driver to supervisor mailbox got %d expected ring denied\n", (int)status);
        failures++;
    }

    status = atarix_mailbox_ring_authorize(ATARIX_RING_SUPERVISOR,
                                           ATARIX_RING_DRIVER,
                                           ATARIX_MAILBOX_MESSAGE_STATUS_QUERY);
    if (status != ATARIX_MAILBOX_STATUS_OK) {
        printf("supervisor to driver mailbox got %d expected ok\n", (int)status);
        failures++;
    }

    return failures ? 1 : 0;
}
