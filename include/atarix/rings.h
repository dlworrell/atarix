#ifndef ATARIX_RINGS_H
#define ATARIX_RINGS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum atarix_ring {
    ATARIX_RING_SUPERVISOR = -2,
    ATARIX_RING_FABRIC = -1,
    ATARIX_RING_KERNEL = 0,
    ATARIX_RING_DRIVER = 1,
    ATARIX_RING_SERVICE = 2,
    ATARIX_RING_APPLICATION = 3
} atarix_ring_t;

typedef enum atarix_ring_wire {
    ATARIX_RING_WIRE_SUPERVISOR = 0xFFFEu,
    ATARIX_RING_WIRE_FABRIC = 0xFFFFu,
    ATARIX_RING_WIRE_KERNEL = 0x0000u,
    ATARIX_RING_WIRE_DRIVER = 0x0001u,
    ATARIX_RING_WIRE_SERVICE = 0x0002u,
    ATARIX_RING_WIRE_APPLICATION = 0x0003u
} atarix_ring_wire_t;

static inline uint16_t atarix_ring_to_wire(atarix_ring_t ring) {
    switch (ring) {
    case ATARIX_RING_SUPERVISOR:
        return ATARIX_RING_WIRE_SUPERVISOR;
    case ATARIX_RING_FABRIC:
        return ATARIX_RING_WIRE_FABRIC;
    case ATARIX_RING_KERNEL:
        return ATARIX_RING_WIRE_KERNEL;
    case ATARIX_RING_DRIVER:
        return ATARIX_RING_WIRE_DRIVER;
    case ATARIX_RING_SERVICE:
        return ATARIX_RING_WIRE_SERVICE;
    case ATARIX_RING_APPLICATION:
        return ATARIX_RING_WIRE_APPLICATION;
    default:
        return ATARIX_RING_WIRE_APPLICATION;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_RINGS_H */
