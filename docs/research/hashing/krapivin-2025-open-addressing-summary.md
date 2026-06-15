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
