---
document: ATX-100-CH15
title: Lookup Acceleration
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-020
implements:
  - include/atarix/index.h
  - rtl/atarix/atx_spec_020_accelerator.v
verified_by:
  - atx_spec_020_accelerator_ci_tb
  - atx_spec_020_modules_tb
---

# Chapter 15: Lookup Acceleration

## Principle

Lookup acceleration is not authority.

Atarix uses accelerated lookup structures to reduce latency, memory movement, cache misses, synchronization cost, and search cost. These structures may accelerate discovery, filtering, routing, indexing, or selection.

They do not grant permission.

## Canonical Rule

A lookup result identifies a candidate target. It does not authorize access to that target.

Authority still derives from:

- Capability state.
- Policy state.
- Native object state.
- Lifecycle state.
- Recovery state.
- Audit rules.

## Human-Readable Projection

Every accelerated structure must provide an explainable projection suitable for audit and debugging.

Examples:

```text
Runtime hash table -> registry view
Compressed index -> ordered generation window
Bitmap query -> explicit result set
Membership filter -> precheck explanation
```

## Audit Window Requirement

Every accelerated lookup must be reconstructable inside a bounded audit window.

An audit window should answer:

- Who requested the lookup?
- What key was used?
- Which structure was used?
- What version and generation were involved?
- What candidate was returned?
- What authority check followed?
- What policy decision followed?

## Hardware Acceleration

ATX-SPEC-020 defines an initial RTL-backed lookup acceleration path.

The implementation includes SIMD-style control byte probing, scalar comparison reference behavior, Krapivin-style stepping, Elias-Fano-style decoding, and hardware audit-window support.

The RTL simulations validate both correctness and a scalar-vs-SIMD gain proxy.

## Relationship To ATX-SPEC-020

This chapter owns the stable architectural explanation of lookup acceleration.

ATX-SPEC-020 remains responsible for detailed payload definitions, RTL module interfaces, simulation behavior, and verification evidence.

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md -->
### Integrated source: `docs/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md`

# ATX-SPEC-020 Hardware Mailbox Integration Notes

## Status

Draft v0.1

## Purpose

This note records the hardware and mailbox integration decisions for ATX-SPEC-020 Hash Table and Lookup Acceleration Model.

It captures the W65C816 plus ECP5 Fabric Northbridge integration model, corrected message payload direction, RTL correction notes, and implementation TODOs.

## Core Architectural Decision

The lookup accelerator is a service behind the Atarix mailbox framework.

```text
Client
  -> Mailbox frame
  -> Ring authorization
  -> Capability context validation
  -> Lookup service
  -> Audit window commit
  -> Reply
```

The accelerator is not directly exposed as an ambient hardware primitive.

```text
Lookup acceleration is not authority.
```

## Mailbox-First Rule

All hardware lookup acceleration requests must enter through an Atarix mailbox message.

No lookup accelerator may execute a table probe, bitmap query, membership-filter query, Elias-Fano decode, or other accelerated search operation before mailbox authorization succeeds.

## Capability Context Rule

Capability authority should be derived from mailbox context and native capability state.

The caller should not be trusted merely because it supplies a capability-like scalar in the payload.

Recommended direction:

```text
Mailbox header/source/ring context
  -> native capability lookup
  -> policy check
  -> accelerator execution permission
```

Payloads may carry a capability reference or descriptor only if the receiving service validates it against native capability state.

## Registry Identifier Rule

Accelerator requests should not expose raw physical table addresses as caller-controlled authority.

Prefer:

```text
registry_id
table_id
index_id
view_id
```

instead of:

```text
table_base
raw physical pointer
fabric-local memory address
```

The service resolves the identifier to physical placement after authorization.

## CRC Rule

CRC validation is an integrity check, not an authority check.

CRC may detect bus corruption, framing errors, or malformed transfers.

CRC must not be used for authentication, policy approval, authority, or anti-tamper guarantees.

## Deterministic Probe Limit Rule

Every probe-capable lookup structure must define a bounded forward-progress limit.

For hardware acceleration, this limit may map to a fabric down-counter.

