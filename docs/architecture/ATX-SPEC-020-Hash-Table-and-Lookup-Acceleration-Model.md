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
