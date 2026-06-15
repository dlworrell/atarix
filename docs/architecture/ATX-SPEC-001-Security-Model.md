# ATX-SPEC-001 Security Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the mandatory security architecture of Atarix.

The Security Model exists to make compromise expensive, contained, observable, recoverable, and difficult to expand. It does not claim that exploitation can be made impossible.

## Core Doctrine

Atarix assumes:

- Software defects exist.
- Remote code execution may occur.
- Services may be compromised.
- User identities may be stolen.
- Internal nodes are not inherently trusted.
- Connectivity is not trust.
- Cleanup failures become security failures.
- Unknown state is expected.

Security is therefore centered on explicit authority, containment, lifecycle control, and auditability.

## Architectural Axioms

The Security Model depends on these axioms:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Execution is not authority.
Ownership is not authority.
Allocation is not ownership.
Persistence is not ownership.
Connectivity is not trust.
Bootstrap authority is not runtime authority.
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```

## Authority Rule

Code execution never creates authority.

A process, object, service, driver, mailbox endpoint, CPU card, fabric node, or compatibility environment may only perform operations for which it has explicit authority.

Authority may be derived from:

```text
Ownership policy
Capabilities
Delegation
Ring policy
Supervisor-mediated recovery authority
Bootstrap authority during boot only
Administrative policy
```

Authority may not be derived from:

```text
Identity alone
User name alone
Object name alone
Directory lookup
Physical location
Network reachability
Execution context
System component status
```

## Security Evaluation

A normal runtime access decision is constrained by:

```text
Object identity
  -> Ring policy
  -> Ownership / authority policy
  -> Capability validation
  -> Revocation state
  -> Expiration state
  -> Lifecycle state
  -> Resource policy
  -> Operation
  -> ALLOW / DENY
```

If any required security state cannot be verified, the operation must fail closed.

## Ring And Capability Separation

Rings provide coarse-grained boundaries.

Capabilities provide fine-grained authority.

A capability does not automatically bypass ring policy.

A ring position does not automatically grant capability authority.

Cross-ring operations must be explicit, mediated, and auditable.

## Directory And Security

The Directory Service provides discoverability, not authority.

The following are explicitly false:

```text
Knowing a name grants authority.
Resolving a name grants authority.
Enumerating a directory grants authority.
Owning a name grants authority over the target object.
```

Directory state may influence policy decisions, but directory lookup itself is not access.

## Resource Security

Resource use is authority-bearing.

Resource visibility does not grant control.

Resource allocation must be explicit, owned, accounted, auditable, and reclaimable.

Resource exhaustion must not produce broader authority as a fallback.

Reserved recovery, supervisor, and audit resources must be protected from normal workload exhaustion.

## Audit Security

Audit is evidence.

Audit authority and operational authority are separate.

The ability to read audit data does not imply the ability to control the audited component.

The ability to modify system state does not imply the ability to modify audit history.

Audit suppression, loss, exhaustion, and degradation must be observable.

## Error Security

Unknown state must be explicit.

Unknown state may require degraded operation, quarantine, recovery, or manual review.

Unknown state must not be silently treated as valid.

Authority-bearing operations must fail closed when security state is unknown.

## Supervisor Security

The Supervisor Management Fabric is control-isolated from the Operational Fabric.

The Operational Fabric may observe supervisor state through authorized audit or observation bridges.

The Operational Fabric may not directly control supervisor resources.

Supervisor resources include reset, watchdog, RTC, power, recovery, secure boot state, firmware validation state, and fault logs.

Observation is not control.

## Bootstrap Security

Bootstrap authority is not runtime authority.

Before the runtime security model exists, only narrowly scoped bootstrap authority may exist.

Bootstrap authority may validate firmware, initialize minimal hardware, fetch boot data, record boot audit, start supervisor services, or enter recovery mode.

Bootstrap authority may not become normal runtime administrative authority.

## Compatibility Security

Compatibility environments, including future POSIX or virtual hardware personalities, are objects.

They must have explicit capabilities, bounded resources, lifecycle policy, audit visibility, and cleanup.

Compatibility must not weaken native Atarix security semantics.

## Required Security Events

The following must be auditable:

```text
Capability grant
Capability revoke
Delegation
Authorization denial
Policy override
Ring violation
Resource allocation
Resource exhaustion
Directory mutation
Supervisor action
Bootstrap security failure
Recovery mode entry
Quarantine transition
Cleanup failure
```

## Failure Rule

Atarix prefers denial, quarantine, and explicit degraded operation over assumed safety.

The controlling rule is:

```text
When authority cannot be verified, fail closed.
```
