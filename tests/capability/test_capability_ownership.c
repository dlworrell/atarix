#include <stdio.h>
#include <string.h>

#include "atarix/capability_policy.h"
#include "atarix/operations.h"

static int expect_decision(const char *name,
                           atarix_policy_status_t got_status,
                           atarix_policy_decision_t got_decision,
                           atarix_policy_status_t expected_status,
                           atarix_policy_decision_t expected_decision) {
    if (got_status != expected_status || got_decision != expected_decision) {
        printf("%s: got status %d decision %d expected status %d decision %d\n",
               name,
               (int)got_status,
               (int)got_decision,
               (int)expected_status,
               (int)expected_decision);
        return 1;
    }
    return 0;
}

static void make_capability(atarix_capability_record_v1_t *capability) {
    memset(capability, 0, sizeof(*capability));
    capability->capability_id = 7u;
    capability->operation_id = ATARIX_OPERATION_READ;
    capability->target_handle = atarix_discovery_make_handle(3u, 1u, 1u);
    capability->minimum_ring = ATARIX_RING_WIRE_KERNEL;
    capability->maximum_ring = ATARIX_RING_WIRE_APPLICATION;
    capability->flags = ATARIX_CAPABILITY_FLAG_READ_ALLOWED;
    capability->issuer_handle = atarix_discovery_make_handle(1u, 1u, 1u);
    capability->trust_requirement = ATARIX_TRUST_IDENTIFIED;
}

static void make_request(atarix_policy_request_t *request, const atarix_capability_record_v1_t *capability) {
    memset(request, 0, sizeof(*request));
    request->caller_principal_id = 42u;
    request->owner_principal_id = 42u;
    request->issuer_principal_id = 1u;
    request->caller_ring = ATARIX_RING_SERVICE;
    request->issuer_trust = ATARIX_TRUST_TRUSTED;
    request->owner_trust = ATARIX_TRUST_IDENTIFIED;
    request->target_handle = capability->target_handle;
    request->operation_id = ATARIX_OPERATION_READ;
    request->capability = capability;
}

int main(void) {
    atarix_capability_record_v1_t capability;
    atarix_policy_request_t request;
    atarix_policy_result_t result;
    atarix_policy_status_t status;
    int failures = 0;

    make_capability(&capability);
    make_request(&request, &capability);
    status = atarix_policy_evaluate(&request, &result);
    failures += expect_decision("owner accepted", status, result.decision, ATARIX_POLICY_STATUS_OK, ATARIX_POLICY_DECISION_ALLOW);

    make_capability(&capability);
    make_request(&request, &capability);
    request.caller_principal_id = 43u;
    status = atarix_policy_evaluate(&request, &result);
    failures += expect_decision("owner mismatch", status, result.decision, ATARIX_POLICY_STATUS_OWNER_MISMATCH, ATARIX_POLICY_DECISION_DENY);

    make_capability(&capability);
    make_request(&request, &capability);
    request.owner_trust = ATARIX_TRUST_REVOKED;
    status = atarix_policy_evaluate(&request, &result);
    failures += expect_decision("revoked owner", status, result.decision, ATARIX_POLICY_STATUS_UNTRUSTED_OWNER, ATARIX_POLICY_DECISION_DENY);

    make_capability(&capability);
    make_request(&request, &capability);
    request.issuer_trust = ATARIX_TRUST_UNTRUSTED;
    status = atarix_policy_evaluate(&request, &result);
    failures += expect_decision("untrusted issuer", status, result.decision, ATARIX_POLICY_STATUS_UNTRUSTED_ISSUER, ATARIX_POLICY_DECISION_DENY);

    return failures ? 1 : 0;
}