If the probe sequence exceeds the limit, the operation must terminate with an explicit sequence error rather than hanging the memory engine.

## Audit Window Rule

The accelerator must emit an audit-window record before reporting successful completion to the CPU when the operation is authority-bearing or policy-relevant.

The audit window should include:

```text
Message sequence
Message type
Source identity
Target service identity
Registry or table identifier
Capability context result
Policy result
Structure type
Generation
Probe count or query window
Result status
Returned object or row identifier where applicable
```

Human-readable text output may be used for early development, but the final format should be a structured audit event that can also project to text.

## Corrected Message Direction

Initial sketch used raw fields such as:

```text
table_base
cap_token
fingerprint
probe_limit
```

Corrected architecture should move toward declarative service requests:

```text
message_type: INDEX_QUERY
registry_id or table_id
query_type
key or key hash
expected generation
probe budget
projection flags
```

The accelerator service chooses the physical implementation.

Clients request results.

Services choose implementations.

## Proposed Message Types

Initial proposed registry additions:

```text
ATARIX_MAILBOX_MESSAGE_INDEX_QUERY
ATARIX_MAILBOX_MESSAGE_INDEX_MUTATE
ATARIX_MAILBOX_MESSAGE_INDEX_RECOVER
ATARIX_MAILBOX_MESSAGE_INDEX_STATUS
```

Legacy or lower-level probe-specific messages may exist inside the service boundary, but should not be the public compatibility interface unless explicitly approved by architecture review.

## Corrected Payload Sketch

The following C sketch is not yet committed to `include/atarix/index.h` because that path was not found in the current repository search results.

It records the corrected direction for a future implementation issue.

```c
#ifndef ATARIX_INDEX_H
#define ATARIX_INDEX_H

#include <stdint.h>
#include "atarix/mailbox.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_MAILBOX_MESSAGE_INDEX_QUERY   0x0020u
#define ATARIX_MAILBOX_MESSAGE_INDEX_MUTATE  0x0021u
#define ATARIX_MAILBOX_MESSAGE_INDEX_RECOVER 0x0022u
#define ATARIX_MAILBOX_MESSAGE_INDEX_STATUS  0x0023u

typedef uint32_t atarix_index_registry_id_t;
typedef uint32_t atarix_index_generation_t;

typedef enum atarix_index_query_type {
    ATARIX_INDEX_QUERY_EXACT      = 0x00u,
    ATARIX_INDEX_QUERY_RANGE      = 0x01u,
    ATARIX_INDEX_QUERY_FILTER     = 0x02u,
    ATARIX_INDEX_QUERY_BITMAP     = 0x03u,
    ATARIX_INDEX_QUERY_ORDERED    = 0x04u
} atarix_index_query_type_t;

#pragma pack(push, 1)

typedef struct atarix_payload_index_query {
    uint32_t registry_id;
    uint32_t key_hash;
    uint32_t expected_generation;
    uint8_t  query_type;
    uint8_t  probe_limit;
    uint8_t  projection_flags;
    uint8_t  reserved;
} atarix_payload_index_query_t;

typedef struct atarix_payload_index_reply {
    uint32_t resolved_id;
    uint32_t resolved_generation;
    uint8_t  status;
    uint8_t  match_offset;
    uint16_t audit_window_id;
} atarix_payload_index_reply_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_INDEX_H */
```

## RTL Correction Notes

The RTL sketches discussed so far are not yet a verified implementation.

Known corrections before implementation:

```text
1. Replace any typo such as landmark_offset with resolved_lane_offset.
2. Do not expose raw table_base as public authority.
3. Treat caller-provided capability tokens as references requiring native validation, not authority.
4. CRC32 is integrity only, not security.
5. Confirm CRC polynomial, bit reflection, seed, and final XOR against the mailbox framing contract.
6. Ensure the priority encoder has deterministic default behavior.
7. Ensure Elias-Fano select/rank logic is implemented as a verified primitive, not an accidental behavioral loop.
8. Audit commit must occur before successful completion is signaled for authority-bearing operations.
9. Probe-limit exhaustion must return an explicit sequence error.
10. Filter positive results must always be confirmed by authoritative lookup.
```

