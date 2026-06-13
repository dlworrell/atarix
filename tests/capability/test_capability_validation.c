#include <stdio.h>
#include <string.h>

#include "atarix/capability_policy.h"
#include "atarix/operations.h"

static int expect_status(const char *name, atarix_policy_status_t got, atarix_policy_status_t expected) {
    if (got != expected) {
        printf("%s: got %d expected %d\n", name, (int)got, (int)expected);
        return 1;
    }
    return 0;
}

static void make_valid_capability(atarix_capability_record_v1_t *capability) {
    memset(capability, 0, sizeof(*capability));
    capability->capability_id = 1u;
    capability->operation_id = ATARIX_OPERATION_READ;
    capability->target_handle = atarix_discovery_make_handle(3u, 1u, 1u);
    capability->minimum_ring = ATARIX_RING_WIRE_KERNEL;
    capability->maximum_ring = ATARIX_RING_WIRE_APPLICATION;
    capability->flags = ATARIX_CAPABILITY_FLAG_READ_ALLOWED;
    capability->issuer_handle = atarix_discovery_make_handle(1u, 1u, 1u);
    capability->trust_requirement = ATARIX_TRUST_IDENTIFIED;
}

int main(void) {
    atarix_capability_record_v1_t capability;
    int failures = 0;

    failures += expect_status("null capability", atarix_capability_record_validate(0), ATARIX_POLICY_STATUS_NULL);

    make_valid_capability(&capability);
    failures += expect_status("valid capability", atarix_capability_record_validate(&capability), ATARIX_POLICY_STATUS_OK);

    make_valid_capability(&capability);
    capability.operation_id = ATARIX_OPERATION_INVALID;
    failures += expect_status("invalid operation", atarix_capability_record_validate(&capability), ATARIX_POLICY_STATUS_INVALID_CAPABILITY);

    make_valid_capability(&capability);
    capability.minimum_ring = ATARIX_RING_WIRE_APPLICATION;
    capability.maximum_ring = ATARIX_RING_WIRE_KERNEL;
    failures += expect_status("invalid ring range", atarix_capability_record_validate(&capability), ATARIX_POLICY_STATUS_INVALID_CAPABILITY);

    make_valid_capability(&capability);
    if (!atarix_capability_record_allows_operation(&capability, ATARIX_OPERATION_READ)) {
        printf("read operation should be allowed\n");
        failures++;
    }

    if (atarix_capability_record_allows_operation(&capability, ATARIX_OPERATION_WRITE)) {
        printf("write operation should be denied\n");
        failures++;
    }

    make_valid_capability(&capability);
    capability.flags = ATARIX_CAPABILITY_FLAG_DESTRUCTIVE_OPERATION;
    if (!atarix_capability_is_destructive(capability.flags)) {
        printf("destructive capability flag not recognized\n");
        failures++;
    }

    make_valid_capability(&capability);
    capability.flags = ATARIX_CAPABILITY_FLAG_REQUIRES_SUPERVISOR_APPROVAL;
    if (!atarix_capability_requires_supervisor(capability.flags)) {
        printf("supervisor approval flag not recognized\n");
        failures++;
    }

    return failures ? 1 : 0;
}
