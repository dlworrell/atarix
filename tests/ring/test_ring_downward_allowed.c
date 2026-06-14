#include <stdio.h>

#include "atarix/ring.h"

int main(void) {
    int failures = 0;

    if (atarix_ring_can_access(ATARIX_RING_SUPERVISOR,
                               ATARIX_RING_DEVICE,
                               ATARIX_RING_OPERATION_MANAGE) != ATARIX_RING_STATUS_ALLOW) {
        printf("supervisor manage device should be allowed\n");
        failures++;
    }

    if (atarix_ring_can_access(ATARIX_RING_FABRIC,
                               ATARIX_RING_DEVICE,
                               ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_ALLOW) {
        printf("fabric request to device should be allowed\n");
        failures++;
    }

    return failures ? 1 : 0;
}
