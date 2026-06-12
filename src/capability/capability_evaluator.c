#include "atarix/capability_engine.h"

static int ring_allows(uint16_t minimum_ring_wire, uint16_t maximum_ring_wire, atarix_ring_t requester_ring) {
    uint16_t requester = atarix_ring_to_wire(requester_ring);

    if (minimum_ring_wire == ATARIX_RING_WIRE_SUPERVISOR) {
        return requester == ATARIX_RING_WIRE_SUPERVISOR;
    }

    if (minimum_ring_wire == ATARIX_RING_WIRE_FABRIC) {
        return requester == ATARIX_RING_WIRE_SUPERVISOR || requester == ATARIX_RING_WIRE_FABRIC;
    }

    return requester >= minimum_ring_wire && requester <= maximum_ring_wire;
}

int atarix_capability_evaluate(
    const atarix_capability_record_v1_t *capabilities,
    size_t capability_count,
    const atarix_capability_request_t *request,
    atarix_capability_evaluation_t *evaluation) {
    size_t i;

    if (!capabilities || !request || !evaluation) {
        return -1;
    }

    evaluation->decision = ATARIX_DECISION_DENY;
    evaluation->matched_capability = 0;
    evaluation->reason_code = 0;

    for (i = 0; i < capability_count; ++i) {
        const atarix_capability_record_v1_t *cap = &capabilities[i];

        if (cap->target_handle != request->target_handle) {
            continue;
        }

        if (cap->operation_id != request->operation_id) {
            continue;
        }

        if (request->trust_level < (atarix_trust_level_t)cap->trust_requirement) {
            evaluation->decision = ATARIX_DECISION_RESTRICT;
            evaluation->matched_capability = cap;
            evaluation->reason_code = 1;
            return 0;
        }

        if (!ring_allows(cap->minimum_ring, cap->maximum_ring, request->requester_ring)) {
            evaluation->decision = ATARIX_DECISION_DENY;
            evaluation->matched_capability = cap;
            evaluation->reason_code = 2;
            return 0;
        }

        if ((cap->flags & ATARIX_CAPABILITY_FLAG_RECOVERY_ONLY) != 0u) {
            evaluation->decision = ATARIX_DECISION_RECOVERY_ONLY;
        } else {
            evaluation->decision = ATARIX_DECISION_ALLOW;
        }

        evaluation->matched_capability = cap;
        evaluation->reason_code = 0;
        return 0;
    }

    evaluation->decision = ATARIX_DECISION_DENY;
    evaluation->reason_code = 3;
    return 0;
}
