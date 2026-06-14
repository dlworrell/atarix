#include <stdio.h>

#include "atarix/ring.h"

int main(void) {
    int failures = 0;

    if (atarix_ring_can_access(ATARIX_RING_QUARANTINE,
                               ATARIX_RING_SUPERVISOR,
                               ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_DENY) {
        printf("restricted ring to supervisor should be denied\n");
        failures++;
    }

    if (atarix_ring_can_access(ATARIX_RING_QUARANTINE,
                               ATARIX_RING_APPLICATION,
                               ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_DENY) {
        printf("restricted ring to application should be denied\n");
        failures++;
    }

    if (atarix_ring_can_access(ATARIX_RING_APPLICATION,
                               ATARIX_RING_QUARANTINE,
                               ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_DENY) {
        printf("application to restricted ring should be denied\n");
        failures++;
    }

    return failures ? 1 : 0;
}
