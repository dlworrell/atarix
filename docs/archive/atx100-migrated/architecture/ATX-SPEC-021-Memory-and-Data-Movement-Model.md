---
document: ATX-SPEC-021
title: Memory and Data Movement Model
type: Specification
status: Draft
lifecycle: Draft
version: 0.2
canonical_owner: ATX-100
canonical_section: Chapter 14
related:
  - ATX-SPEC-005
  - ATX-SPEC-009
  - ATX-SPEC-017
  - ATX-SPEC-018
  - ATX-SPEC-020
  - ATX-SPEC-091
implements:
  - include/atarix/memory.h
verified_by:
  - memory/test_memory_descriptor
---

# ATX-SPEC-021 Memory and Data Movement Model

## Status

Draft v0.2

## Purpose

This document defines the Atarix Memory and Data Movement Model.

Memory movement is a system resource.

ATX-SPEC-021 defines how Atarix moves data between CPUs, services, mailboxes, accelerators, storage, and fabric-attached devices without weakening authority, policy, audit, recovery, human readability, or compatibility semantics.

This specification covers:

- Memory ownership
- Data movement authority
- Copy, move, map, and share semantics
- Fabric transfer rules
- Zero-copy constraints
- Mailbox payload movement
- Lookup-accelerator memory access
- POSIX compatibility memory behavior
- Audit-window reconstruction for data movement
- Recovery-safe memory transfer behavior

## Core Rule

Data movement is not authority.

A component may copy, move, map, share, cache, stage, or accelerate access to data only after the relevant Atarix authority checks have succeeded.

Reachability is not permission.

No bus master, fabric node, lookup accelerator, cache, descriptor, or compatibility layer may treat physical addressability as authority.

## Human-Readable Abstraction Rule

Atarix must preserve human-readable architecture even when data movement is optimized.

Every optimized data movement path must provide an explainable projection.

Examples:

```text
copy operation      -> source, destination, length, authority, generation
shared mapping      -> owner, borrower, lifetime, revocation rule
fabric transfer     -> service route, mailbox sequence, audit event
cache promotion     -> original object, generation, validation rule
```

No memory optimization may become opaque authority.

## Audit Window Rule

Privileged or cross-domain data movement must be reconstructable inside a bounded audit window.

An audit window should answer:

- Who requested the movement?
- What object, buffer, page, record, or mailbox was involved?
- Which authority and policy checks applied?
- What source and destination domains participated?
- What generation, version, or lifecycle window applied?
- Was the operation completed, denied, aborted, retried, reconciled, or quarantined?

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
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model
- ATX-SPEC-020 Hash Table and Lookup Acceleration Model

This specification informs:

- Future fabric controller RTL
- Persistent storage pipelines
- POSIX compatibility memory behavior
- Lookup accelerator memory access
- Service-to-service payload routing
- Zero-copy data-plane design

## Design Goals

Atarix memory and data movement should provide:

- Explicit ownership
- Explicit authority checks
- Bounded transfer semantics
- Auditable movement
- Recovery-safe completion rules
- Low copy overhead where safe
- Clear separation between control-plane and data-plane behavior
- Compatibility with mailbox routing
- Compatibility with POSIX expectations
- Hardware-friendly bus and fabric behavior

Performance is valuable only when it preserves architecture semantics.

## Initial Movement Forms

Atarix initially defines these movement forms.

### Copy

A copy creates a distinct destination representation while preserving the source.

Copies must preserve provenance metadata when crossing authority, policy, audit, or recovery boundaries.

### Move

A move transfers ownership or active responsibility from one domain to another.

Moves must have explicit lifecycle and recovery semantics.

### Map

A map creates an addressable view into data owned by another object, service, or memory domain.

Mappings must be revocable unless explicitly declared permanent by native object state and policy.

### Share

A share allows multiple authorized parties to access the same data under defined rules.

Sharing requires explicit authority, policy, lifetime, and recovery behavior.

### Fabric Transfer

A fabric transfer moves data without ordinary CPU copy loops.

A fabric engine is never authority.

Fabric movement must operate through explicit descriptors validated by capability, policy, lifecycle, generation, and recovery rules.

### Zero-Copy Path

Zero-copy is an optimization, not a permission model.

A zero-copy path may avoid data copying only when ownership, lifetime, authority, policy, audit, and recovery requirements remain explicit.

## Ownership Rule

Every data buffer must have an owner.

Ownership may belong to an object, service, mailbox domain, storage domain, compatibility personality, or fabric endpoint.

Borrowed access must identify:

- Owner
- Borrower
- Access mode
- Lifetime
- Generation
- Revocation rule
- Recovery outcome

## Descriptor Rule

External memory movement must be described by explicit descriptors.

A movement descriptor is not authority.

