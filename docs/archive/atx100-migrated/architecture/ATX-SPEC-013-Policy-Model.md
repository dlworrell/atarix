# ATX-SPEC-013 Policy Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Policy Model.

The Policy Model defines how Atarix evaluates, approves, denies, constrains, delegates, revokes, audits, and reconciles authority-bearing operations.

Policy is not authority.

Policy governs whether authority may be exercised.

## Core Rule

```text
Authority is explicit.
Policy determines whether authority may be exercised.
```

A valid capability describes what could be done.

Policy decides whether it may be done now.

## Architectural Axioms

```text
Policy is not identity.
Policy is not authority.
Policy is not ownership.
Policy is not a capability.
Policy evaluation must be auditable.
Unknown policy state must be explicit.
Policy failure must fail closed.
```

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model

This specification informs:

- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## What Policy Governs

Policy may govern:

```text
Capabilities
Delegation
Revocation
Directory mutation
Resource allocation
Resource reservation
Service registration
Mailbox communication
Lifecycle transitions
Persistence
Recovery actions
Supervisor requests
Compatibility modes
Version negotiation
```

## Policy Layers

Atarix recognizes policy layers.

### Layer 0: Architectural Policy

Hard architectural invariants.

These cannot be overridden.

Examples:

```text
Identity is not authority.
Lookup is not access.
Observation is not control.
Unknown authority fails closed.
```

### Layer 1: Security Policy

Global security rules.

Examples:

```text
No delegation across rings without explicit approval.
Recovery operations require approved authority.
Supervisor requests require mediated approval.
```

### Layer 2: Administrative Policy

Site-specific administrative rules.

Examples:

```text
Maximum RAM allocation
Audit retention period
Allowed service providers
Persistent storage limits
```

### Layer 3: Object Policy

Object-specific rules.

Examples:

```text
Storage object is read-only.
Directory branch is immutable.
Resource pool has quota.
```

### Layer 4: Session Policy

Temporary or session-scoped rules.

Examples:

```text
Maintenance window
Recovery session
Emergency operation
Temporary override
```

Session policy should expire by default.

## Evaluation Order

Policy evaluation must be deterministic.

Conceptually:

```text
Architectural Policy
  -> Security Policy
  -> Administrative Policy
  -> Object Policy
  -> Session Policy
```

Deny overrides allow.

Unknown policy state fails closed for authority-bearing operations.

## Policy Results

Policy evaluation may return:

```text
ALLOW
DENY
DEFER
REQUIRES_APPROVAL
QUARANTINE
UNKNOWN
```

Policy results are not merely Boolean.

## Policy Object

Policy records are objects or object-owned records.

A policy record has:

```text
Policy ID
Version
Generation
Owner
Scope
Lifecycle state
Audit policy
```

Version describes schema compatibility.

Generation describes policy evolution history.

## Policy Scope

Policy may apply to:

```text
System
Fabric
Node
Service
Directory
Resource
Object
User-associated label
Session
Capability
Supervisor request
Compatibility environment
```

## Policy Inheritance

Policies may inherit or compose by scope.

Example:

```text
System
  -> Storage
      -> Dataset
          -> Object
```

Child policy may restrict further.

Child policy may not weaken architectural policy.

## Policy And Authority

Policy does not create authority by itself.

Policy can allow, deny, restrict, require approval, quarantine, or defer use of existing authority.

Policy cannot turn identity, lookup, observation, connectivity, or execution into authority.

## Policy And Capabilities

A capability may be valid but still denied by policy.

A policy may not allow an operation when the required authority does not exist unless a higher authority explicitly defines that exceptional path.

## Policy And Delegation

Delegation requires policy approval.

Policy may:

```text
Allow delegation
Restrict delegation
Deny delegation
Require approval
Require reduced authority
Require expiration
Require audit
```

Delegation may never exceed the source authority.

## Policy And Resources

Policy may constrain resources such as:

```text
Memory
CPU
Storage
Bandwidth
DMA windows
Mailbox capacity
Audit resources
Compatibility environments
Auxiliary compute providers
```

Resource exhaustion policy must not grant broader authority.

## Policy And Supervisor Requests

Supervisor actions are policy-governed.

Examples:

```text
Reset
Power off
Recovery mode
Firmware update
Clock modification
Watchdog modification
```

The Operational Fabric may request supervisor action only through mediated policy-controlled paths.

Supervisor control isolation remains in effect.

## Policy And Compatibility

Compatibility environments are policy-controlled.

Examples:

```text
POSIX personality
Virtual hardware
Legacy protocol translation
Compatibility VM
```

Policy may disable compatibility, restrict it, require quarantine, require audit, or require explicit approval.

Compatibility must not weaken native Atarix semantics.

## Policy And Versioning

Every policy record must contain:

```text
Policy Version
Policy Generation
```

Unknown policy versions must fail closed for authority-bearing operations.

Policy downgrade, migration, or rollback must be auditable.

## Policy And Audit

Every authority-bearing policy decision should be auditable.

Minimum audit fields:

```text
Policy ID
Policy Version
Policy Generation
Actor identity label
Authority used
Operation
Target object or resource
Result
Reason
Timestamp
```

## Policy And Errors

If policy cannot be evaluated, the result is UNKNOWN.

UNKNOWN fails closed for authority-bearing operations.

Policy errors must distinguish:

```text
Policy missing
Policy malformed
Policy unsupported
Policy version unsupported
Policy unavailable
Policy conflict
Policy evaluation failed
```

## Policy Cleanup Escalation Model

Policy may fail, deny, defer, or become unavailable while cleanup is required.

