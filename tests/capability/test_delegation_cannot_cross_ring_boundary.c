#include <stdio.h>
#include <string.h>

#include "atarix/capability_policy.h"
#include "atarix/operations.h"
#include "atarix/ring.h"

static void make_capability(atarix_capability_record_v1_t *capability,
                            uint32_t capability_id,
                            uint16_t minimum_ring,
                            uint16_t maximum_ring,
                            uint32_t flags) {
    memset(capability, 0, sizeof(*capability));
    capability->capability_id = capability_id;
    capability->operation_id = ATARIX_OPERATION_READ;
    capability->target_handle = atarix_discovery_make_handle(3u, 1u, 1u);
    capability->minimum_ring = minimum_ring;
    capability->maximum_ring = maximum_ring;
    capability->flags = flags;
    capability->issuer_handle = atarix_discovery_make_handle(1u, 1u, 1u);
    capability->trust_requirement = ATARIX_TRUST_IDENTIFIED;
}

int main(void) {
    atarix_capability_record_v1_t parent;
    atarix_capability_record_v1_t child;
    atarix_policy_status_t status;
    atarix_ring_status_t ring_status;
    int failures = 0;

    make_capability(&parent,
                    201u,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED |
                    ATARIX_CAPABILITY_FLAG_DELEGATION_ALLOWED);

    make_capability(&child,
                    202u,
                    ATARIX_RING_WIRE_KERNEL,
                    ATARIX_RING_WIRE_KERNEL,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED);

    status = atarix_capability_delegation_validate(&parent, &child);
    if (status != ATARIX_POLICY_STATUS_DELEGATION_DENIED) {
        printf("delegation got %d expected delegation denied\n", (int)status);
        failures++;
    }

    ring_status = atarix_ring_can_access(ATARIX_RING_APPLICATION,
                                         ATARIX_RING_KERNEL,
                                         ATARIX_RING_OPERATION_REQUEST);
    if (ring_status != ATARIX_RING_STATUS_DENY) {
        printf("application to kernel ring boundary got %d expected deny\n", (int)ring_status);
        failures++;
    }

    return failures ? 1 : 0;
}