A descriptor may describe:

- Source identity
- Destination identity
- Source address or object reference
- Destination address or object reference
- Length
- Movement form
- Required access mode
- Expected generation
- Completion rule
- Audit projection rule
- Recovery rule

A descriptor must be validated before execution.

## Canonical Movement Descriptor v1

The initial public descriptor is `atarix_memory_descriptor_v1_t`.

It is intended to be small enough for mailbox-mediated setup and explicit enough for audit, validation, and future fabric execution.

Required fields:

```text
magic              descriptor format marker
version            descriptor version
flags              movement form and validation flags
source_object      source object, mailbox, buffer, or service identifier
source_offset      source-local byte offset
destination_object destination object, mailbox, buffer, or service identifier
destination_offset destination-local byte offset
length             requested byte length
capability_id      explicit capability authorizing the movement
generation         expected source or transfer generation
audit_hint         human-readable projection selector
```

Descriptor validation MUST reject:

- Null descriptors
- Invalid magic
- Unsupported version
- Zero-length transfers
- Missing capability identifiers
- Missing source identifiers
- Missing destination identifiers
- Invalid or reserved movement forms
- Lengths exceeding implementation limits

## Initial Movement Flags

Initial movement forms are encoded as flags:

```text
ATARIX_MEMORY_MOVE_COPY
ATARIX_MEMORY_MOVE_MOVE
ATARIX_MEMORY_MOVE_MAP
ATARIX_MEMORY_MOVE_SHARE
ATARIX_MEMORY_MOVE_FABRIC
ATARIX_MEMORY_MOVE_ZERO_COPY
```

Exactly one movement form should be selected for the initial validation profile.

## Initial Validation Statuses

Initial validation statuses:

```text
ATARIX_MEMORY_STATUS_OK
ATARIX_MEMORY_STATUS_NULL_DESCRIPTOR
ATARIX_MEMORY_STATUS_BAD_MAGIC
ATARIX_MEMORY_STATUS_BAD_VERSION
ATARIX_MEMORY_STATUS_ZERO_LENGTH
ATARIX_MEMORY_STATUS_MISSING_CAPABILITY
ATARIX_MEMORY_STATUS_MISSING_SOURCE
ATARIX_MEMORY_STATUS_MISSING_DESTINATION
ATARIX_MEMORY_STATUS_BAD_FLAGS
ATARIX_MEMORY_STATUS_LENGTH_EXCEEDED
```

## Mailbox Interaction

Mailbox messages may carry small payloads directly or may reference external buffers, descriptors, or data windows.

External references must be:

- Capability-checked
- Policy-checked
- Generation-aware
- Bounded by length
- Auditable
- Recovery-safe
- Non-authoritative by themselves

Mailbox control flow remains separate from bulk data movement.

## Lookup Accelerator Interaction

ATX-SPEC-020 lookup accelerators may fetch, inspect, or compare memory only through ATX-SPEC-021-compliant movement rules.

Lookup accelerators must not infer authority from reachable memory.

Accelerated reads must preserve:

- Registry identity
- Table generation
- Probe bounds
- Capability result
- Policy result
- Audit projection
- Recovery status

## POSIX Compatibility Interaction

POSIX compatibility memory behavior is a personality mapping over native Atarix memory movement.

POSIX read, write, mmap, pipe, socket, and descriptor behavior must not bypass native ownership, authority, audit, or recovery rules.

## Recovery Rule

Every cross-domain data movement operation must have a defined recovery outcome.

Valid outcomes include:

- Completed
- Denied
- Aborted before visibility
- Reconciled
- Retried under a new generation
- Quarantined

Unrecoverable ambiguous movement is forbidden.

## Initial Implementation Plan

The initial implementation should add:

- `include/atarix/memory.h`
- `src/memory.c`
- `tests/memory/test_memory_descriptor.c`
- Automake integration for the memory descriptor test
- AEMS traceability metadata
- Engineering Gate coverage through `make check`

## Initial Test Plan

The initial unit test must validate:

- A valid copy descriptor succeeds.
- Null descriptor fails.
- Bad magic fails.
- Bad version fails.
- Zero length fails.
- Missing capability fails.
- Missing source fails.
- Missing destination fails.
- Unknown flags fail.
- Excessive length fails.

## Open Questions

- Which transfers require pre-commit audit rather than post-commit audit?
- How should zero-copy lifetimes be represented in the object model?
- How should POSIX mmap semantics map onto native Atarix movement forms?
- What minimum hardware signals are required for an ECP5 transfer descriptor gate?

## TODO

- Implement canonical movement descriptor layout.
- Add descriptor validation helper.
- Add tests and wire them into `make check`.
- Add ATX-100 Chapter 14 once this specification stabilizes.