## ECP5 Northbridge Pipeline Requirement

The hardware pipeline should enforce this order:

```text
Frame receive
  -> Magic/header validation
  -> CRC integrity validation
  -> mailbox ring authorization
  -> capability context validation
  -> policy validation
  -> optional membership precheck
  -> authoritative table/index lookup
  -> ordered/bitmap decode if required
  -> audit-window commit
  -> reply
```

No implementation may move membership probing ahead of authorization when that would leak presence information.

## Elias-Fano Hardware Direction

Elias-Fano acceleration should expose verified rank/select style primitives.

The implementation must produce a human-readable ordered projection for audit windows.

The architecture should not depend on an unbounded bit-by-bit walk.

## Roaring Bitmap Direction

Roaring bitmap routing should be developed as a separate implementation task.

Required properties:

```text
Container type visibility
Query plan auditability
Explicit result window projection
Policy-preserving intersection
Recovery-checkable container metadata
```

## Prime Catalog Direction

Hardware and software table sizing should use signed offline prime catalogs where appropriate.

The catalog is a sizing aid only.

It is not authority.

## Summary

The hardware integration is architecturally sound if implemented as a mailbox-authorized lookup service rather than as a raw probe primitive.

Required corrections are:

```text
Use registry/table identifiers instead of raw addresses.
Validate capability through native mailbox context.
Treat CRC as integrity only.
Make probe limits mandatory.
Commit audit windows before successful completion.
Keep filters non-authoritative.
Expose human-readable projections.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md`

# ATX-SPEC-020 Hash Table and Lookup Acceleration Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Hash Table and Lookup Acceleration Model.

Lookup cost is a system resource.

ATX-SPEC-020 defines how Atarix minimizes lookup latency, memory footprint, cache misses, synchronization overhead, and search cost without weakening authority, policy, audit, recovery, human readability, or compatibility semantics.

This specification covers lookup and indexing for:

```text
Objects
Capabilities
Mailboxes
Services
Namespaces
Directories
Resources
Policies
Audit events
Storage metadata
POSIX compatibility state
Research-driven acceleration structures
```

## Core Rule

```text
Lookup acceleration is not authority.
```

A lookup structure may accelerate discovery, indexing, filtering, routing, caching, or search.

It must not become the source of authority.

Authority still comes from:

```text
Capabilities
Policy
Native object state
Lifecycle state
Recovery state
Audit rules
```

## Human-Readable Abstraction Rule

Atarix must preserve human-readable architecture even when machine-optimized data structures are used.

Every accelerated structure must provide an explainable projection.

```text
Machine-optimized structure
    +
Human-readable audit projection
```

Examples:

```text
Runtime hash table -> registry view
Roaring bitmap -> explicit result set
Elias-Fano index -> ordered event or generation window
Membership filter -> precheck explanation, never authority
```

No compressed, probabilistic, cached, or accelerated structure may become opaque authority.

## Audit Window Rule

Every accelerated lookup must be reconstructable inside a bounded audit window.

An audit window should be able to answer:

```text
Who requested the lookup?
What key was looked up?
Which structure was used?
What version and generation were involved?
What object, service, mailbox, or capability was returned?
What authority check followed?
What policy decision followed?
What time, generation, or version window applied?
```

Lookup acceleration must preserve audit explainability.

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

This specification informs:

- ATX-SPEC-021 Memory and Data Movement Model

Related research:

- ATX-RESEARCH-003 Advanced Lookup and Index Structures
- ATX-RESEARCH-004 Memory Locality and Allocation Systems
- ATX-RESEARCH-005 Zero-Copy and Data Movement Architectures
- ATX-RESEARCH-006 Prime Lookup and Sieve Acceleration

## Design Goals

Atarix lookup structures should provide:

```text
Low average lookup latency
Predictable tail latency
Cache-efficient memory layout
NUMA-friendly placement where applicable
Shard-local operation where applicable
Human-readable projections
Audit-window reconstruction
Recovery-safe rebuild
Version-aware behavior
Generation-aware behavior
Capability-safe lookup
Policy-safe lookup
```

