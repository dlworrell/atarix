#ifndef ATARIX_CAPABILITY_POLICY_H
#define ATARIX_CAPABILITY_POLICY_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/capability.h"
#include "atarix/discovery.h"
#include "atarix/rings.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum atarix_policy_status {
    ATARIX_POLICY_STATUS_OK = 0,
    ATARIX_POLICY_STATUS_NULL = 1,
    ATARIX_POLICY_STATUS_INVALID_REQUEST = 2,
    ATARIX_POLICY_STATUS_INVALID_CAPABILITY = 3,
    ATARIX_POLICY_STATUS_UNTRUSTED_ISSUER = 4,
    ATARIX_POLICY_STATUS_UNTRUSTED_OWNER = 5,
    ATARIX_POLICY_STATUS_REVOKED = 6,
    ATARIX_POLICY_STATUS_EXPIRED = 7,
    ATARIX_POLICY_STATUS_RING_DENIED = 8,
    ATARIX_POLICY_STATUS_OWNER_MISMATCH = 9,
    ATARIX_POLICY_STATUS_DELEGATION_DENIED = 10,
    ATARIX_POLICY_STATUS_RESOURCE_DENIED = 11,
    ATARIX_POLICY_STATUS_OPERATION_DENIED = 12
} atarix_policy_status_t;

typedef enum atarix_policy_decision {
    ATARIX_POLICY_DECISION_DENY = 0,
    ATARIX_POLICY_DECISION_ALLOW = 1,
    ATARIX_POLICY_DECISION_RESTRICT = 2,
    ATARIX_POLICY_DECISION_RECOVERY_ONLY = 3
} atarix_policy_decision_t;

typedef struct atarix_policy_request {
    uint32_t caller_principal_id;
    uint32_t owner_principal_id;
    uint32_t issuer_principal_id;

    atarix_ring_t caller_ring;
    atarix_trust_level_t issuer_trust;
    atarix_trust_level_t owner_trust;

    atarix_discovery_handle_t target_handle;
    uint32_t operation_id;

    uint64_t current_time;
    uint32_t request_flags;

    const atarix_capability_record_v1_t *capability;
} atarix_policy_request_t;

typedef struct atarix_policy_result {
    atarix_policy_decision_t decision;
    atarix_policy_status_t status;
    const atarix_capability_record_v1_t *matched_capability;
} atarix_policy_result_t;

atarix_policy_status_t atarix_capability_record_validate(
    const atarix_capability_record_v1_t *capability);

int atarix_capability_record_allows_operation(
    const atarix_capability_record_v1_t *capability,
    uint32_t operation_id);

atarix_policy_status_t atarix_policy_evaluate(
    const atarix_policy_request_t *request,
    atarix_policy_result_t *result);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_CAPABILITY_POLICY_H */
