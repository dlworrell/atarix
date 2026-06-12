#ifndef ATARIX_CAPABILITY_H
#define ATARIX_CAPABILITY_H

#include <stdint.h>
#include "atarix/discovery.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_CAPABILITY_RECORD_SIZE_V1 56u

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

typedef enum atarix_trust_level {
    ATARIX_TRUST_UNTRUSTED = 0,
    ATARIX_TRUST_IDENTIFIED = 1,
    ATARIX_TRUST_VALIDATED = 2,
    ATARIX_TRUST_TRUSTED = 3,
    ATARIX_TRUST_PRIVILEGED = 4,
    ATARIX_TRUST_RESERVED5 = 5,
    ATARIX_TRUST_RESERVED6 = 6,
    ATARIX_TRUST_REVOKED = 7
} atarix_trust_level_t;

typedef enum atarix_operation_id {
    ATARIX_OPERATION_INVALID = 0x00000000u,
    ATARIX_OPERATION_READ = 0x00000001u,
    ATARIX_OPERATION_WRITE = 0x00000002u,
    ATARIX_OPERATION_CONTROL = 0x00000003u,
    ATARIX_OPERATION_RESET = 0x00000004u,
    ATARIX_OPERATION_CAPTURE = 0x00000005u,
    ATARIX_OPERATION_STIMULATE = 0x00000006u,
    ATARIX_OPERATION_BOOT = 0x00000007u,
    ATARIX_OPERATION_RECOVER = 0x00000008u,
    ATARIX_OPERATION_PROVISION = 0x00000009u,
    ATARIX_OPERATION_ENUMERATE = 0x0000000Au,
    ATARIX_OPERATION_VALIDATE = 0x0000000Bu,
    ATARIX_OPERATION_MONITOR = 0x0000000Cu
} atarix_operation_id_t;

typedef enum atarix_capability_flags {
    ATARIX_CAPABILITY_FLAG_READ_ALLOWED = 1u << 0,
    ATARIX_CAPABILITY_FLAG_WRITE_ALLOWED = 1u << 1,
    ATARIX_CAPABILITY_FLAG_CONTROL_ALLOWED = 1u << 2,
    ATARIX_CAPABILITY_FLAG_DELEGATION_ALLOWED = 1u << 3,
    ATARIX_CAPABILITY_FLAG_REVOCATION_SUPPORTED = 1u << 4,
    ATARIX_CAPABILITY_FLAG_TIME_LIMITED = 1u << 5,
    ATARIX_CAPABILITY_FLAG_DEVELOPMENT_ONLY = 1u << 6,
    ATARIX_CAPABILITY_FLAG_RECOVERY_ONLY = 1u << 7,
    ATARIX_CAPABILITY_FLAG_DESTRUCTIVE_OPERATION = 1u << 8,
    ATARIX_CAPABILITY_FLAG_REQUIRES_SUPERVISOR_APPROVAL = 1u << 9
} atarix_capability_flags_t;

#pragma pack(push, 1)
typedef struct atarix_capability_record_v1 {
    uint32_t capability_id;
    uint32_t operation_id;
    atarix_discovery_handle_t target_handle;
    uint16_t minimum_ring;
    uint16_t maximum_ring;
    uint32_t flags;
    uint64_t delegation_mask;
    uint64_t expiration_or_lifetime;
    atarix_discovery_handle_t issuer_handle;
    uint32_t trust_requirement;
    uint32_t reserved;
} atarix_capability_record_v1_t;
#pragma pack(pop)

_Static_assert(sizeof(atarix_capability_record_v1_t) == ATARIX_CAPABILITY_RECORD_SIZE_V1,
               "ATARIX Capability Record v1 must be 56 bytes");

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

static inline int atarix_capability_is_destructive(uint32_t flags) {
    return (flags & ATARIX_CAPABILITY_FLAG_DESTRUCTIVE_OPERATION) != 0;
}

static inline int atarix_capability_requires_supervisor(uint32_t flags) {
    return (flags & ATARIX_CAPABILITY_FLAG_REQUIRES_SUPERVISOR_APPROVAL) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_CAPABILITY_H */
