#include <stdio.h>
#include <string.h>

#include "atarix/capability_policy.h"
#include "atarix/operations.h"

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

static int expect_delegation(const char *name,
                             uint16_t parent_minimum_ring,
                             uint16_t parent_maximum_ring,
                             uint16_t child_minimum_ring,
                             uint16_t child_maximum_ring,
                             atarix_policy_status_t expected_status) {
    atarix_capability_record_v1_t parent;
    atarix_capability_record_v1_t child;
    atarix_policy_status_t status;

    make_capability(&parent,
                    101u,
                    parent_minimum_ring,
                    parent_maximum_ring,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED |
                    ATARIX_CAPABILITY_FLAG_DELEGATION_ALLOWED);

    make_capability(&child,
                    102u,
                    child_minimum_ring,
                    child_maximum_ring,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED);

    status = atarix_capability_delegation_validate(&parent, &child);
    if (status != expected_status) {
        printf("%s: got status %d expected %d\n", name, (int)status, (int)expected_status);
        return 1;
    }

    return 0;
}

int main(void) {
    atarix_capability_record_v1_t parent;
    atarix_capability_record_v1_t child;
    int failures = 0;

    failures += expect_delegation("kernel delegates to driver",
                                  ATARIX_RING_WIRE_KERNEL,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_DRIVER,
                                  ATARIX_RING_WIRE_DRIVER,
                                  ATARIX_POLICY_STATUS_OK);

    failures += expect_delegation("driver delegates to service",
                                  ATARIX_RING_WIRE_DRIVER,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_SERVICE,
                                  ATARIX_RING_WIRE_SERVICE,
                                  ATARIX_POLICY_STATUS_OK);

    failures += expect_delegation("service delegates to application",
                                  ATARIX_RING_WIRE_SERVICE,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_POLICY_STATUS_OK);

    failures += expect_delegation("application cannot delegate to service",
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_SERVICE,
                                  ATARIX_RING_WIRE_SERVICE,
                                  ATARIX_POLICY_STATUS_DELEGATION_DENIED);

    failures += expect_delegation("service cannot delegate to driver",
                                  ATARIX_RING_WIRE_SERVICE,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_DRIVER,
                                  ATARIX_RING_WIRE_DRIVER,
                                  ATARIX_POLICY_STATUS_DELEGATION_DENIED);

    failures += expect_delegation("driver cannot delegate to kernel",
                                  ATARIX_RING_WIRE_DRIVER,
                                  ATARIX_RING_WIRE_APPLICATION,
                                  ATARIX_RING_WIRE_KERNEL,
                                  ATARIX_RING_WIRE_KERNEL,
                                  ATARIX_POLICY_STATUS_DELEGATION_DENIED);

    make_capability(&parent,
                    201u,
                    ATARIX_RING_WIRE_KERNEL,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED);
    make_capability(&child,
                    202u,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED);
    if (atarix_capability_delegation_validate(&parent, &child) != ATARIX_POLICY_STATUS_DELEGATION_DENIED) {
        printf("delegation without delegation flag should be denied\n");
        failures++;
    }

    make_capability(&parent,
                    301u,
                    ATARIX_RING_WIRE_SUPERVISOR,
                    ATARIX_RING_WIRE_SUPERVISOR,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED |
                    ATARIX_CAPABILITY_FLAG_DELEGATION_ALLOWED);
    make_capability(&child,
                    302u,
                    ATARIX_RING_WIRE_KERNEL,
                    ATARIX_RING_WIRE_KERNEL,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED);
    if (atarix_capability_delegation_validate(&parent, &child) != ATARIX_POLICY_STATUS_DELEGATION_DENIED) {
        printf("supervisor authority delegation should be denied\n");
        failures++;
    }

    make_capability(&parent,
                    401u,
                    ATARIX_RING_WIRE_KERNEL,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED |
                    ATARIX_CAPABILITY_FLAG_DELEGATION_ALLOWED);
    make_capability(&child,
                    402u,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_RING_WIRE_APPLICATION,
                    ATARIX_CAPABILITY_FLAG_READ_ALLOWED |
                    ATARIX_CAPABILITY_FLAG_WRITE_ALLOWED);
    if (atarix_capability_delegation_validate(&parent, &child) != ATARIX_POLICY_STATUS_DELEGATION_DENIED) {
        printf("delegation that adds flags should be denied\n");
        failures++;
    }

    return failures ? 1 : 0;
}
