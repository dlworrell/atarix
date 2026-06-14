#include <stdio.h>

#include "atarix/ring.h"

int main(void) {
    int failures = 0;

    if (atarix_ring_can_access(ATARIX_RING_SUPERVISOR,
                               ATARIX_RING_QUARANTINE,
                               ATARIX_RING_OPERATION_RECOVER) != ATARIX_RING_STATUS_ALLOW) {
        printf("supervisor recover restricted ring should be allowed\n");
        failures++;
    }

    if (atarix_ring_can_access(ATARIX_RING_SUPERVISOR,
                               ATARIX_RING_APPLICATION,
                               ATARIX_RING_OPERATION_QUARANTINE) != ATARIX_RING_STATUS_ALLOW) {
        printf("supervisor restrict application should be allowed\n");
        failures++;
    }

    if (atarix_ring_can_access(ATARIX_RING_APPLICATION,
                               ATARIX_RING_DEVICE,
                               ATARIX_RING_OPERATION_QUARANTINE) != ATARIX_RING_STATUS_DENY) {
        printf("application restrict device should be denied\n");
        failures++;
    }

    return failures ? 1 : 0;
}
