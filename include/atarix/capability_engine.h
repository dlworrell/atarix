#ifndef ATARIX_CAPABILITY_ENGINE_H
#define ATARIX_CAPABILITY_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/capability.h"
#include "atarix/discovery.h"
#include "atarix/rings.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum atarix_decision {
    ATARIX_DECISION_ALLOW = 0,
    ATARIX_DECISION_DENY = 1,
    ATARIX_DECISION_RESTRICT = 2,
    ATARIX_DECISION_RECOVERY_ONLY = 3
} atarix_decision_t;

typedef struct atarix_capability_request {
    atarix_discovery_handle_t target_handle;
    uint32_t operation_id;
    atarix_ring_t requester_ring;
    atarix_trust_level_t trust_level;
    uint32_t request_flags;
} atarix_capability_request_t;

typedef struct atarix_capability_evaluation {
    atarix_decision_t decision;
    const atarix_capability_record_v1_t *matched_capability;
    uint32_t reason_code;
} atarix_capability_evaluation_t;

int atarix_capability_evaluate(
    const atarix_capability_record_v1_t *capabilities,
    size_t capability_count,
    const atarix_capability_request_t *request,
    atarix_capability_evaluation_t *evaluation);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_CAPABILITY_ENGINE_H */
