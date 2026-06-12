#ifndef ATARIX_CAPABILITY_H
#define ATARIX_CAPABILITY_H

#include <stdint.h>
#include "atarix/discovery.h"
#include "atarix/operations.h"
#include "atarix/rings.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_CAPABILITY_RECORD_SIZE_V1 56u

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
