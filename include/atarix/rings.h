#ifndef ATARIX_RINGS_H
#define ATARIX_RINGS_H

/*
 * ATARIX privilege rings.
 *
 * The canonical definition of every ring, its domain and its authority is
 * docs/ADR-RING-SECURITY-MODEL.md. This header must match it.
 *
 *   Platform rings (outside the CPU operating system; not reachable by
 *   any CPU-card software):
 *     -2  SUPERVISOR   hardware safety and supervisor authority (RP2350
 *                      supervisor): reset, power, watchdog, identity
 *                      validation, quarantine, recovery.
 *     -1  FABRIC       fabric and platform firmware authority (ECP5 fabric
 *                      controller, backplane BIOS): enumeration, transport
 *                      setup, service binding support. Describes; grants
 *                      nothing on its own.
 *
 *   Operating-system rings (the four CPU execution rings):
 *      0  KERNEL       kernel / executive: memory, scheduling, capability
 *                      enforcement, driver loading, transport use.
 *      1  DRIVER       driver / service: device drivers and hardware-facing
 *                      privileged services (for example the display card's
 *                      GPU compute service). May receive delegated, narrow
 *                      device capabilities.
 *      2  USER_SERVICE user services: filesystem, network, desktop and
 *                      session services with explicit service capabilities
 *                      and no direct hardware authority.
 *      3  APPLICATION  applications, scripts and user tools. No hardware
 *                      authority unless mediated by a lower ring.
 *
 *   Containment state (not an execution ring):
 *      4  QUARANTINE   a contained subject. It may not request anything;
 *                      only the supervisor may act on it.
 *
 * Lower numbers are more privileged. Ring checks constrain authority; they
 * never grant it: a capability is still required (identity is not
 * authority).
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum atarix_ring {
    ATARIX_RING_SUPERVISOR = -2,
    ATARIX_RING_FABRIC = -1,
    ATARIX_RING_KERNEL = 0,
    ATARIX_RING_DRIVER = 1,
    ATARIX_RING_USER_SERVICE = 2,
    ATARIX_RING_APPLICATION = 3,
    ATARIX_RING_QUARANTINE = 4,

    /* Deprecated alias: ring 2 is the user-service ring, not "the" service
     * ring. Hardware-facing services belong in ATARIX_RING_DRIVER. */
    ATARIX_RING_SERVICE = ATARIX_RING_USER_SERVICE
} atarix_ring_t;

/* Wire encoding (capability-record-format-v1.md). */
typedef enum atarix_ring_wire {
    ATARIX_RING_WIRE_SUPERVISOR = 0xFFFEu,
    ATARIX_RING_WIRE_FABRIC = 0xFFFFu,
    ATARIX_RING_WIRE_KERNEL = 0x0000u,
    ATARIX_RING_WIRE_DRIVER = 0x0001u,
    ATARIX_RING_WIRE_USER_SERVICE = 0x0002u,
    ATARIX_RING_WIRE_APPLICATION = 0x0003u,
    ATARIX_RING_WIRE_QUARANTINE = 0x0004u,

    /* Not a ring. Returned for an out-of-range ring so that no comparison
     * against a capability's ring range can succeed (fail closed). */
    ATARIX_RING_WIRE_INVALID = 0x8000u,

    /* Deprecated alias, see ATARIX_RING_SERVICE. */
    ATARIX_RING_WIRE_SERVICE = ATARIX_RING_WIRE_USER_SERVICE
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
    case ATARIX_RING_USER_SERVICE:
        return ATARIX_RING_WIRE_USER_SERVICE;
    case ATARIX_RING_APPLICATION:
        return ATARIX_RING_WIRE_APPLICATION;
    case ATARIX_RING_QUARANTINE:
        return ATARIX_RING_WIRE_QUARANTINE;
    default:
        /* Fail closed: an unknown ring must never be treated as a real one. */
        return ATARIX_RING_WIRE_INVALID;
    }
}

/* Short stable name for logs and audit records ("ring-2" style is avoided
 * so that names cannot be confused with numbers). */
static inline const char *atarix_ring_name(atarix_ring_t ring) {
    switch (ring) {
    case ATARIX_RING_SUPERVISOR:
        return "supervisor";
    case ATARIX_RING_FABRIC:
        return "fabric";
    case ATARIX_RING_KERNEL:
        return "kernel";
    case ATARIX_RING_DRIVER:
        return "driver";
    case ATARIX_RING_USER_SERVICE:
        return "user-service";
    case ATARIX_RING_APPLICATION:
        return "application";
    case ATARIX_RING_QUARANTINE:
        return "quarantine";
    default:
        return "invalid";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_RINGS_H */
