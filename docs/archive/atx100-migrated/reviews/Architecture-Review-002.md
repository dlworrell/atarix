# Architecture Review 002

## Status

Review complete

## Scope

This review covers the Atarix architecture baseline developed through the current architecture and research pass.

Architecture scope:

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
ATX-SPEC-012 Versioning Model
ATX-SPEC-013 Policy Model
ATX-SPEC-014 Bootstrap Security Model
ATX-SPEC-016 Supervisor Management Fabric
ATX-SPEC-017 Storage and Persistence Model
ATX-SPEC-018 Recovery and Reconciliation Model
ATX-SPEC-019 Service Model
```

Backlog scope:

```text
ATX-SPEC-015 POSIX Compatibility Model
ATX-SPEC-020 Hash Table and Lookup Acceleration Model
ATX-SPEC-021 Memory and Data Movement Model
```

Research scope:

```text
ATX-RESEARCH-002 Data-Oriented Systems Architecture
ATX-RESEARCH-003 Advanced Lookup and Index Structures
ATX-RESEARCH-004 Memory Locality and Allocation Systems
ATX-RESEARCH-005 Zero-Copy and Data Movement Architectures
ATX-RESEARCH-006 Prime Lookup and Sieve Acceleration
```

## Executive Summary

The architecture has reached a coherent baseline.

The central theme is consistent:

```text
Discovery is not authority.
Observation is not control.
Persistence is not authority.
Recovery is not administrative access.
Service location is not service identity.
Visibility is not ownership.
Compatibility is not authority.
```

The architecture is strongest where it separates visibility, identity, authority, policy, lifecycle, and audit.

The largest remaining risks are not inside individual specifications. They are cross-document consistency risks.

## Review Result

```text
Architecture Coherence:        Strong
Security Model Alignment:      Strong
Authority Separation:          Strong
Audit Coverage:                Strong
Recovery Coverage:             Good
Versioning Coverage:           Good
Lifecycle Coverage:            Good
Implementation Specificity:    Intentionally early
Research Capture:              Good and improving
```

## Major Strengths

### 1. Authority Is Consistently Separated

The architecture repeatedly distinguishes identity, lookup, discovery, observation, persistence, recovery, compatibility, and service location from authority.

This is the strongest architectural property of the current system.

### 2. Policy Is Correctly Positioned

The Policy Model places policy between possible authority and exercised authority.

Capabilities describe what could be done.

Policy decides whether it may be done now.

This prevents capabilities from becoming unconditional access grants.

### 3. Recovery Does Not Become Administrative Bypass

The Recovery and Reconciliation Model explicitly prevents recovery from becoming hidden administrative access.

Recovery restores trust.

Recovery does not restore authority.

### 4. Supervisor Isolation Is Coherent

The Supervisor Management Fabric is control-isolated but audit-visible.

This aligns with the broader rule that observation is not control.

### 5. Versioning And Generation Are Properly Separated

The Versioning Model establishes a necessary distinction:

```text
Version    = compatibility
Generation = instance history
```

This distinction should remain mandatory in all future specifications.

### 6. Research Is Now Properly Separated From Architecture

The research backlog explicitly states that architecture specifications are decisions while research items are investigations.

This separation helps prevent promising algorithms or experiments from becoming undocumented architectural requirements.

## Findings

### Finding 001: ATX-SPEC-015 Is Now The Highest Priority Functional Gap

POSIX compatibility is the next major semantic risk.

POSIX concepts such as users, groups, processes, file descriptors, sockets, signals, fork, exec, permissions, and paths must be mapped onto Atarix objects, capabilities, mailboxes, services, policy, audit, and lifecycle.

Recommendation:

```text
Draft ATX-SPEC-015 POSIX Compatibility Model next.
```

### Finding 002: ATX-SPEC-020 And ATX-SPEC-021 Should Remain Research-Informed

Hash table acceleration, lookup structures, memory locality, zero-copy transfer, prime lookup, and succinct indexing are valuable research directions.

They should not harden into architecture until benchmarked and reconciled with authority, audit, recovery, and lifecycle constraints.

Recommendation:

```text
Keep ATX-SPEC-020 and ATX-SPEC-021 in backlog until after POSIX compatibility and at least one benchmark/research synthesis pass.
```

### Finding 003: Lifecycle State Names Need A Reconciliation Pass

Many specifications define lifecycle-like state sets, but their names differ by subsystem.

Examples include:

```text
ACTIVE
DEGRADED
QUARANTINED
FAILED
UNKNOWN
RECOVERY_PENDING
RECOVERY_ACTIVE
REGISTERED
MIGRATING
RETIRED
DESTROYED
```

The variation is reasonable, but common states should be standardized where possible.

Recommendation:

```text
Create a lifecycle state vocabulary appendix or reconciliation table.
```

### Finding 004: Error Codes Need A Cross-Spec Registry

The Error Model defines the concept of structured errors, and later specs introduce domain-specific errors.

These should be consolidated into a registry before implementation begins.

Recommendation:

```text
Create docs/architecture/error-code-registry.md or an appendix to ATX-SPEC-011.
```

### Finding 005: Audit Event Types Need A Cross-Spec Registry

Audit requirements are strong, but audit event names are distributed across many documents.

Bootstrap, supervisor, policy, service, storage, recovery, and versioning each introduce events.

Recommendation:

```text
Create docs/architecture/audit-event-registry.md or an appendix to ATX-SPEC-010.
```

### Finding 006: Authority Classes Need A Compact Registry

The architecture now references several authority classes.

Examples:

```text
Runtime authority
Bootstrap authority
Supervisor authority
Recovery authority
Recovery signing authority
Audit export authority
Observation authority
Service authority
Cleanup authority
```

Recommendation:

```text
Create docs/architecture/authority-class-registry.md or an appendix to ATX-SPEC-002.
```

### Finding 007: Research Mapping Is Now Good Enough For Forward Work

The research backlog now captures:

```text
Data-oriented architecture
Advanced lookup and indexing
Memory locality and allocation
Zero-copy/data movement
Prime lookup and sieve acceleration
```

The backlog correctly maps research to future specifications and target subsystems.

Recommendation:

```text
Keep research separate and require benchmark evidence before adopting research-driven mechanisms into specifications.
```

## Dependency Review

The current high-level dependency structure is coherent.

### Foundation Layer

```text
Security
  -> Authority
      -> Capability
          -> Policy