Cleanup is a security function.

Policy failure must not cause garbage, leaked authority, leaked resources, or unbounded persistence.

Atarix therefore defines a cleanup fallback ladder.

### Cleanup Levels

```text
L1 Normal Cleanup
L2 Owner Cleanup
L3 Parent / Session Cleanup
L4 System Lifecycle Cleanup
L5 Recovery Cleanup
L6 Supervisor Recovery Cleanup
L7 Evidence Preservation
L8 Last-Resort Containment
```

### L1 Normal Cleanup

Normal cleanup under the object's active cleanup policy.

Typical actions:

```text
Release temporary resources
Close mailboxes
Remove temporary directory bindings
Expire leases
Flush ordinary audit records
```

### L2 Owner Cleanup

Cleanup under owner authority or owner cleanup policy when object-local cleanup is denied, incomplete, or unavailable.

Typical actions:

```text
Retry cleanup with owner cleanup capability
Release owner-scoped temporary allocations
Mark unresolved state for audit
```

### L3 Parent / Session Cleanup

Cleanup under parent object, session, or scope authority when the owner is unavailable.

Typical actions:

```text
Expire session leases
Mark objects orphan-pending
Revoke session-external capabilities
Audit owner unavailability
```

### L4 System Lifecycle Cleanup

System lifecycle cleanup when object, owner, and session cleanup cannot safely complete.

Typical actions:

```text
Transition object to cleanup-pending
Revoke outward authority
Release safe resources
Mark unsafe resources for quarantine
```

### L5 Recovery Cleanup

Recovery cleanup when ordinary lifecycle cleanup cannot establish safe state.

Typical actions:

```text
Quarantine object or resource
Deny new authority
Preserve recovery metadata
Require reconciliation
```

### L6 Supervisor Recovery Cleanup

Supervisor-mediated recovery cleanup for failures affecting recovery paths, boot state, power state, watchdogs, or critical system state.

Typical actions:

```text
Local-only recovery action
Recovery-mode transition
Supervisor audit event
No new runtime authority
```

### L7 Evidence Preservation

Evidence-preservation fallback when destructive cleanup may destroy information needed for audit, recovery, or incident reconstruction.

Typical actions:

```text
Stop destructive cleanup
Snapshot or preserve state where possible
Quarantine preserved state
Audit heavily
Require manual or recovery review
```

### L8 Last-Resort Containment

Last-resort containment when cleanup safety is unknown and continued exposure is riskier than isolation.

Typical actions:

```text
Revoke external capabilities
Disconnect service endpoints
Freeze mutation
Preserve recovery path
Keep evidence intact where practical
```

## Cleanup Result Mapping

Policy result maps to cleanup behavior as follows:

```text
ALLOW
  -> Normal cleanup may proceed.

DENY
  -> Destructive cleanup denied. Attempt safe release only and escalate if needed.

DEFER
  -> Hold object, retry policy evaluation, and prevent new authority.

REQUIRES_APPROVAL
  -> Suspend object, preserve state, audit, and alert.

QUARANTINE
  -> Isolate object, revoke external authority, preserve evidence.

UNKNOWN
  -> Fail closed and use recovery-only cleanup path.
```

## Cleanup Escalation Rules

Cleanup escalation follows these rules:

```text
Cleanup must never create broader authority.
Cleanup must never silently delete evidence.
Cleanup must never silently preserve authority.
If cleanup safety is unknown, quarantine beats deletion.
All escalation steps must be auditable.
Escalation may reduce authority.
Escalation may revoke authority.
Escalation may quarantine.
Escalation may release temporary resources when safe.
Escalation may not grant new runtime authority.
```

## Policy Lifecycle

Policies participate in lifecycle management.

Suggested states:

```text
CREATED
ACTIVE
SUSPENDED
DEPRECATED
QUARANTINED
DESTROYED
UNKNOWN
```

Policies are not immortal.

Policy cleanup and replacement must be auditable.

## Policy Language

A future policy language may be defined later.

Rev A should prefer:

```text
Static policies
Structured policies
Table-driven policies
Explicit C-visible policy records
```

before introducing scripting, rules engines, or policy DSLs.

## Initial Policy Sprint Scope

Policy Sprint 1 should implement:

```text
Policy result enum
Policy record metadata
Version and generation fields
Basic allow / deny / unknown evaluation
Cleanup fallback level enum
Audit hooks for policy decisions
Basic tests
```

Policy Sprint 1 should not implement:

```text
Policy language
Distributed policy federation
Complex inheritance
Runtime scripting
Remote administrative policy
```

## Required Tests

Initial tests should verify:

```text
Allow decision
Deny decision
Unknown decision fails closed
Policy version unsupported fails closed
Capability valid but policy denied
Delegation denied by policy
Resource allocation denied by policy
Cleanup escalation from L1 to L5
Evidence-preservation fallback
Last-resort containment fallback
Audit record generated for policy decision
Audit record generated for cleanup escalation
```

## Required Future Work

- Define policy record wire format.
- Define policy result C API.
- Define policy conflict resolution.
- Define policy inheritance rules.
- Define supervisor request policy in ATX-SPEC-016.
- Define bootstrap policy in ATX-SPEC-014.
- Define persistent policy storage in ATX-SPEC-017.
- Define recovery policy reconciliation in ATX-SPEC-018.

## Summary

Capabilities describe what could be done.

Policy decides whether it may be done now.

When policy cannot decide, Atarix fails closed, audits the uncertainty, and escalates cleanup toward quarantine, evidence preservation, or supervisor-mediated recovery rather than permissive deletion or leaked authority.
