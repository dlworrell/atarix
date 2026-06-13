#include "atarix/capability_policy.h"

static int trust_is_usable(atarix_trust_level_t trust) {
    return trust != ATARIX_TRUST_UNTRUSTED && trust != ATARIX_TRUST_REVOKED;
}

static uint16_t ring_to_wire(atarix_ring_t ring) {
    return atarix_ring_to_wire(ring);
}

static int capability_ring_range_valid(const atarix_capability_record_v1_t *capability) {
    if (capability->minimum_ring == ATARIX_RING_WIRE_SUPERVISOR) {
        return capability->maximum_ring == ATARIX_RING_WIRE_SUPERVISOR;
    }

    if (capability->minimum_ring == ATARIX_RING_WIRE_FABRIC) {
        return capability->maximum_ring == ATARIX_RING_WIRE_FABRIC ||
               capability->maximum_ring <= ATARIX_RING_WIRE_APPLICATION;
    }

    return capability->minimum_ring <= capability->maximum_ring &&
           capability->maximum_ring <= ATARIX_RING_WIRE_APPLICATION;
}

static int ring_in_capability_range(atarix_ring_t caller_ring, const atarix_capability_record_v1_t *capability) {
    uint16_t caller = ring_to_wire(caller_ring);

    if (capability->minimum_ring == ATARIX_RING_WIRE_SUPERVISOR) {
        return caller == ATARIX_RING_WIRE_SUPERVISOR;
    }

    if (capability->minimum_ring == ATARIX_RING_WIRE_FABRIC) {
        return caller == ATARIX_RING_WIRE_FABRIC;
    }

    return caller >= capability->minimum_ring && caller <= capability->maximum_ring;
}

static int capability_uses_supervisor_authority(const atarix_capability_record_v1_t *capability) {
    return capability->minimum_ring == ATARIX_RING_WIRE_SUPERVISOR ||
           capability->maximum_ring == ATARIX_RING_WIRE_SUPERVISOR;
}

static int request_capability_revoked(const atarix_policy_request_t *request) {
    return (request->request_flags & ATARIX_POLICY_REQUEST_FLAG_CAPABILITY_REVOKED) != 0u;
}

static int request_capability_expired(const atarix_policy_request_t *request) {
    const atarix_capability_record_v1_t *capability = request->capability;

    if ((capability->flags & ATARIX_CAPABILITY_FLAG_TIME_LIMITED) == 0u) {
        return 0;
    }

    if (capability->expiration_or_lifetime == 0u) {
        return 1;
    }

    return request->current_time > capability->expiration_or_lifetime;
}

atarix_policy_status_t atarix_capability_record_validate(
    const atarix_capability_record_v1_t *capability) {
    if (!capability) {
        return ATARIX_POLICY_STATUS_NULL;
    }

    if (capability->capability_id == 0u) {
        return ATARIX_POLICY_STATUS_INVALID_CAPABILITY;
    }

    if (capability->operation_id == ATARIX_OPERATION_INVALID) {
        return ATARIX_POLICY_STATUS_INVALID_CAPABILITY;
    }

    if (!capability_ring_range_valid(capability)) {
        return ATARIX_POLICY_STATUS_INVALID_CAPABILITY;
    }

    if (capability->target_handle == 0u) {
        return ATARIX_POLICY_STATUS_INVALID_CAPABILITY;
    }

    if (capability->issuer_handle == 0u) {
        return ATARIX_POLICY_STATUS_INVALID_CAPABILITY;
    }

    return ATARIX_POLICY_STATUS_OK;
}

int atarix_capability_record_allows_operation(
    const atarix_capability_record_v1_t *capability,
    uint32_t operation_id) {
    if (!capability) {
        return 0;
    }

    if (capability->operation_id != operation_id) {
        return 0;
    }

    if (operation_id == ATARIX_OPERATION_READ) {
        return (capability->flags & ATARIX_CAPABILITY_FLAG_READ_ALLOWED) != 0u;
    }

    if (operation_id == ATARIX_OPERATION_WRITE) {
        return (capability->flags & ATARIX_CAPABILITY_FLAG_WRITE_ALLOWED) != 0u;
    }

    if (operation_id == ATARIX_OPERATION_CONTROL) {
        return (capability->flags & ATARIX_CAPABILITY_FLAG_CONTROL_ALLOWED) != 0u;
    }

    return 1;
}

