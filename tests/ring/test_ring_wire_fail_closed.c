/* SPDX-FileCopyrightText: Donovan Worrell */
/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Every ring has exactly one wire value and name; an out-of-range ring maps
 * to ATARIX_RING_WIRE_INVALID (not to a real ring), and neither an invalid
 * nor a quarantined requester is admitted by any capability range.
 */
#include <stdio.h>
#include <string.h>

#include "atarix/capability_engine.h"
#include "atarix/ring.h"

int main(void) {
    int failures = 0;
    static const struct {
        atarix_ring_t ring;
        uint16_t wire;
        const char *name;
    } rings[] = {
        {ATARIX_RING_SUPERVISOR, 0xFFFEu, "supervisor"},
        {ATARIX_RING_FABRIC, 0xFFFFu, "fabric"},
        {ATARIX_RING_KERNEL, 0x0000u, "kernel"},
        {ATARIX_RING_DRIVER, 0x0001u, "driver"},
        {ATARIX_RING_USER_SERVICE, 0x0002u, "user-service"},
        {ATARIX_RING_APPLICATION, 0x0003u, "application"},
        {ATARIX_RING_QUARANTINE, 0x0004u, "quarantine"},
    };
    size_t i;

    for (i = 0; i < sizeof(rings) / sizeof(rings[0]); ++i) {
        if (atarix_ring_to_wire(rings[i].ring) != rings[i].wire) {
            printf("ring %d has the wrong wire value\n", (int)rings[i].ring);
            failures++;
        }
        if (strcmp(atarix_ring_name(rings[i].ring), rings[i].name) != 0) {
            printf("ring %d has the wrong name\n", (int)rings[i].ring);
            failures++;
        }
        if (!atarix_ring_is_valid(rings[i].ring)) {
            printf("ring %d should be valid\n", (int)rings[i].ring);
            failures++;
        }
    }

    if (ATARIX_RING_SERVICE != ATARIX_RING_USER_SERVICE) {
        printf("deprecated alias must equal the user-service ring\n");
        failures++;
    }

    {
        const atarix_ring_t bad[] = {(atarix_ring_t)-3, (atarix_ring_t)5, (atarix_ring_t)99};
        for (i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
            if (atarix_ring_to_wire(bad[i]) != ATARIX_RING_WIRE_INVALID) {
                printf("invalid ring %d must map to the invalid wire value\n", (int)bad[i]);
                failures++;
            }
            if (atarix_ring_is_valid(bad[i])) {
                printf("invalid ring %d reported valid\n", (int)bad[i]);
                failures++;
            }
            if (atarix_ring_can_access(bad[i], ATARIX_RING_APPLICATION,
                                       ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_INVALID_RING) {
                printf("invalid ring %d must not be admitted\n", (int)bad[i]);
                failures++;
            }
        }
    }

    /* A capability open to the widest legal range (kernel..application)
     * must still refuse an invalid or quarantined requester. */
    {
        atarix_capability_record_v1_t cap = {0};
        atarix_capability_request_t req = {0};
        atarix_capability_evaluation_t eval;
        const atarix_ring_t refused[] = {(atarix_ring_t)5, (atarix_ring_t)-7, ATARIX_RING_QUARANTINE};

        cap.capability_id = 1;
        cap.operation_id = 1;
        cap.target_handle = 0x1234u;
        cap.minimum_ring = ATARIX_RING_WIRE_KERNEL;
        cap.maximum_ring = ATARIX_RING_WIRE_APPLICATION;

        /* Control: a kernel requester is admitted, so the refusals below
         * come from the requester's ring and nothing else. */
        req.target_handle = 0x1234u;
        req.operation_id = 1;
        req.trust_level = ATARIX_TRUST_TRUSTED;
        req.requester_ring = ATARIX_RING_KERNEL;
        if (atarix_capability_evaluate(&cap, 1, &req, &eval) != 0 || eval.decision != ATARIX_DECISION_ALLOW) {
            printf("control: kernel requester should be admitted\n");
            failures++;
        }

        for (i = 0; i < sizeof(refused) / sizeof(refused[0]); ++i) {
            req.requester_ring = refused[i];
            if (atarix_capability_evaluate(&cap, 1, &req, &eval) != 0 || eval.decision == ATARIX_DECISION_ALLOW) {
                printf("ring %d admitted by a kernel..application capability\n", (int)refused[i]);
                failures++;
            }
        }
    }

    return failures ? 1 : 0;
}
