---
document: ATX-100
title: Atarix System Architecture
type: Master Architecture Reference
status: Draft
lifecycle: Draft
version: 0.2
canonical: true
related:
  - ATX-DESIGN-001
  - ATX-DESIGN-002
  - ATX-SPEC-020
  - ATX-SPEC-090
  - ATX-SPEC-091
---

# ATX-100: Atarix System Architecture

## Status

Draft v0.2

## Purpose

ATX-100 is the master architecture reference for Atarix.

It explains the architecture as an integrated system and provides the canonical entry point for contributors, reviewers, implementers, and future maintainers.

ATX-100 is not a replacement for the ATX-SPEC series. Instead, it integrates stable architectural concepts from design documents, specifications, reviews, and implementation evidence into a single living reference manual.

## Documentation Lifecycle

Atarix documentation follows this lifecycle:

```text
Concept
  -> Working Specification
  -> Integrated Architecture
  -> Historical Archive
```

Working specifications capture evolving requirements, alternatives, implementation details, and verification plans.

When a concept stabilizes, the stable architectural material is integrated into ATX-100. The original specification is then reduced to implementation detail or archived for historical traceability.

## Canonical Ownership Rule

Each architectural concept has exactly one canonical home.

Supporting documents may elaborate, implement, test, review, or historically explain that concept, but they must not redefine it.

## Manual Structure

ATX-100 is organized as a living architecture manual:

```text
Part I:    Introduction
Part II:   Architectural Philosophy
Part III:  System Architecture
Part IV:   Hardware Architecture
Part V:    Engineering System
Part VI:   Development Model
Part VII:  Future Directions
Appendix:  Generated Reference Material
```

## Chapter Sources

Initial chapter source files:

- [Chapter 1: Purpose and Scope](chapters/01-purpose-and-scope.md)
- Chapter 2: Historical Context
- Chapter 3: Documentation Taxonomy
- Chapter 4: Architecture Before Implementation
- Chapter 5: Security Before Convenience
- Chapter 6: Human Readability
- Chapter 7: Fabric-Centric Computing
- Chapter 8: Engineering as Architecture
- [Chapter 9: System Layers](chapters/09-system-layers.md)
- Chapter 10: Object Model
- Chapter 11: Capability Model
- Chapter 12: Mailbox Transport
- Chapter 13: Directory and Discovery Services
- Chapter 14: Memory and Data Movement
- [Chapter 15: Lookup Acceleration](chapters/15-lookup-acceleration.md)
- Chapter 16: Processor Modules
- Chapter 17: Fabric Northbridge
- Chapter 18: Supervisor Controller
- Chapter 19: Storage and Networking
- [Chapter 20: AEMS and Engineering Gates](chapters/20-aems-and-engineering-gates.md)
- Chapter 21: Requirements and Traceability
- Chapter 22: Roadmap and Evolution

## Migration Model

Stable explanatory material should migrate into ATX-100 chapter sources.

The originating specification remains responsible for normative detail, implementation detail, test vectors, RTL behavior, C interfaces, and release evidence.

When a working document is superseded by ATX-100 material, the original document is reduced or archived rather than deleted.

## Relationship To Other Documents

ATX-100 references:

- ATX-DESIGN-001 through ATX-DESIGN-007 for design philosophy.
- ATX-SPEC-001 through ATX-SPEC-091 for normative specifications.
- Architecture Reviews for independent assessment and reconciliation history.
- AEMS generated reports for dependency graphs, traceability, issue state, and gate evidence.

## Generated Outputs

Future AEMS book-building commands should generate:

- A complete ATX-100 Markdown edition.
- HTML architecture portal pages.
- PDF architecture manual.
- Dependency graph.
- Traceability matrix.
- Engineering Gate history.
- Release evidence appendix.
