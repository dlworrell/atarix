# Trust Model v1

Status: Draft v1

## Purpose

This document defines the ATARIX trust model.

Trust answers:

```text
How much confidence should the platform place in this card, provider, service, or resource?
```

Trust does not identify hardware.

Trust does not describe resources.

Trust does not grant capabilities by itself.

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Trust States

Initial trust states:

```text
UNTRUSTED
IDENTIFIED
VALIDATED
TRUSTED
PRIVILEGED
REVOKED
```

## State Definitions

### UNTRUSTED

The platform has insufficient confidence in the entity.

This may occur when:

- identity is missing
- identity validation failed
- discovery validation failed
- policy rejects the entity
- the entity is explicitly blocked

### IDENTIFIED

The entity has a valid identity source.

Identity proves existence only.

IDENTIFIED does not imply operational authority.

### VALIDATED

The entity has passed required validation steps.

Examples:

- Identity EEPROM CRC valid
- Discovery ROM CRC valid
- health checks acceptable
- provisioning records acceptable

### TRUSTED

The entity is trusted by platform policy for normal participation.

TRUSTED does not imply privileged authority.

### PRIVILEGED

The entity is trusted for elevated platform functions.

Examples:

- supervisor-adjacent services
- recovery services
- boot authority
- fabric control
- privileged instrumentation

### REVOKED

The entity is explicitly denied by policy.

Revocation overrides identity, discovery, and prior trust.

## Homebrew Policy

Homebrew cards may be:

```text
Identified
Validated
Enumerated
```

without becoming TRUSTED or PRIVILEGED.

Default homebrew posture:

```text
Identity:      Allowed if valid
Discovery:     Allowed if valid
Validation:    Allowed if safe
Enumeration:   Allowed if policy permits
Capabilities:  Restricted
Supervisor:    Denied
DMA:           Denied unless explicitly authorized
Boot:          Denied unless explicitly authorized
```

## Trust Evaluation Inputs

Trust may consider:

- Identity EEPROM validation state
- Discovery ROM validation state
- board class
- trust class
- provider ID
- provisioning state
- factory test signature
- supervisor policy
- development-mode switch
- ring policy
- future cryptographic signatures
- revocation lists

## Trust and Rings

Trust and rings are separate.

Trust describes platform confidence.

Rings describe authority boundaries.

A trusted service may still be unavailable to Ring 3.

A privileged service may be visible to Ring 0 but controlled only by Ring -2.

## Trust and Capabilities

Capabilities authorize operations.

Trust evaluation determines whether capabilities may be accepted, reduced, or rejected.

A capability from an untrusted or revoked entity shall not be accepted for normal operation.

## Trust Transitions

Typical transition:

```text
UNTRUSTED
    ↓
IDENTIFIED
    ↓
VALIDATED
    ↓
TRUSTED
```

Privileged transition:

```text
TRUSTED
    ↓
PRIVILEGED
```

Revocation:

```text
ANY STATE
    ↓
REVOKED
```

## Recovery Rule

Recovery must not depend on trusted directory services.

Ring -2 supervisor authority must retain enough visibility to diagnose and recover from trust failures.

## Related Documents

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- ADR-RING-SECURITY-MODEL.md
- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- capability-record-format-v1.md
