#include <stdio.h>
#include <string.h>

#include "atarix/capability_policy.h"
#include "atarix/operations.h"
#include "atarix/ring.h"

static void make_capability(atarix_capability_record_v1_t *capability) {
    memset(capability, 0, sizeof(*capability));
    capability->capability_id = 301u;
    capability->operation_id = ATARIX_OPERATION_CONTROL;
    capability->target_handle = atarix_discovery_make_handle(3u, 1u, 1u);
    capability->minimum_ring = ATARIX_RING_WIRE_DRIVER;
    capability->maximum_ring = ATARIX_RING_WIRE_DRIVER;
    capability->flags = ATARIX_CAPABILITY_FLAG_CONTROL_ALLOWED;
    capability->issuer_handle = atarix_discovery_make_handle(1u, 1u, 1u);
    capability->trust_requirement = ATARIX_TRUST_IDENTIFIED;
}

static void make_request(atarix_policy_request_t *request,
                         const atarix_capability_record_v1_t *capability) {
    memset(request, 0, sizeof(*request));
    request->caller_principal_id = 42u;
    request->owner_principal_id = 42u;
    request->issuer_principal_id = 1u;
    request->caller_ring = ATARIX_RING_DRIVER;
    request->issuer_trust = ATARIX_TRUST_TRUSTED;
    request->owner_trust = ATARIX_TRUST_IDENTIFIED;
    request->target_handle = capability->target_handle;
    request->operation_id = ATARIX_OPERATION_CONTROL;
    request->request_flags = ATARIX_POLICY_REQUEST_FLAG_CAPABILITY_REVOKED;
    request->capability = capability;
}

int main(void) {
    atarix_capability_record_v1_t capability;
    atarix_policy_request_t request;
    atarix_policy_result_t result;
    atarix_ring_status_t ring_status;
    atarix_policy_status_t policy_status;
    int failures = 0;

    make_capability(&capability);
    make_request(&request, &capability);

    ring_status = atarix_ring_can_access(ATARIX_RING_DRIVER,
                                         ATARIX_RING_APPLICATION,
                                         ATARIX_RING_OPERATION_REQUEST);
    if (ring_status != ATARIX_RING_STATUS_ALLOW) {
        printf("driver to application ring boundary got %d expected allow\n", (int)ring_status);
        failures++;
    }

    policy_status = atarix_policy_evaluate(&request, &result);
    if (policy_status != ATARIX_POLICY_STATUS_REVOKED || result.decision != ATARIX_POLICY_DECISION_DENY) {
        printf("inactive capability got status %d decision %d expected inactive/deny\n",
               (int)policy_status,
               (int)result.decision);
        failures++;
    }

    return failures ? 1 : 0;
}
