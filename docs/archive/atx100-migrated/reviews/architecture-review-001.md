# Architecture Review 001

## Status

Draft review v0.1

## Scope

This review covers the current Atarix architecture foundation:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

## Purpose

This review checks whether the current specifications agree on terminology, authority, identity, ownership, lifecycle, observability, cleanup, failure, and recovery.

The goal is to catch contradictions before implementation hardens them into public APIs.

## Architectural Axioms Under Review

The following axioms are currently treated as controlling architectural rules:

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

## Review Summary

Overall status:

```text
ACCEPTED WITH REQUIRED FOLLOW-UP
```

The current specification set is coherent enough to continue architectural development, but several cross-cutting topics require future specifications or deeper expansion.

## Consistency Findings

### Identity And Names

Status: PASS

The Object, Namespace, and Directory specifications consistently separate object identity from names and bindings.

Confirmed rules:

```text
Names are bindings.
Object identities are opaque.
Lookup does not grant authority.
Directory entries are not capabilities.
```

Required follow-up:

- Define concrete object identity wire format.
- Define path syntax and limits.
- Define whether kernel-visible paths support Unicode or restricted ASCII.

### Authority

Status: PASS WITH FOLLOW-UP

The Security, Authority, Capability, Directory, Resource, and Error specifications consistently reject ambient authority.

Confirmed rules:

```text
Execution does not imply authority.
Identity does not imply authority.
Connectivity does not imply trust.
Lookup does not imply access.
Observation does not imply control.
```

Required follow-up:

- Expand ATX-SPEC-002 Authority Model beyond stub status.
- Define bootstrap authority separately from runtime authority.
- Define supervisor authority separately from operational fabric authority.

### Ownership

Status: PASS WITH FOLLOW-UP

The current specifications consistently treat ownership as accountability rather than unrestricted control.

Confirmed rules:

```text
Ownership is not authority.
Binding ownership and object ownership may differ.
Resource ownership and resource authority may differ.
```

Required follow-up:

- Define ownership transfer semantics.
- Define shared ownership or administrative sponsorship.
- Define owner disappearance behavior more rigorously in Lifecycle Model.

### Lifecycle And Cleanup

Status: PASS WITH FOLLOW-UP

Lifecycle, Resource, Directory, Audit, and Error specifications consistently treat cleanup as a security requirement.

Confirmed rules:

```text
No object is immortal by default.
No resource may silently leak.
Stale bindings are observable.
Cleanup failures are auditable.
Unknown state is explicit.
```

Required follow-up:

- Expand ATX-SPEC-004 Lifecycle Model beyond stub status.
- Define object lifecycle state machine.
- Define lease expiration semantics.
- Define uninstall and owner-disappearance behavior.

### Directory And Lookup

Status: PASS

ATX-SPEC-008 correctly separates discoverability from authority.

Confirmed rules:

```text
Lookup is not access.
Enumeration is not access.
Aliases do not grant authority.
Rebinding requires authority.
```

Required follow-up:

- Define Directory Sprint 1 public C interfaces.
- Define binding ID and generation representation.
- Define directory failure error codes after Error Model matures.

### Resource Management

Status: PASS WITH FOLLOW-UP

ATX-SPEC-009 correctly treats resources as first-class architectural entities with ownership, accounting, leases, quota, cleanup, and audit.

Confirmed rules:

```text
Resource use is authority-bearing.
Resource visibility is not control.
Resource exhaustion must fail safely.
Supervisor resources are control-isolated.
```

Required follow-up:

- Define minimum Rev A resource classes.
- Define resource identity format.
- Define reserved resources for audit, recovery, and supervisor paths.
- Define RAM-card-backed audit buffers once fabric memory services mature.

### Audit

Status: PASS WITH FOLLOW-UP

ATX-SPEC-010 establishes audit as evidence rather than debugging output.

Confirmed rules:

```text
Audit is evidence.
Observation is not control.
Audit suppression must be observable.
Supervisor events are auditable.
Cleanup failures are security events.
```

Required follow-up:

- Define audit record wire format.
- Define audit buffer hierarchy.
- Define RAM-backed audit ingest buffers.
- Define asynchronous audit persistence.
- Define ZFS-inspired or copy-on-write persistent audit storage in future Storage Model.

### Error Handling

Status: PASS WITH FOLLOW-UP

ATX-SPEC-011 correctly treats failure and unknown state as first-class architectural concepts.

Confirmed rules:

```text
Unknown state must be explicit.
Fail closed for authority-bearing operations.
Prefer quarantine over assumption.
Degraded operation must be observable.
Recovery must be auditable.
```

Required follow-up:

- Define concrete error code namespace.
- Define structured error object format.
- Define quarantine state transitions.
- Define recovery workflow semantics.

