#include <stdio.h>
#include <string.h>

#include "atarix/mailbox.h"

static int expect_sequence(const char *name,
                           atarix_mailbox_sequence_window_t *window,
                           uint32_t sequence,
                           atarix_mailbox_status_t expected) {
    atarix_mailbox_status_t status = atarix_mailbox_sequence_accept(window, sequence);
    if (status != expected) {
        printf("%s: got %d expected %d\n", name, (int)status, (int)expected);
        return 1;
    }
    return 0;
}

int main(void) {
    atarix_mailbox_sequence_window_t window;
    int failures = 0;

    memset(&window, 0, sizeof(window));

    failures += expect_sequence("first sequence accepted", &window, 10u, ATARIX_MAILBOX_STATUS_OK);
    failures += expect_sequence("next sequence accepted", &window, 11u, ATARIX_MAILBOX_STATUS_OK);
    failures += expect_sequence("duplicate rejected", &window, 11u, ATARIX_MAILBOX_STATUS_DUPLICATE);
    failures += expect_sequence("old sequence rejected", &window, 10u, ATARIX_MAILBOX_STATUS_OUT_OF_ORDER);
    failures += expect_sequence("gap rejected", &window, 13u, ATARIX_MAILBOX_STATUS_SEQUENCE_ERROR);
    failures += expect_sequence("recovery next accepted", &window, 12u, ATARIX_MAILBOX_STATUS_OK);

    return failures ? 1 : 0;
}
