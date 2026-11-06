/* SPDX-FileCopyrightText: Donovan Worrell */
/* SPDX-License-Identifier: Apache-2.0 */
/* ATX-SPEC-005 lifecycle admission: only ACTIVE accepts; fail closed. */
#include <stdio.h>

#include "atarix/mailbox.h"

int main(void) {
    static const struct {
        atarix_mailbox_endpoint_state_t state;
        atarix_mailbox_status_t want;
    } cases[] = {
        {ATARIX_MAILBOX_ENDPOINT_CREATED, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE},
        {ATARIX_MAILBOX_ENDPOINT_ACTIVE, ATARIX_MAILBOX_STATUS_OK},
        {ATARIX_MAILBOX_ENDPOINT_SUSPENDED, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE},
        {ATARIX_MAILBOX_ENDPOINT_DRAINING, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE},
        {ATARIX_MAILBOX_ENDPOINT_CLOSING, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE},
        {ATARIX_MAILBOX_ENDPOINT_CLOSED, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE},
        {ATARIX_MAILBOX_ENDPOINT_QUARANTINED, ATARIX_MAILBOX_STATUS_QUARANTINED},
        {ATARIX_MAILBOX_ENDPOINT_FAILED, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE},
        {(atarix_mailbox_endpoint_state_t)42, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE},
    };
    int failures = 0;
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (atarix_mailbox_endpoint_admit(cases[i].state) != cases[i].want) {
            printf("state %d admitted as %d, expected %d\n", (int)cases[i].state,
                   (int)atarix_mailbox_endpoint_admit(cases[i].state), (int)cases[i].want);
            failures++;
        }
    }
    return failures ? 1 : 0;
}
