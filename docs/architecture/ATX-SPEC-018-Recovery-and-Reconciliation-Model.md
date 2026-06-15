# ATX-SPEC-018 Recovery and Reconciliation Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Recovery and Reconciliation Model.

Recovery describes how Atarix returns from failure, degradation, crash, corruption, version mismatch, audit discontinuity, or unsafe state to a known and policy-approved operating state.

Reconciliation describes how Atarix proves that system subsystems agree with each other after failure or recovery.

Recovery is not administrative access.

Recovery is not authority restoration.

## Core Rule

```text
Recovery restores trust.
Recovery does not restore authority.
```

Recovery may verify, repair, reconcile, rebuild, quarantine, replay, or report state.

Recovery may not bypass policy, create ownership, grant arbitrary capabilities, suppress audit, or silently restore authority.

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
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-019 Service Model

This specification informs:

- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-021 Memory and Data Movement Model

## Design Philosophy

Failure is expected.

Recovery must prove safety rather than assume it.

The system may restart, reload, replay, rebuild, or reattach components, but none of those actions automatically restore authority.

The system is not fully recovered until required reconciliation has completed.

## Recovery Domains

Recovery may apply to:

```text
Bootstrap state
Supervisor state
Directory state
Storage state
Service state
Resource state
Capability state
Audit state
Policy state
Compatibility environments
Memory and buffer state
```

Each domain may require different recovery procedures, but all recovery domains must preserve auditability and fail closed when authority is uncertain.

## Recovery Authority

Recovery authority is a distinct authority class.

Recovery authority may:

```text
Inspect state
Verify integrity
Replay journals
Import audit records
Rebuild indexes
Reconcile metadata
Enter quarantine
Produce recovery reports
Request supervisor-mediated recovery
```

Recovery authority may not:

```text
Create administrator authority
Grant arbitrary capabilities
Bypass policy
Suppress audit
Silently override ownership
Silently restore revoked authority
```

Recovery authority is temporary and scoped.

## Recovery States

Suggested recovery states:

```text
RECOVERY_PENDING
RECOVERY_ACTIVE
RECOVERY_DEGRADED
RECOVERY_RECONCILING
RECOVERY_QUARANTINED
RECOVERY_COMPLETE
RECOVERY_FAILED
RECOVERY_UNKNOWN
```

Unknown recovery state must be explicit.

## Recovery Objects

Recovery artifacts are objects.

Examples:

```text
Recovery Manifest
Recovery Journal
Recovery Snapshot
Recovery Report
Recovery Audit Package
Recovery Image
Recovery Policy
Recovery Signing Record
```

Recovery objects are versioned, lifecycle-managed, auditable, and policy-controlled.

## Reconciliation

Recovery and reconciliation are distinct.

Recovery asks:

```text
Can the system operate?
```

Reconciliation asks:

```text
Does the system agree with itself?
```

A system may be booted but not reconciled.

A system may be partially operational but still degraded, quarantined, or pending reconciliation.

## Reconciliation Targets

Reconciliation may compare:

```text
Directory vs Object Store
Directory vs Service Registry
Storage vs Audit
Capabilities vs Objects
Capabilities vs Revocation State
Resources vs Allocations
Services vs Directory Bindings
Supervisor vs Runtime State
Bootstrap Audit vs Runtime Audit
Policy Records vs Runtime Policy Cache
```

Mismatch must be explicit.

## Reconciliation Outcomes

Reconciliation may produce:

```text
CONSISTENT
REPAIRED
REBUILT
QUARANTINED
MANUAL_REVIEW_REQUIRED
FAILED
UNKNOWN
```

Unknown reconciliation state must fail closed for authority-bearing operations that depend on the unreconciled state.

## Journals

Recovery may use journals.

Examples:

```text
Directory Journal
Storage Journal
Audit Journal
Service Registration Journal
Resource Allocation Journal
Policy Journal
```

Journal replay must be:

```text
Version-aware
Generation-aware
Deterministic where practical
Auditable
Policy-controlled
```

Journal replay must not silently restore authority.

## Snapshots

Recovery snapshots are objects.

Recovery snapshots should be:

```text
Read-only by default
Versioned
Auditable
Integrity-checked
Policy-controlled
```

A snapshot preserves state.

A snapshot is not authority.

## Quarantine

Quarantine isolates unsafe or uncertain state while preserving evidence.

Quarantine is required when:

```text
Ownership is uncertain
Authority is uncertain
Integrity is uncertain
Version is unsupported
Policy cannot be evaluated
Audit continuity is broken
Provider behavior is unsafe
Storage provenance is uncertain
```

Core rule:

```text
When trust cannot be established, quarantine beats execution.
```

## Recovery And Audit

Every recovery action should generate audit.

Minimum fields:

```text
Recovery ID
Actor identity label
Recovery authority used
Object or resource identity
Operation
Result
Reason
Timestamp
Version
Generation where applicable
```

Audit continuity must be reconciled before recovered state is considered fully trusted.

## Recovery And Bootstrap

Recovery integrates with the Bootstrap Security Model.

Conceptual flow:

