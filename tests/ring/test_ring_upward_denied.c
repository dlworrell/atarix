#include <stdio.h>

#include "atarix/ring.h"

int main(void) {
    int failures = 0;

    if (atarix_ring_can_access(ATARIX_RING_APPLICATION,
                               ATARIX_RING_SUPERVISOR,
                               ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_DENY) {
        printf("application to supervisor should be denied\n");
        failures++;
    }

    if (atarix_ring_can_access(ATARIX_RING_DRIVER,
                               ATARIX_RING_FABRIC,
                               ATARIX_RING_OPERATION_REQUEST) != ATARIX_RING_STATUS_DENY) {
        printf("driver to fabric should be denied\n");
        failures++;
    }

    return failures ? 1 : 0;
}
