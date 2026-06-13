#include <stdio.h>
#include <string.h>

#include "atarix/capability_policy.h"
#include "atarix/operations.h"

static void make_capability(atarix_capability_record_v1_t *capability) {
    memset(capability, 0, sizeof(*capability));
    capability->capability_id = 19u;
    capability->operation_id = ATARIX_OPERATION_READ;
    capability->target_handle = atarix_discovery_make_handle(3u, 1u, 1u);
    capability->minimum_ring = ATARIX_RING_WIRE_SERVICE;
    capability->maximum_ring = ATARIX_RING_WIRE_APPLICATION;
    capability->flags = ATARIX_CAPABILITY_FLAG_READ_ALLOWED;
    capability->issuer_handle = atarix_discovery_make_handle(1u, 1u, 1u);
    capability->trust_requirement = ATARIX_TRUST_IDENTIFIED;
}

static void make_request(atarix_policy_request_t *request,
                         const atarix_capability_record_v1_t *capability) {
    memset(request, 0, sizeof(*request));
    request->caller_principal_id = 42u;
    request->owner_principal_id = 42u;
    request->issuer_principal_id = 1u;
    request->caller_ring = ATARIX_RING_SERVICE;
    request->issuer_trust = ATARIX_TRUST_TRUSTED;
    request->owner_trust = ATARIX_TRUST_IDENTIFIED;
    request->target_handle = capability->target_handle;
    request->operation_id = ATARIX_OPERATION_READ;
    request->current_time = 100u;
    request->capability = capability;
}

static int expect_policy(const char *name,
                         atarix_policy_request_t *request,
                         atarix_policy_status_t expected_status,
                         atarix_policy_decision_t expected_decision) {
    atarix_policy_result_t result;
    atarix_policy_status_t status;

    status = atarix_policy_evaluate(request, &result);
    if (status != expected_status || result.decision != expected_decision) {
        printf("%s: got status %d decision %d expected status %d decision %d\n",
               name,
               (int)status,
               (int)result.decision,
               (int)expected_status,
               (int)expected_decision);
        return 1;
    }

    return 0;
}

int main(void) {
    atarix_capability_record_v1_t capability;
    atarix_policy_request_t request;
    int failures = 0;

    make_capability(&capability);
    make_request(&request, &capability);
    failures += expect_policy("valid capability allowed",
                              &request,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    make_capability(&capability);
    make_request(&request, &capability);
    request.request_flags = ATARIX_POLICY_REQUEST_FLAG_CAPABILITY_REVOKED;
    failures += expect_policy("revoked capability denied",
                              &request,
                              ATARIX_POLICY_STATUS_REVOKED,
                              ATARIX_POLICY_DECISION_DENY);

    make_capability(&capability);
    capability.flags |= ATARIX_CAPABILITY_FLAG_TIME_LIMITED;
    capability.expiration_or_lifetime = 99u;
    make_request(&request, &capability);
    request.current_time = 100u;
    failures += expect_policy("expired capability denied",
                              &request,
                              ATARIX_POLICY_STATUS_EXPIRED,
                              ATARIX_POLICY_DECISION_DENY);

    make_capability(&capability);
    capability.flags |= ATARIX_CAPABILITY_FLAG_TIME_LIMITED;
    capability.expiration_or_lifetime = 100u;
    make_request(&request, &capability);
    request.current_time = 100u;
    failures += expect_policy("capability valid at expiration boundary",
                              &request,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    make_capability(&capability);
    capability.flags |= ATARIX_CAPABILITY_FLAG_TIME_LIMITED;
    capability.expiration_or_lifetime = 0u;
    make_request(&request, &capability);
    request.current_time = 100u;
    failures += expect_policy("zero lifetime denied",
                              &request,
                              ATARIX_POLICY_STATUS_EXPIRED,
                              ATARIX_POLICY_DECISION_DENY);

    make_capability(&capability);
    make_request(&request, &capability);
    request.request_flags = ATARIX_POLICY_REQUEST_FLAG_CAPABILITY_REVOKED;
    request.caller_principal_id = request.owner_principal_id;
    failures += expect_policy("revocation overrides ownership",
                              &request,
                              ATARIX_POLICY_STATUS_REVOKED,
                              ATARIX_POLICY_DECISION_DENY);

    make_capability(&capability);
    make_request(&request, &capability);
    request.request_flags = ATARIX_POLICY_REQUEST_FLAG_CAPABILITY_REVOKED;
    request.issuer_trust = ATARIX_TRUST_TRUSTED;
    request.owner_trust = ATARIX_TRUST_TRUSTED;
    failures += expect_policy("revocation overrides trust",
                              &request,
                              ATARIX_POLICY_STATUS_REVOKED,
                              ATARIX_POLICY_DECISION_DENY);

    return failures ? 1 : 0;
}
