# Atarix Architecture Backlog

## Status

Living planning document

## Purpose

This document tracks planned architecture specifications and major architecture follow-up work discovered during specification review.

The backlog prevents future work from being hidden inside implementation tickets or ad hoc discussions.

## Current Foundation

The current architecture foundation includes:

```text
ATX-SPEC-001 Security Model
ATX-SPEC-002 Authority Model
ATX-SPEC-003 Capability Model
ATX-SPEC-004 Lifecycle Model
ATX-SPEC-005 Mailbox Model
ATX-SPEC-006 Object Model
ATX-SPEC-007 Namespace Model
ATX-SPEC-008 Directory Service Model
ATX-SPEC-009 Resource Model
ATX-SPEC-010 Audit Model
ATX-SPEC-011 Error Model
```

Architecture Review 001 accepted the current foundation with required follow-up.

## Planned Specifications

### ATX-SPEC-012 Versioning Model

Priority: High

Purpose:

Define versioning for wire formats, object schemas, capability records, directory bindings, resource records, audit records, error records, firmware, services, storage formats, and mixed-version operation.

Key rule:

```text
Version uncertainty must be explicit.
```

### ATX-SPEC-013 Policy Model

Priority: High

Purpose:

Define policy evaluation, policy objects, policy scope, policy inheritance, policy versioning, administrative policy, site policy, and fail-closed behavior when policy cannot be evaluated.

### ATX-SPEC-014 Bootstrap Security Model

Priority: High

Purpose:

Define root of trust, boot-time authority, secure boot, measured boot, pre-OS networking, time synchronization, recovery mode, bootstrap audit, and handoff into runtime security.

Key rule:

```text
Bootstrap authority is not runtime authority.
```

### ATX-SPEC-015 POSIX Compatibility Model

Priority: Deferred

Purpose:

Define how POSIX/FOSS applications may run on top of Atarix without importing ambient-authority assumptions into the native architecture.

Key rule:

```text
POSIX is a compatibility target, not an architectural foundation.
```

### ATX-SPEC-016 Supervisor Management Fabric

Priority: High

Purpose:

Define the isolated Supervisor Management Fabric, supervisor authority, supervisor resources, supervisor command request path, supervisor audit export, physical/logical isolation, and off-board security requirements.

Key rule:

```text
The Supervisor Fabric is control-isolated but audit-visible.
```

### ATX-SPEC-017 Storage and Persistence Model

Priority: Medium-High

Purpose:

Define persistent object storage, checksummed storage, journaling, copy-on-write storage targets, RAM-backed audit buffers, ZFS-inspired recovery architecture, snapshot semantics, and persistence cleanup.

Key rule:

```text
Persistence must be explicit and recoverable.
```

### ATX-SPEC-018 Recovery and Reconciliation Model

Priority: Medium-High

Purpose:

Define crash recovery phases, quarantine, reconciliation of resources, directory bindings, capabilities, audit continuity, storage state, and post-crash authority restoration rules.

Key rule:

```text
Recovery must not silently regrant authority.
```

### ATX-SPEC-019 Service Model

Priority: Medium

Purpose:

Define services as discoverable, versioned, capability-mediated, location-independent providers of system functionality.

Examples:

```text
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/directory
```

Key rule:

```text
Service location is not service identity.
```

## Cross-Cutting Follow-Up Work

Architecture Review 001 identified these required follow-up areas:

```text
Object identity wire format
Path syntax and limits
Authority Model expansion
Lifecycle state machine
Ownership transfer semantics
Directory binding generation representation
Resource identity format
Audit record wire format
Error code namespace
Quarantine transition semantics
Supervisor Audit Bridge semantics
Bootstrap network security
Storage recovery semantics
```

## Architectural Axioms

Current recurring Atarix axioms:

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

## Recommended Priority Order

```text
1. ATX-SPEC-012 Versioning Model
2. ATX-SPEC-013 Policy Model
3. ATX-SPEC-014 Bootstrap Security Model
4. ATX-SPEC-016 Supervisor Management Fabric
5. ATX-SPEC-017 Storage and Persistence Model
6. ATX-SPEC-018 Recovery and Reconciliation Model
7. ATX-SPEC-019 Service Model
8. ATX-SPEC-015 POSIX Compatibility Model
```

## Implementation Rule

No implementation sprint should silently define behavior that belongs in a planned architecture specification.

If implementation requires unresolved architecture, update this backlog and write the specification before hardening the behavior into public interfaces.