atarix_policy_status_t atarix_capability_delegation_validate(
    const atarix_capability_record_v1_t *parent,
    const atarix_capability_record_v1_t *child) {
    atarix_policy_status_t status;

    if (!parent || !child) {
        return ATARIX_POLICY_STATUS_NULL;
    }

    status = atarix_capability_record_validate(parent);
    if (status != ATARIX_POLICY_STATUS_OK) {
        return status;
    }

    status = atarix_capability_record_validate(child);
    if (status != ATARIX_POLICY_STATUS_OK) {
        return status;
    }

    if ((parent->flags & ATARIX_CAPABILITY_FLAG_DELEGATION_ALLOWED) == 0u) {
        return ATARIX_POLICY_STATUS_DELEGATION_DENIED;
    }

    if (capability_uses_supervisor_authority(parent) || capability_uses_supervisor_authority(child)) {
        return ATARIX_POLICY_STATUS_DELEGATION_DENIED;
    }

    if (parent->target_handle != child->target_handle) {
        return ATARIX_POLICY_STATUS_RESOURCE_DENIED;
    }

    if (parent->operation_id != child->operation_id) {
        return ATARIX_POLICY_STATUS_OPERATION_DENIED;
    }

    if ((child->flags & ~parent->flags) != 0u) {
        return ATARIX_POLICY_STATUS_DELEGATION_DENIED;
    }

    if (child->minimum_ring < parent->minimum_ring || child->maximum_ring < parent->minimum_ring) {
        return ATARIX_POLICY_STATUS_DELEGATION_DENIED;
    }

    if (child->maximum_ring > parent->maximum_ring) {
        return ATARIX_POLICY_STATUS_DELEGATION_DENIED;
    }

    return ATARIX_POLICY_STATUS_OK;
}

atarix_policy_status_t atarix_policy_evaluate(
    const atarix_policy_request_t *request,
    atarix_policy_result_t *result) {
    atarix_policy_status_t status;

    if (!request || !result) {
        return ATARIX_POLICY_STATUS_NULL;
    }

    result->decision = ATARIX_POLICY_DECISION_DENY;
    result->status = ATARIX_POLICY_STATUS_INVALID_REQUEST;
    result->matched_capability = 0;

    if (!request->capability) {
        result->status = ATARIX_POLICY_STATUS_NULL;
        return result->status;
    }

    status = atarix_capability_record_validate(request->capability);
    if (status != ATARIX_POLICY_STATUS_OK) {
        result->status = status;
        return status;
    }

    result->matched_capability = request->capability;

    if (request_capability_revoked(request)) {
        result->status = ATARIX_POLICY_STATUS_REVOKED;
        return result->status;
    }

    if (request_capability_expired(request)) {
        result->status = ATARIX_POLICY_STATUS_EXPIRED;
        return result->status;
    }

    if (!trust_is_usable(request->issuer_trust)) {
        result->status = ATARIX_POLICY_STATUS_UNTRUSTED_ISSUER;
        return result->status;
    }

    if (!trust_is_usable(request->owner_trust)) {
        result->status = ATARIX_POLICY_STATUS_UNTRUSTED_OWNER;
        return result->status;
    }

    if (request->caller_principal_id != request->owner_principal_id) {
        result->status = ATARIX_POLICY_STATUS_OWNER_MISMATCH;
        return result->status;
    }

    if (!ring_in_capability_range(request->caller_ring, request->capability)) {
        result->status = ATARIX_POLICY_STATUS_RING_DENIED;
        return result->status;
    }

    if (request->target_handle != request->capability->target_handle) {
        result->status = ATARIX_POLICY_STATUS_RESOURCE_DENIED;
        return result->status;
    }

    if (!atarix_capability_record_allows_operation(request->capability, request->operation_id)) {
        result->status = ATARIX_POLICY_STATUS_OPERATION_DENIED;
        return result->status;
    }

    result->decision = ATARIX_POLICY_DECISION_ALLOW;
    result->status = ATARIX_POLICY_STATUS_OK;
    return result->status;
}