Performance is valuable only when it preserves architecture semantics.

## Lookup Hierarchy

Atarix defines a lookup hierarchy.

```text
Tier 0: Direct identity lookup
Tier 1: Runtime hash tables
Tier 2: Ordered compressed indexes
Tier 3: Membership filters
Tier 4: Bitmap and search structures
Tier 5: Persistent indexes
```

Not every lookup problem is a hash table problem.

Each subsystem should use the lowest-complexity structure that satisfies correctness, performance, auditability, and recovery requirements.

## Tier 0: Direct Identity Lookup

Direct identity lookup is the fastest lookup class.

It applies when an Atarix identifier already carries enough information to directly locate or validate the target through native object state.

Examples:

```text
ObjectID
CapabilityID
MailboxID
ServiceID
GenerationID
VersionID
DescriptorID
AuditEventID
```

Direct identity lookup must still validate lifecycle, generation, authority, and policy where required.

Direct identity lookup is not authority by itself.

## Tier 1: Runtime Hash Tables

Runtime hash tables are used for high-frequency dynamic registries.

Atarix defines the initial canonical runtime table family:

```text
ATARIX_TABLE_V1
```

`ATARIX_TABLE_V1` should be based on open addressing without element reordering, informed by Krapivin-style optimal bounds for open addressing without reordering.

Required properties:

```text
Open addressing
No element reordering during ordinary insertion
Cache-friendly probe sequences
Generation-aware entries
Version-aware table format
Recovery-checkable metadata
Deterministic rebuild behavior
High-load-factor support
Human-readable registry projection
```

Candidate users:

```text
Object Registry
Capability Registry
Mailbox Registry
Service Registry
Descriptor Registry
Policy Registry
Resource Registry
POSIX process table
POSIX descriptor table
```

## Tier 2: Ordered Compressed Indexes

Ordered compressed indexes are used for monotonic identifiers, generation windows, event windows, and version chains.

The initial approved structure family is:

```text
Elias-Fano-style monotonic indexes
```

Used for:

```text
Audit event IDs
Object generation sequences
Storage version chains
Namespace generation lists
Recovery checkpoints
Time-ordered service events
```

Required capabilities:

```text
Ordered traversal
Range lookup
Predecessor/successor lookup where applicable
Compact storage
Human-readable ordered projection
Audit window reconstruction
```

## Tier 3: Membership Filters

Membership filters are precheck structures.

They may reduce expensive lookups.

They are never authoritative.

Approved initial families:

```text
Quotient filters
XOR filters
Cuckoo filters
```

Rules:

```text
A positive filter result is not authority.
A positive filter result must be confirmed by authoritative lookup.
A negative filter result may avoid work only when the filter type and policy permit that use.
Filter version and generation must be auditable.
```

Recommended use:

```text
Quotient filter -> dynamic namespace and directory presence checks
XOR filter -> read-mostly catalogs and static repositories
Cuckoo filter -> dynamic cache membership with deletion support
```

## Tier 4: Bitmap And Search Structures

Bitmap and search structures accelerate query-style lookups.

The initial approved bitmap family is:

```text
Roaring bitmap-style indexes
```

Used for:

```text
Policy matching
Capability search
Mailbox ownership search
Service tag search
Audit event search
Object attribute search
Namespace attribute search
```

Bitmap query results must project to human-readable result sets.

Bitmap intersections must be auditable as query plans and result windows.

## Tier 5: Persistent Indexes

Persistent indexes belong to storage and persistence layers.

They may use structures such as:

```text
B+ trees
LSM trees
Fractal trees
Append-only indexes
Persistent hash indexes
```

Persistent indexes must conform to ATX-SPEC-017 and ATX-SPEC-018.

Persistent lookup structures are not runtime authority.

Storage metadata must reconcile with native object and authority state before use.

## Prime And Sizing Strategy

Atarix may use prime-sized tables, shard sizes, bucket counts, and partition sizes where mathematically justified.

Prime lookup and sizing should use signed offline-generated catalogs rather than repeated runtime prime searches.

This requirement is informed by ATX-RESEARCH-006 and the existing code-noodling prime/sieve work.

