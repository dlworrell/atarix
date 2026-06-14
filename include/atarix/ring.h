#ifndef ATARIX_RING_H
#define ATARIX_RING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum atarix_ring {
    ATARIX_RING_SUPERVISOR = 0,
    ATARIX_RING_FABRIC = 1,
    ATARIX_RING_DEVICE = 2,
    ATARIX_RING_APPLICATION = 3,
    ATARIX_RING_QUARANTINE = 4
} atarix_ring_t;

typedef enum atarix_ring_operation {
    ATARIX_RING_OPERATION_OBSERVE = 0,
    ATARIX_RING_OPERATION_REQUEST = 1,
    ATARIX_RING_OPERATION_MANAGE = 2,
    ATARIX_RING_OPERATION_QUARANTINE = 3,
    ATARIX_RING_OPERATION_RECOVER = 4
} atarix_ring_operation_t;

typedef enum atarix_ring_status {
    ATARIX_RING_STATUS_ALLOW = 0,
    ATARIX_RING_STATUS_DENY,
    ATARIX_RING_STATUS_INVALID_RING,
    ATARIX_RING_STATUS_INVALID_OPERATION
} atarix_ring_status_t;

atarix_ring_status_t atarix_ring_can_access(
    atarix_ring_t requester,
    atarix_ring_t target,
    atarix_ring_operation_t operation);

int atarix_ring_is_valid(
    atarix_ring_t ring);

int atarix_ring_operation_is_valid(
    atarix_ring_operation_t operation);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_RING_H */
