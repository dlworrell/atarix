#include <stdio.h>

#include "atarix/ring.h"

int main(void) {
    int failures = 0;

    if (atarix_ring_can_access(ATARIX_RING_APPLICATION,
                               ATARIX_RING_APPLICATION,
                               ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_ALLOW) {
        printf("application self access denied\n");
        failures++;
    }

    if (atarix_ring_can_access(ATARIX_RING_DEVICE,
                               ATARIX_RING_DEVICE,
                               ATARIX_RING_OPERATION_OBSERVE) != ATARIX_RING_STATUS_ALLOW) {
        printf("device self observe denied\n");
        failures++;
    }

    return failures ? 1 : 0;
}