Prime catalogs may include:

```text
Prime values
Prime gaps
Recommended table capacities
Recommended resize thresholds
Shard sizing metadata
Filter sizing metadata
Generation and signature metadata
```

Rules:

```text
No authority derives from prime catalogs.
Prime catalogs are sizing aids.
Prime catalogs must be versioned.
Prime catalogs used by trusted components must be signed or otherwise integrity-checked.
Runtime fallback sizing must be deterministic.
```

## Optimization Budget

Atarix permits implementation optimizations only when architecture semantics are preserved.

Required baseline families:

```text
ATARIX_TABLE_V1 runtime hash tables
Roaring bitmap-style query indexes
Elias-Fano-style ordered indexes
Membership filters
Human-readable audit windows
Recovery-safe rebuild
```

Permitted optimization families:

```text
SIMD or vectorized probing
Control-byte probing
Prefetch-aware probe scheduling
NUMA-aware table placement
Per-core or shard-local tables
Hot/cold key separation
Epoch-based reclamation
Perfect hashing for immutable registries
Learned indexes for large read-mostly datasets
GPU or FPGA acceleration for bulk audit/search workloads
Offline signed sizing catalogs
```

Forbidden optimization outcomes:

```text
Opaque lookup authority
Unexplainable cache hits
Probabilistic authority
Unrecoverable indexes
Indexes that bypass policy
Indexes that bypass capability checks
Indexes that bypass audit
Indexes that cannot project human-readable state
```

## Namespace Acceleration

Namespace and directory lookup should use layered acceleration.

Recommended stack:

```text
Membership filter
Runtime hash table
Ordered generation index
```

Example:

```text
Quotient filter
    -> ATARIX_TABLE_V1
        -> Elias-Fano generation index
```

Rules:

```text
Path lookup is not access.
Namespace lookup is not authority.
Directory visibility is policy-controlled.
Accelerated namespace state must reconcile with native directory state.
```

## Mailbox Acceleration

Mailbox lookup and routing should use:

```text
ATARIX_TABLE_V1
Roaring bitmap-style ownership and routing indexes
```

Used for:

```text
Mailbox identity lookup
Mailbox ownership lookup
Subscription lookup
Group routing
Broadcast targeting
Queue accounting
```

All accelerated mailbox state must conform to ATX-SPEC-005 lifecycle rules.

## Service Acceleration

Service discovery and service lookup should use:

```text
ATARIX_TABLE_V1 for service identity lookup
Membership filters for presence checks
Roaring bitmap-style tag indexes
Ordered indexes for service generations
```

Rules:

```text
Service discovery is not service authority.
Service location is not service identity.
Service endpoint names are discovery artifacts.
```

## Policy Acceleration

Policy evaluation may use accelerated structures.

Recommended stack:

```text
ATARIX_TABLE_V1
Roaring bitmap-style policy match sets
Elias-Fano-style generation windows
```

Policy acceleration must preserve the distinction between possible authority and exercised authority.

A cached policy result must remain version-aware, generation-aware, and audit-visible.

## Audit Acceleration

Audit search and audit window construction should use:

```text
Roaring bitmap-style indexes
Elias-Fano-style ordered event indexes
Persistent indexes for long-term storage
```

Audit acceleration must preserve:

```text
Event order
Actor identity
Object identity
Policy decision context
Authority decision context
Generation window
Version window
Integrity status
```

Audit acceleration must never erase native decision context.

## POSIX Compatibility Acceleration

ATX-SPEC-015 compatibility state may use ATX-SPEC-020 structures.

Candidate structures:

```text
POSIX descriptor tables -> ATARIX_TABLE_V1
POSIX process tables -> ATARIX_TABLE_V1
Path caches -> membership filter + table
Network endpoint caches -> table + service binding index
Compatibility audit windows -> bitmap + ordered index
```

Compatibility acceleration must preserve ATX-SPEC-015 rules:

```text
POSIX is a compatibility personality.
Path lookup is not access.
Descriptors are compatibility handles.
Native IPC is mailbox-based.
Compatibility recovery restores execution, not authority.
```