### Supervisor And Operational Fabric

Status: NEEDS SPECIFICATION

The current documents identify Supervisor Management Fabric and Supervisor Audit Bridge concepts, but a dedicated specification is required.

Confirmed direction:

```text
Operational Fabric contains discovery, control, service, and data planes.
Supervisor Management Fabric is control-isolated.
Operational Fabric may observe supervisor state through audit paths.
Operational Fabric may not directly control supervisor authority.
Observation is not control.
```

Required follow-up:

- Create ATX-SPEC-016 Supervisor Management Fabric.
- Define Supervisor Audit Bridge semantics.
- Define supervisor command request path.
- Define physical vs logical isolation requirements.
- Define cryptographic requirements for off-board supervisor access.

### Bootstrap Security

Status: NEEDS SPECIFICATION

The current Security, Audit, Error, and Resource specifications all reference bootstrap behavior, but bootstrap authority remains undefined.

Confirmed direction:

```text
Bootstrap authority is not runtime authority.
Pre-OS networking is not runtime networking.
Boot audit imports into runtime audit where practical.
Failure before Supervisor enters recovery-only mode.
```

Required follow-up:

- Create ATX-SPEC-014 Bootstrap Security Model.
- Define root of trust.
- Define secure boot and measured boot requirements.
- Define pre-OS network services for netboot and time sync.
- Define recovery mode restrictions.

### Storage And Persistence

Status: NEEDS SPECIFICATION

The current specifications repeatedly reference persistence, crash recovery, journal replay, RAM-backed buffers, and copy-on-write storage, but no Storage and Persistence Model exists yet.

Confirmed direction:

```text
Persistence must be explicit.
Persistent state must be recoverable.
Unknown persistent state must be quarantined.
Audit persistence should be checksummed and crash-recoverable.
ZFS-style semantics are a long-term target, not a Rev A dependency.
```

Required follow-up:

- Create ATX-SPEC-017 Storage and Persistence Model.
- Define persistent object storage.
- Define audit persistence hierarchy.
- Define snapshot and rollback semantics.
- Define recovery after partial writes.

### Recovery And Reconciliation

Status: NEEDS SPECIFICATION

Recovery and reconciliation appear in Lifecycle, Resource, Audit, and Error specifications, but remain cross-cutting.

Confirmed direction:

```text
Recovery must not silently regrant authority.
Unknown state remains visible.
Unreconciled resources are quarantined.
Recovery is auditable.
```

Required follow-up:

- Create ATX-SPEC-018 Recovery and Reconciliation Model.
- Define crash recovery phases.
- Define resource reconciliation.
- Define directory reconciliation.
- Define capability and authority reconciliation.
- Define audit continuity verification.

## Required Future Specifications

The review identifies the following future specifications as required:

```text
ATX-SPEC-012 Versioning Model
ATX-SPEC-013 Policy Model
ATX-SPEC-014 Bootstrap Security Model
ATX-SPEC-015 POSIX Compatibility Model
ATX-SPEC-016 Supervisor Management Fabric
ATX-SPEC-017 Storage and Persistence Model
ATX-SPEC-018 Recovery and Reconciliation Model
```

## Recommended Priority Order

Recommended next specification order:

```text
1. ATX-SPEC-012 Versioning Model
2. ATX-SPEC-013 Policy Model
3. ATX-SPEC-014 Bootstrap Security Model
4. ATX-SPEC-016 Supervisor Management Fabric
5. ATX-SPEC-017 Storage and Persistence Model
6. ATX-SPEC-018 Recovery and Reconciliation Model
7. ATX-SPEC-015 POSIX Compatibility Model
```

Rationale:

- Versioning affects every wire format and persistent record.
- Policy determines conditional authority.
- Bootstrap Security and Supervisor Fabric govern early trust and recovery.
- Storage and Recovery define durable state.
- POSIX compatibility should remain deferred until native semantics are stable.

## Implementation Guidance

No major subsystem implementation should proceed without checking:

```text
Does this expose identity as authority?
Does this turn lookup into access?
Does this turn observation into control?
Does this create ambient authority?
Does this create persistence without policy?
Does this create resources without cleanup?
Does this hide unknown state?
Does this fail open when authority is uncertain?
```

## Review Decision

```text
Decision: ACCEPTED WITH REQUIRED FOLLOW-UP
Reviewer: Architecture Review 001
Date: 2026-06-14
```

## Closing Note

The current architecture is coherent enough to continue specification work, but the next phase must focus on cross-cutting models: versioning, policy, bootstrap trust, supervisor isolation, storage persistence, and recovery.

The strongest recurring Atarix principle is separation of concepts that legacy systems often conflate:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Unknown is not valid.
```