```

### Object And Naming Layer

```text
Object
  -> Namespace
      -> Directory
```

### Operational Layer

```text
Mailbox
Resource
Audit
Error
Versioning
Lifecycle
```

### Control And Trust Layer

```text
Bootstrap Security
Supervisor Management Fabric
Recovery and Reconciliation
```

### Runtime Layer

```text
Service Model
Storage and Persistence
```

### Future Layer

```text
POSIX Compatibility
Hash Table and Lookup Acceleration
Memory and Data Movement
```

## Cross-Cutting Principles Confirmed

The following principles should be treated as architecture-wide rules:

```text
Discovery is not authority.
Lookup is not access.
Observation is not control.
Persistence is not authority.
Recovery is not administrative access.
Compatibility is not authority.
Service location is not service identity.
Visibility is not ownership.
Unknown authority fails closed.
Unknown compatibility is explicit.
Quarantine beats unsafe execution.
```

## Risks

### Risk 001: POSIX Semantics May Conflict With Native Atarix Semantics

POSIX assumes ambient process identity, file descriptors, permissions, signals, fork, and path-based access.

Atarix is object, service, capability, and policy oriented.

Mitigation:

```text
Define POSIX as a compatibility personality, not native system truth.
```

### Risk 002: Research Optimizations Could Accidentally Weaken Security Semantics

Lookup acceleration, zero-copy movement, and persistent prime/index tables may bypass ordinary authority paths if adopted carelessly.

Mitigation:

```text
Require capability mediation, policy checks, audit hooks, and recovery behavior for optimized paths.
```

### Risk 003: Registry Debt

As specifications grow, distributed lists of states, errors, events, and authority classes may drift.

Mitigation:

```text
Create registries before implementation begins.
```

## Required Follow-Up Work

```text
1. Draft ATX-SPEC-015 POSIX Compatibility Model.
2. Create lifecycle state reconciliation table.
3. Create error code registry.
4. Create audit event registry.
5. Create authority class registry.
6. Later draft ATX-SPEC-020 after research synthesis.
7. Later draft ATX-SPEC-021 after storage, recovery, and POSIX interactions are clearer.
```

## Architecture Review Decision

Architecture Review 002 accepts the current architecture baseline as coherent enough to proceed.

The next specification should be:

```text
ATX-SPEC-015 POSIX Compatibility Model
```

The next reconciliation work should focus on:

```text
Lifecycle states
Error codes
Audit events
Authority classes
```

## Summary

The Atarix architecture is now internally coherent at the conceptual level.

The strongest property of the architecture is the repeated separation of identity, visibility, discovery, persistence, compatibility, recovery, and authority.

The next major challenge is POSIX compatibility, because POSIX will introduce legacy assumptions that must not weaken Atarix-native authority, policy, audit, and recovery semantics.