## Recovery Requirements

Every acceleration structure must support recovery behavior.

Required recovery operations:

```text
Snapshot
Verify
Rebuild
Reconcile
Quarantine
Human-readable projection
```

A corrupt index must not become authority.

If an index cannot be verified, the system must use an authoritative fallback path or quarantine the affected lookup domain.

```text
Lookup state is not authority.
Index state is not authority.
Cache state is not authority.
```

## Versioning And Generation Requirements

All acceleration structures must have explicit version and generation metadata.

Required metadata:

```text
Structure type
Structure version
Generation
Owner subsystem
Authority domain reference
Recovery domain reference
Audit projection format
Human-readable projection format
Integrity metadata where required
```

Unknown versions must be explicit.

Unknown versions must not silently downgrade into unsafe behavior.

## Human-Readable Projection Requirements

Every accelerated structure must define a projection format suitable for diagnostics and audit review.

Projection examples:

```text
Registry table -> list of entries by key, object, generation, lifecycle
Bitmap query -> explicit result set plus query terms
Membership filter -> filter metadata and confirmation path
Ordered index -> bounded ordered event/window listing
Persistent index -> storage object mapping plus generation range
```

A human reviewer must be able to inspect what the structure claims and how it relates to native Atarix state.

## Mathematical Foundations Registry

Initial approved mathematical and data-structure foundations:

```text
Krapivin-style open addressing without reordering
Elias-Fano monotonic compressed indexes
Roaring bitmap-style compressed bitmaps
Quotient filters
XOR filters
Cuckoo filters
Prime sizing catalogs
Cache-aware segmented lookup structures
```

Research items do not become architecture merely by appearing in this registry.

A structure becomes normative only when this specification or a later Architecture Review assigns it to a subsystem or table class.

## Required Tests

Initial tests should verify:

```text
Accelerated lookup does not grant authority
Membership filter positives require authoritative confirmation
Membership filter negatives obey policy and filter semantics
Table lookup preserves generation checks
Table lookup preserves lifecycle checks
Bitmap query produces human-readable result set
Audit window can reconstruct accelerated lookup
Corrupt index fails closed
Index rebuild preserves authority boundaries
Version mismatch is explicit
Unknown structure type fails safely
POSIX descriptor lookup preserves ATX-SPEC-015 semantics
Mailbox lookup preserves ATX-SPEC-005 lifecycle semantics
Service lookup preserves service identity vs location distinction
```

## Required Future Work

- Define ATARIX_TABLE_V1 record format.
- Define table class registry.
- Define prime catalog format.
- Define hash function policy.
- Define probe sequence policy.
- Define bitmap query plan format.
- Define ordered index projection format.
- Define membership filter confirmation rules.
- Define benchmark suite.
- Define adversarial lookup tests.
- Define NUMA and shard-local placement rules.
- Define GPU/FPGA bulk search eligibility.
- Reconcile with ATX-SPEC-021 Memory and Data Movement Model.

## Summary

ATX-SPEC-020 defines lookup acceleration for Atarix.

Its central rules are:

```text
Lookup acceleration is not authority.
Optimized lookup must remain human-readable.
Every accelerated lookup must fit inside an audit window.
Probabilistic filters are never authority.
A corrupt index must fail closed or rebuild.
Performance must not weaken architecture semantics.
```

<!-- AEMS-MIGRATED-SOURCE: docs/research/RESEARCH-BACKLOG.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/research/RESEARCH-BACKLOG.md -->
### Integrated source: `docs/research/RESEARCH-BACKLOG.md`

# ATARIX Research Backlog

## Purpose

This document tracks research topics under investigation.

Architecture specifications represent decisions.
Research items represent investigations.

## Active Research Areas

### ATX-RESEARCH-002 Data-Oriented Systems Architecture

Focus:
- Ownership-centric design
- Arena allocation
- Cache-friendly layouts
- Zero-copy transport
- Authority movement vs data movement
- Persistent memory integration
- Recovery-aware design

### ATX-RESEARCH-003 Advanced Lookup and Index Structures

