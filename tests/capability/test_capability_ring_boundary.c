#include <stdio.h>
#include <string.h>

#include "atarix/capability_policy.h"
#include "atarix/operations.h"

static void make_capability(atarix_capability_record_v1_t *capability,
                            uint16_t minimum_ring,
                            uint16_t maximum_ring) {
    memset(capability, 0, sizeof(*capability));
    capability->capability_id = 11u;
    capability->operation_id = ATARIX_OPERATION_READ;
    capability->target_handle = atarix_discovery_make_handle(3u, 1u, 1u);
    capability->minimum_ring = minimum_ring;
    capability->maximum_ring = maximum_ring;
    capability->flags = ATARIX_CAPABILITY_FLAG_READ_ALLOWED;
    capability->issuer_handle = atarix_discovery_make_handle(1u, 1u, 1u);
    capability->trust_requirement = ATARIX_TRUST_IDENTIFIED;
}

static void make_request(atarix_policy_request_t *request,
                         const atarix_capability_record_v1_t *capability,
                         atarix_ring_t caller_ring) {
    memset(request, 0, sizeof(*request));
    request->caller_principal_id = 42u;
    request->owner_principal_id = 42u;
    request->issuer_principal_id = 1u;
    request->caller_ring = caller_ring;
    request->issuer_trust = ATARIX_TRUST_TRUSTED;
    request->owner_trust = ATARIX_TRUST_IDENTIFIED;
    request->target_handle = capability->target_handle;
    request->operation_id = ATARIX_OPERATION_READ;
    request->capability = capability;
}

static int expect_policy(const char *name,
                         atarix_ring_t caller_ring,
                         uint16_t minimum_ring,
                         uint16_t maximum_ring,
                         atarix_policy_status_t expected_status,
                         atarix_policy_decision_t expected_decision) {
    atarix_capability_record_v1_t capability;
    atarix_policy_request_t request;
    atarix_policy_result_t result;
    atarix_policy_status_t status;

    make_capability(&capability, minimum_ring, maximum_ring);
    make_request(&request, &capability, caller_ring);

    status = atarix_policy_evaluate(&request, &result);

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
    int failures = 0;

    failures += expect_policy("application self access",
                              ATARIX_RING_APPLICATION,
                              ATARIX_RING_WIRE_APPLICATION,
                              ATARIX_RING_WIRE_APPLICATION,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    failures += expect_policy("service self access",
                              ATARIX_RING_SERVICE,
                              ATARIX_RING_WIRE_SERVICE,
                              ATARIX_RING_WIRE_SERVICE,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    failures += expect_policy("driver self access",
                              ATARIX_RING_DRIVER,
                              ATARIX_RING_WIRE_DRIVER,
                              ATARIX_RING_WIRE_DRIVER,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    failures += expect_policy("kernel self access",
                              ATARIX_RING_KERNEL,
                              ATARIX_RING_WIRE_KERNEL,
                              ATARIX_RING_WIRE_KERNEL,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    failures += expect_policy("application denied kernel authority",
                              ATARIX_RING_APPLICATION,
                              ATARIX_RING_WIRE_KERNEL,
                              ATARIX_RING_WIRE_KERNEL,
                              ATARIX_POLICY_STATUS_RING_DENIED,
                              ATARIX_POLICY_DECISION_DENY);

    failures += expect_policy("service denied driver authority",
                              ATARIX_RING_SERVICE,
                              ATARIX_RING_WIRE_DRIVER,
                              ATARIX_RING_WIRE_DRIVER,
                              ATARIX_POLICY_STATUS_RING_DENIED,
                              ATARIX_POLICY_DECISION_DENY);

    failures += expect_policy("driver denied fabric authority",
                              ATARIX_RING_DRIVER,
                              ATARIX_RING_WIRE_FABRIC,
                              ATARIX_RING_WIRE_FABRIC,
                              ATARIX_POLICY_STATUS_RING_DENIED,
                              ATARIX_POLICY_DECISION_DENY);

    failures += expect_policy("kernel denied supervisor authority",
                              ATARIX_RING_KERNEL,
                              ATARIX_RING_WIRE_SUPERVISOR,
                              ATARIX_RING_WIRE_SUPERVISOR,
                              ATARIX_POLICY_STATUS_RING_DENIED,
                              ATARIX_POLICY_DECISION_DENY);

    failures += expect_policy("fabric self access",
                              ATARIX_RING_FABRIC,
                              ATARIX_RING_WIRE_FABRIC,
                              ATARIX_RING_WIRE_FABRIC,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    failures += expect_policy("supervisor self access",
                              ATARIX_RING_SUPERVISOR,
                              ATARIX_RING_WIRE_SUPERVISOR,
                              ATARIX_RING_WIRE_SUPERVISOR,
                              ATARIX_POLICY_STATUS_OK,
                              ATARIX_POLICY_DECISION_ALLOW);

    return failures ? 1 : 0;
}
