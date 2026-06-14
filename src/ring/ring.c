#include "atarix/ring.h"

int atarix_ring_is_valid(
    atarix_ring_t ring) {
    return ring >= ATARIX_RING_SUPERVISOR && ring <= ATARIX_RING_QUARANTINE;
}

int atarix_ring_operation_is_valid(
    atarix_ring_operation_t operation) {
    return operation >= ATARIX_RING_OPERATION_OBSERVE &&
           operation <= ATARIX_RING_OPERATION_RECOVER;
}

atarix_ring_status_t atarix_ring_can_access(
    atarix_ring_t requester,
    atarix_ring_t target,
    atarix_ring_operation_t operation) {
    if (!atarix_ring_is_valid(requester) || !atarix_ring_is_valid(target)) {
        return ATARIX_RING_STATUS_INVALID_RING;
    }

    if (!atarix_ring_operation_is_valid(operation)) {
        return ATARIX_RING_STATUS_INVALID_OPERATION;
    }

    if (requester == ATARIX_RING_QUARANTINE) {
        return ATARIX_RING_STATUS_DENY;
    }

    if (target == ATARIX_RING_QUARANTINE) {
        return requester == ATARIX_RING_SUPERVISOR
            ? ATARIX_RING_STATUS_ALLOW
            : ATARIX_RING_STATUS_DENY;
    }

    if (requester == target) {
        return ATARIX_RING_STATUS_ALLOW;
    }

    if (requester > target) {
        return ATARIX_RING_STATUS_DENY;
    }

    if (operation == ATARIX_RING_OPERATION_MANAGE ||
        operation == ATARIX_RING_OPERATION_QUARANTINE ||
        operation == ATARIX_RING_OPERATION_RECOVER) {
        return requester == ATARIX_RING_SUPERVISOR
            ? ATARIX_RING_STATUS_ALLOW
            : ATARIX_RING_STATUS_DENY;
    }

    return ATARIX_RING_STATUS_ALLOW;
}