Focus:
- Krapivin open addressing
- Bubble-up d-ary cuckoo hashing
- Modern cuckoo filters
- Roaring bitmaps
- SPIDER rank/select structures
- Elias-Fano indexes
- Succinct data structures

Target Subsystems:
- Directory
- Services
- Capabilities
- Resources
- Audit

### ATX-RESEARCH-004 Memory Locality and Allocation Systems

Focus:
- Arena allocators
- Region allocators
- Generational allocation
- NUMA placement
- Fabric-aware memory placement
- Persistent memory
- Recovery-aware allocation

### ATX-RESEARCH-005 Zero-Copy and Data Movement Architectures

Focus:
- Zero-copy networking
- Scatter/gather I/O
- DMA capability models
- Buffer ownership systems
- Content-addressed transfer
- Fabric transport optimization

### ATX-RESEARCH-006 Prime Lookup and Sieve Acceleration

Focus:
- Existing OSE.c segmented 6-wheel CPU sieve from dlworrell/code-noodling
- Existing OSE_CUDA.cc multi-GPU segmented odd-only CUDA sieve from dlworrell/code-noodling
- JSON prime table export
- Cache-aware segmented sieves
- Bit-packed sieve representations
- Wheel factorization
- Incremental sieves
- Prime-counting algorithms
- Distributed sieve partitioning
- FPGA sieve acceleration
- Persistent prime databases
- Prime lookup services

Target Subsystems:
- Hash and lookup acceleration
- Memory locality research
- Data movement research
- Service-oriented computational fabrics
- Future math service experiments

## Guiding Principle

Research informs architecture.

Research does not become architecture until benchmarked, reviewed, documented, and accepted.

<!-- AEMS-MIGRATED-SOURCE: docs/research/hashing/krapivin-2025-open-addressing-summary.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/research/hashing/krapivin-2025-open-addressing-summary.md -->
### Integrated source: `docs/research/hashing/krapivin-2025-open-addressing-summary.md`

# Research Summary: Optimal Bounds for Open Addressing Without Reordering

## Status

Research note

## Paper

```text
Title: Optimal Bounds for Open Addressing Without Reordering
Authors: Martin Farach-Colton, Andrew Krapivin, William Kuszmaul
Date: 2025-01-04
arXiv: 2501.02305
```

## Repository Use

This note records why the paper is relevant to Atarix and how it may influence future lookup-table architecture.

The paper itself is not vendored into this repository. The repository should track architectural implications, implementation hypotheses, and benchmark requirements rather than storing a static PDF artifact.

## High-Level Relevance

Atarix depends on fast lookup structures in multiple subsystems:

```text
Directory Service
Service Registry
Object Manager
Capability Engine
Resource Manager
Audit Indexing
Error and Metadata Tables
```

Lookup performance affects both runtime speed and security enforcement latency.

A slow lookup path can become a denial-of-service surface or force excessive caching that complicates lifecycle, revocation, and audit semantics.

## Paper Claim To Investigate

The paper revisits open-addressed hash tables and studies insertion without reordering existing elements.

The authors show that open addressing without reordering can achieve stronger expected search complexities than previously believed, with matching lower bounds, and they revisit Yao's earlier optimality conjecture.

## Atarix Architectural Interpretation

The immediate Atarix conclusion is not to adopt a specific algorithm blindly.

The architectural conclusion is:

```text
Lookup structures are architectural performance infrastructure,
not ad hoc implementation detail.
```

Future lookup tables should be specified, versioned, benchmarked, and tested as part of the system architecture.

## Candidate Atarix Applications

### Directory Service

Path and object identity lookup may use hash tables for binding lookup, alias resolution caches, and generation-aware binding indexes.

### Service Registry

Service discovery may require fast lookup by service name, service identity, interface version, provider identity, and policy scope.

### Capability Engine

Capability validation may require fast lookup by capability identity, target object identity, operation, revocation state, and generation.

### Resource Manager

Resource allocation and accounting may require fast lookup by resource identity, owner, allocation ID, quota group, and lifecycle state.

### Audit Index

Audit queries may require fast secondary indexes over event type, object identity, actor label, service identity, and sequence ranges.

## Design Constraints

