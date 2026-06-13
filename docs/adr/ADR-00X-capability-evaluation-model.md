# ADR-00X: Capability Evaluation Model

Status: Accepted

## Context

ATARIX uses a layered security model built around identity, trust, rings, capabilities, resources, and operations.

The Capability Engine must not be an independent permission system. It must implement the ATARIX trust and ring model.

The security chain is:

```text
Identity
    -> Trust
    -> Revocation
    -> Ring
    -> Ownership
    -> Capability
    -> Resource
    -> Operation
    -> ALLOW / DENY
```

## Decision

ATARIX capability evaluation shall be based on the following authority model:

```text
Supervisor (-2)
    Root of Trust

Fabric (-1)
    Enforcement Authority

Kernel (0)
    System Authority

Drivers (1)
    Device Authority

Services (2)
    Service Authority

Applications (3)
    User Authority
```

The Supervisor is the root of trust.

The Fabric is the enforcement authority.

Capabilities are bound to principals.

Capabilities are not transferable.

Delegation is permitted only when authority is reduced.

Revocation is hybrid: capabilities may expire naturally, and the Supervisor may revoke them immediately.

The default decision is DENY.

## Root of Trust

The Supervisor operates at Ring -2 and is the root of trust.

Responsibilities include:

```text
Identity management
Trust establishment
Trust revocation
Health monitoring
Recovery coordination
```

No entity below Ring -2 may establish system trust.

## Enforcement Authority

The Fabric operates at Ring -1 and enforces access decisions.

Responsibilities include:

```text
Capability enforcement
Resource mediation
Transport authorization
Ring boundary enforcement
```

A CPU, service, driver, or application may request access, but the Fabric is authoritative for fabric-mediated access.

## Principal-Bound Capabilities

Capabilities are bound to a principal.

Possession does not imply ownership.

A capability issued to one principal may not be used by another principal.

A capability request must therefore identify:

```text
Caller principal
Capability owner principal
Issuer principal
Target resource
Requested operation
```

The evaluator must verify that the caller is allowed to use the capability before evaluating the operation.

## Non-Transferability

Capabilities are non-transferable by default and by design.

A principal may not hand an existing capability to another principal as a way to grant authority.

If authority is to be delegated, the system must create a separate delegated capability with reduced authority and its own identity.

## Delegation

Delegation may reduce authority only.

Delegation may never:

```text
Increase ring privilege
Add operations
Expand resource scope
Bypass trust requirements
Bypass revocation
Create Supervisor authority
```

A delegated capability must remain traceable to its parent capability.

## Revocation

ATARIX uses a hybrid revocation model.

Capabilities may have expiration times.

Capabilities may also be revoked immediately by the Supervisor.

A capability is usable only if:

```text
Capability is structurally valid
Capability is not expired
Capability is not revoked
Issuer remains trusted
Owner remains trusted
```

Revocation is authoritative and immediate for future access attempts once the Fabric has received the revocation state.

## Evaluation Order

Capability evaluation shall proceed in the following order:

```text
1. Validate request structure
2. Validate caller identity
3. Validate issuer trust
4. Validate owner trust
5. Check revocation state
6. Check expiration
7. Check caller ring
8. Check capability ownership
9. Check delegation chain
10. Check resource scope
11. Check requested operation
12. Return ALLOW or DENY
```

The evaluator shall stop at the first failing condition and return DENY or a more specific restricted decision where applicable.

## Security Invariants

The following invariants define the model:

```text
I1  Trust precedes capability evaluation.
I2  Capabilities never increase authority.
I3  Delegation may only reduce authority.
I4  Ring boundaries are absolute.
I5  Supervisor authority is non-delegable.
I6  Fabric enforcement is authoritative.
I7  Deny-by-default.
I8  Capabilities are principal-bound.
I9  Revocation is authoritative and immediate.
```

## Ring Enforcement

A capability may not allow a caller to act above its authorized ring.

Ring boundaries are absolute.

A lower-authority principal cannot use a capability to become a higher-authority principal.

Examples:

```text
Application -> Kernel authority: DENY
Service -> Supervisor authority: DENY
Driver -> Fabric authority: DENY
```

## Supervisor Authority

Supervisor authority is non-delegable.

Operations such as trust establishment, trust revocation, power-cycle authority, and recovery override are Supervisor functions.

A capability may describe access to Supervisor-mediated services, but it may not transfer Supervisor authority to another ring.

## Fabric Enforcement

The Fabric is responsible for enforcing policy for fabric-mediated resources and transports.

The Fabric evaluates:

```text
Identity
Trust
Revocation
Ring
Ownership
Capability
Resource
Operation
```

before allowing access.

## Deny-by-Default

If any required field, trust state, revocation state, ownership relation, ring relation, delegation relation, resource scope, or operation permission is missing or invalid, the result is DENY.

ALLOW requires explicit success through the complete evaluation path.

## Consequences

Benefits:

```text
Clear trust root
Fabric-mediated enforcement
Principal-bound authority
Reduced confused-deputy risk
Immediate revocation support
Deterministic evaluation order
Testable security invariants
```

Costs:

```text
More metadata required per request
Delegation requires explicit derived capabilities
Fabric must receive revocation state
Policy evaluation is stricter than simple ACL checks
```

## Implementation Direction

Capability Sprint 1 shall implement this model through:

```text
include/atarix/capability_policy.h
src/capability/capability_policy.c
tests/capability/
```

Initial tests should cover:

```text
Capability validation
Ownership enforcement
Ring boundary enforcement
Delegation reduction
Revocation denial
Expiration denial
Operation allow/deny
Deny-by-default behavior
```

## Related Documents

```text
docs/trust-model-v1.md
docs/capability-record-format-v1.md
docs/operation-id-registry-v1.md
docs/service-directory-format-v1.md
include/atarix/rings.h
include/atarix/capability.h
```

## Revision History

```text
v1 - Initial accepted capability evaluation model.
```
