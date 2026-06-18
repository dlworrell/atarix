---
document: ATX-100-CH20
title: AEMS and Engineering Gates
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-090
  - ATX-SPEC-091
implements:
  - tools/aems/aems.py
  - scripts/engineering/run_gate.sh
  - scripts/engineering/check_aems_docs.sh
verified_by:
  - engineering-gate.yml
---

# Chapter 20: AEMS and Engineering Gates

## Engineering As Architecture

Atarix treats engineering evidence as part of the architecture.

A design is not considered stable merely because it is written down or implemented. It must be linked to requirements, reviewed, tested, and verified by the Engineering Gate.

## AEMS

AEMS is the Atarix Engineering Management System.

AEMS is responsible for making repository state discoverable and auditable. Its initial responsibilities include:

- Document discovery.
- Metadata parsing.
- Documentation graph generation.
- Traceability edge reporting.
- Specification index reporting.
- Engineering Gate support.

## Documentation Graph

The documentation graph links documents through relationships such as:

- canonical ownership.
- dependencies.
- related specifications.
- implementation artifacts.
- verification artifacts.
- supersession and archival history.

## Engineering Gate

The Engineering Gate is the repository-level consistency check.

It validates architecture invariants, public headers, RTL lint, RTL simulations, and documentation graph health.

The gate is expected to grow over time into a full engineering certification system covering source code, RTL, documentation, traceability, release evidence, and performance history.

## Release Evidence

A future Atarix release should include generated AEMS evidence:

- document graph.
- traceability matrix.
- Engineering Gate summary.
- architecture manual edition.
- specification index.
- release history.

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-090-Atarix-Engineering-Management-System.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-090-Atarix-Engineering-Management-System.md -->
### Integrated source: `docs/architecture/ATX-SPEC-090-Atarix-Engineering-Management-System.md`

# ATX-SPEC-090: Atarix Engineering Management System

## Purpose

This specification defines AEMS, the Atarix Engineering Management System.

AEMS manages repository-native engineering data, including specifications, design documents, architecture reviews, issues, implementation references, tests, engineering gates, releases, and generated documentation.

## Initial Scope

AEMS SHALL support:

- Document discovery
- Metadata parsing
- Document lifecycle validation
- Specification registry generation
- Dependency graph validation
- Traceability graph validation
- Engineering Gate integration
- Documentation book building

## Non-Goals For v0.1

AEMS v0.1 does not own GitHub Projects synchronization, release publication, or generated PDF production.

## Relationship To ATX-100

AEMS supports ATX-100 by generating document maps, dependency graphs, traceability matrices, and release evidence appendices.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-091-Requirements-and-Traceability-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-091-Requirements-and-Traceability-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-091-Requirements-and-Traceability-Model.md`

# ATX-SPEC-091: Requirements and Traceability Model

## Purpose

This specification defines how Atarix records and validates traceability across architecture, implementation, verification, review, and release artifacts.

## Traceability Chain

Atarix traceability links:

```text
Requirement
  -> Design Decision
  -> Specification
  -> Architecture Review
  -> Issue
  -> Commit
  -> Implementation
  -> Test or Simulation
  -> Engineering Gate
  -> Release
```

## Initial Rules

- Every normative requirement should have a stable identifier.
- Every implementation artifact should identify the requirement or specification it supports.
- Every verified artifact should link to a test, simulation, or gate result.
- Superseded documents must remain traceable through archive metadata.
- AEMS should validate traceability graph consistency.

## Relationship To ATX-100

ATX-100 presents the stable architectural view. ATX-SPEC-091 defines the evidence chain that connects that view to implementation and release state.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-003-Engineering-Principles.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-003-Engineering-Principles.md -->
### Integrated source: `docs/design/ATX-DESIGN-003-Engineering-Principles.md`

# ATX-DESIGN-003: Engineering Principles

## Purpose

This document defines the engineering principles used to develop Atarix.

## Principle

Engineering is part of the architecture.

Atarix links vision, design, specifications, reviews, issues, implementation, simulation, engineering gates, releases, and maintenance into a single traceable process.

## Initial Process Model

```text
Vision
  -> Design
  -> Specification
  -> Architecture Review
  -> Issue Planning
  -> Implementation
  -> Simulation
  -> Engineering Gate
  -> Release
  -> Maintenance
```

## Relationship To AEMS

AEMS exists to automate and verify these engineering relationships.