Any Atarix hash-table design must preserve:

```text
Authority checks
Capability revocation
Policy enforcement
Lifecycle cleanup
Audit visibility
Versioned record formats
Recovery and reconciliation
Resource accounting
Deterministic failure behavior
```

Performance must not override security semantics.

## Candidate Design Direction

Future ATX-SPEC-020 should investigate:

```text
Open-addressed hash tables
No-reordering insertion models
Cache-friendly probing
Versioned table headers
Generation-aware entries
Explicit tombstone policy
Explicit rebuild policy
Transient vs persistent table formats
Crash recovery behavior
Benchmark methodology
```

## Versioning Requirements

Lookup table formats should carry explicit versions.

At minimum:

```text
Table format version
Hash policy version
Probe policy version
Entry schema version
Generation or epoch metadata
```

Unknown table versions must fail closed for authority-bearing lookups.

## Recovery Requirements

Persistent or recovery-relevant tables must define:

```text
Integrity checks
Rebuild-from-source policy
Journal or replay relationship
Tombstone handling
Partial-write behavior
Quarantine behavior
```

A corrupted lookup accelerator must not become corrupted authority.

If a lookup table cannot be trusted, the system should rebuild it, quarantine it, or fall back to a slower authoritative structure.

## Benchmark Requirements

A future benchmark suite should test:

```text
Successful lookup probe counts
Failed lookup probe counts
Insertion cost
Deletion and tombstone behavior
High-load-factor behavior
Cache behavior
W65C816-local feasibility
Fabric-memory feasibility
Auxiliary-compute-card feasibility
Recovery rebuild time
Adversarial key distributions
```

Benchmarks should compare candidate designs against simple baseline tables before adopting complexity.

## Open Questions

```text
What load factors are acceptable for each subsystem?
Which tables are transient and which are persistent?
Which tables must be deterministic across reboot?
Which tables can be rebuilt from authoritative records?
How are tombstones audited or compacted?
Can lookup acceleration live on auxiliary compute cards?
What is the minimum viable Rev A implementation?
```

## Related Architecture Work

```text
ATX-SPEC-008 Directory Service Model
ATX-SPEC-009 Resource Model
ATX-SPEC-010 Audit Model
ATX-SPEC-012 Versioning Model
ATX-SPEC-019 Service Model
ATX-SPEC-020 Hash Table and Lookup Acceleration Model
```

## Summary

The paper should inform Atarix's lookup architecture, but the system must prove the performance benefit under Atarix-specific constraints before relying on it.

The target is not merely a faster hash table.

The target is a secure, auditable, recoverable, versioned lookup infrastructure that can support fast authority and object resolution without weakening the architecture.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/Architecture-Review-002.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/Architecture-Review-002.md -->
### Integrated source: `docs/reviews/Architecture-Review-002.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/Architecture-Review-003.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/Architecture-Review-003.md -->
### Integrated source: `docs/reviews/Architecture-Review-003.md`

# Architecture Review 003

## Status

Complete

## Tracking Issue

- Issue #16: Architecture Review 003: ATX-SPEC-015 POSIX Compatibility Reconciliation

## Scope

This review reconciles ATX-SPEC-015 POSIX Compatibility Model against:

- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Result

PASS

ATX-SPEC-015 is architecture-complete for Revision 1.

## Review Findings

Architecture Review 003 identified five clarification-level findings:

```text
1. Mailbox lifecycle conformance reference
2. Native audit chain participation
3. Degraded-state propagation semantics
4. Subordinate recovery domain statement
5. Service-discovery artifact clarification
```

## Resolution

All five findings were incorporated into ATX-SPEC-015.

Resolved by ATX-SPEC-015 cleanup commit:

```text
5f75de430f6b48082ae0e7305eb693c20502c5d3
```

## Final Status

```text
ATX-SPEC-015 POSIX Compatibility Model
Status: Reconciled
Status: Architecture Complete for Revision 1
Blocking Issues: None
```

## Ready For

```text
ATX-SPEC-020 Hash Table and Lookup Acceleration Model
ATX-SPEC-021 Memory and Data Movement Model
```