```text
Boot failure
  -> Recovery mode
  -> Signed recovery netboot image where needed
  -> Runtime startup
  -> Runtime recovery import
  -> Reconciliation
  -> Normal, degraded, or quarantined operation
```

A system booted from a recovery-generated image is not fully recovered until reconciliation completes.

## Recovery And Supervisor

Supervisor may trigger or coordinate recovery.

Examples:

```text
Watchdog fired
Power failure detected
Firmware validation failure
Storage fault detected
Runtime failed to report liveness
Operational fabric wedged
```

Supervisor-triggered recovery must be auditable.

Supervisor triggering recovery does not automatically determine the recovery outcome.

## Recovery And Storage

Storage recovery may include:

```text
Checksum verification
Scrubbing
Journal replay
Snapshot recovery
Replication repair
Object reconciliation
Quarantine of unsafe objects
```

Corrupted storage must never become authority.

## Recovery And Services

A service restart is not reconciliation.

Before a recovered service becomes active, recovery must verify:

```text
Service identity
Service provider identity
Service version
Service lifecycle state
Service policy
Service authority
Service resource ownership
Audit continuity where applicable
```

## Recovery And Directory

Directory recovery may include:

```text
Binding validation
Generation verification
Namespace rebuild
Alias validation
Tombstone verification
Object target verification
```

Directory corruption must not create authority.

Lookup remains not access.

## Recovery And Capabilities

Capabilities must be revalidated after recovery when their backing state may have changed.

Recovery must verify:

```text
Capability exists
Capability version is supported
Capability target still exists
Capability is not revoked
Capability has not expired
Capability policy still permits use
```

Recovery must not silently re-enable revoked capabilities.

## Recovery And Resources

Resource recovery may include:

```text
Allocation reconstruction
Lease expiration
Quota reconciliation
Orphan detection
Resource quarantine
Resource release
```

Resource leaks discovered during recovery are audit-relevant events.

## Recovery Cleanup Escalation

Recovery cleanup builds on the Policy Cleanup Escalation Model.

Suggested recovery cleanup levels:

```text
L1 Recovery Cleanup
L2 Recovery Quarantine
L3 Recovery Snapshot
L4 Recovery Isolation
L5 Supervisor Recovery
L6 Manual Recovery
```

Escalation moves toward:

```text
More audit
Less authority
More containment
Greater evidence preservation
```

Escalation must not create broader authority.

## Replay

Replay may be used for:

```text
Audit import
Directory reconstruction
Storage reconstruction
Service registration reconstruction
Resource allocation reconstruction
```

Replay must be version-aware, generation-aware, auditable, and deterministic where practical.

Replay must not assume compatibility when versioning is unknown.

## Recovery Signing

Recovery-generated artifacts may require Recovery Signing Authority as defined by the Bootstrap Security Model.

Examples:

```text
Recovery netboot images
Recovery manifests
Recovery reports
Recovery audit packages
```

Recovery signing is not runtime signing.

Recovery signing must be auditable.

## Recovery Completion

Recovery is complete only when:

```text
Integrity is verified
Policy is verified
Audit is imported or continuity status is explicit
Required reconciliation is complete
Required services are restored or explicitly degraded
Unsafe state is quarantined
Recovery report is generated where required
```

If these conditions are not satisfied, the system remains degraded, quarantined, failed, or unknown.

## Recovery Failure

Recovery failure must be explicit.

Recovery failure may result in:

```text
Restricted operation
Read-only operation
Recovery shell with limited authority
Supervisor recovery mode
Manual review
Full stop
```

Recovery failure must not silently become normal operation.

## Compatibility Recovery

Compatibility environments must be recovered as objects.

Legacy assumptions must not weaken native recovery rules.

If a compatibility environment cannot be reconciled safely, it must remain quarantined or disabled.

## Initial Recovery Sprint Scope

Recovery Sprint 1 should define:

```text
Recovery state enum
Recovery authority definition
Recovery object metadata
Recovery audit event identifiers
Reconciliation result enum
Basic journal replay model
Basic quarantine transitions
Basic tests
```

Recovery Sprint 1 should not implement:

```text
Distributed recovery
Automated cluster recovery
Complex policy language
Full ZFS integration
Remote recovery federation
```

## Required Tests

Initial tests should verify:

```text
Recovery authority does not grant runtime authority
Unknown recovery state fails closed
Reconciliation mismatch produces explicit result
Quarantine blocks authority-bearing operations
Journal replay is audited
Recovery does not restore revoked capability
Storage corruption triggers quarantine
Service restart does not imply reconciliation
Bootstrap recovery import requires reconciliation
Recovery completion requires audit status
```

## Required Future Work

- Define recovery object wire format.
- Define recovery manifest schema.
- Define recovery report schema.
- Define journal replay metadata.
- Define reconciliation API.
- Define recovery audit import format.
- Define integration with Storage and Persistence Model.
- Define integration with Memory and Data Movement Model.
- Define manual recovery authority rules.

## Summary

Recovery restores trust by proving consistency, not by assuming it.

The central rules are:

```text
Recovery restores trust.
Recovery does not restore authority.
Recovery is incomplete until reconciliation completes.
When trust cannot be established, quarantine beats execution.
```
