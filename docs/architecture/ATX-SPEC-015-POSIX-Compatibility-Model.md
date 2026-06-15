# ATX-SPEC-015 POSIX Compatibility Model

## Status

Draft v0.1 scaffold

## Purpose

This document defines the Atarix POSIX Compatibility Model.

The POSIX Compatibility Model describes how Unix-style applications and interfaces are represented on top of native Atarix objects, services, capabilities, policies, mailboxes, audit, lifecycle, and recovery semantics.

## Core Rule

```text
POSIX is a compatibility personality.
POSIX is not native authority.
```

POSIX compatibility exists to run software.

POSIX compatibility must not weaken native Atarix authority, policy, audit, lifecycle, recovery, or supervisor isolation rules.

## Relationship To Other Specifications

This specification depends on:

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
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

This specification informs:

- ATX-SPEC-020 Hash Table and Lookup Acceleration Model
- ATX-SPEC-021 Memory and Data Movement Model

## Design Philosophy

POSIX compatibility is a boundary layer.

The POSIX view is a projection.

The Atarix model remains authoritative.

A compatibility environment may expose familiar concepts such as:

```text
Processes
Threads
Files
Directories
Descriptors
Pipes
Sockets
Signals
Environment variables
Working directories
Permission metadata
```

Internally, these must map to:

```text
Objects
Capabilities
Services
Mailbox endpoints
Resource allocations
Policy decisions
Audit records
Lifecycle states
Recovery state
```

## Compatibility Environment

A POSIX compatibility environment is an object or service-owned environment.

Suggested service path:

```text
/service/compatibility/posix
```

A compatibility environment has:

```text
Environment identity
Version
Generation
Owner
Policy scope
Lifecycle state
Resource limits
Namespace view
Capability set
Audit policy
Recovery policy
```

## Mapping Rules

The following rules apply:

```text
Path lookup is not access.
Descriptor possession is not native authority.
Process identity is not native authority.
Compatibility roles are not native omnipotence.
Signals are compatibility events, not native control.
Device-like endpoints must be service-mediated.
Recovery does not restore compatibility authority without reconciliation.
```

## Profiles

Atarix may define compatibility profiles.

Examples:

```text
POSIX_MINIMAL
POSIX_USERLAND
POSIX_NETWORKED
POSIX_DEVELOPMENT
POSIX_LEGACY
```

Each profile defines which compatibility features are supported, restricted, emulated, or denied.

## Audit Requirements

Authority-bearing compatibility operations must be auditable.

Audit should record both:

```text
Compatibility view
Native Atarix authority decision
```

## Recovery Requirements

Recovery must reconcile compatibility state with native Atarix state.

Examples:

```text
Descriptor table vs native capabilities
Path bindings vs directory state
Process table vs object lifecycle
Socket state vs network service state
Storage view vs storage objects
Audit continuity
```

If reconciliation fails, the compatibility environment remains degraded or quarantined.

## Initial POSIX Sprint Scope

POSIX Sprint 1 should define:

```text
Compatibility environment metadata
Compatibility profile enum
Identity label mapping
Path lookup mapping
Descriptor table model
Basic open/read/write/close mapping
Basic program replacement mapping
Basic error projection
Audit hooks
Basic tests
```

## Required Future Work

- Define POSIX profile registry.
- Define descriptor record format.
- Define compatibility error mapping table.
- Define compatibility audit event additions.
- Define descriptor inheritance policy.
- Define process creation compatibility strategy.
- Define socket compatibility strategy.
- Define minimal libc target.
- Define FOSS application porting guide.

## Summary

POSIX compatibility is a boundary layer for running existing software.

It is not native Atarix authority.

Its central rules are:

```text
POSIX is a compatibility personality.
Path lookup is not access.
Descriptors are compatibility handles.
POSIX compatibility must not weaken native Atarix security semantics.
```
