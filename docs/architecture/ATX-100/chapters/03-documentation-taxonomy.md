# Chapter 3: Documentation Taxonomy

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/engineering/Documentation-Taxonomy-and-Style-Guide.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/engineering/Documentation-Taxonomy-and-Style-Guide.md -->
### Integrated source: `docs/engineering/Documentation-Taxonomy-and-Style-Guide.md`

# Documentation Taxonomy and Style Guide

## Purpose

This guide defines the initial documentation taxonomy, naming conventions, and lifecycle rules for Atarix documentation.

## Document Classes

Atarix uses these document classes:

- ATX-100: Master architecture reference.
- ATX-DESIGN: Design philosophy and guiding principles.
- ATX-SPEC: Normative architecture and implementation requirements.
- ATX-REVIEW: Architecture review records and reconciliation findings.
- ATX-ENG: Engineering process, gate, tooling, and release documentation.
- Archive: Historical frozen documents retained for traceability.

## Lifecycle States

Documents may use these lifecycle states:

```text
Concept
Draft
Reviewed
Verified
Integrated
Archived
```

## Metadata

Every major document SHOULD include a YAML front matter block with:

- document
- title
- type
- status
- lifecycle
- version
- canonical_owner

## Generated Material

Generated material belongs under docs/generated or an AEMS output directory.

Generated material must identify its source command and input files.

## Archival Rule

Superseded documents are not deleted. They are archived and linked to their canonical successor.

<!-- AEMS-MIGRATED-SOURCE: docs/roadmap.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/roadmap.md -->
### Integrated source: `docs/roadmap.md`

# ATARIX Roadmap

## Phase 1 - Rev A

Objectives:

- Build first hardware
- Validate W65C816 operation
- Establish monitor environment
- Verify memory subsystem

## Phase 2 - Rev B

Objectives:

- Modularization
- Recovery systems
- Networking
- Backplane planning

## Phase 3 - Rev C

Objectives:

- FPGA chipset services
- Interrupt routing
- Mailboxes
- DMA foundations

## Phase 4 - Rev D

Objectives:

- Multiprocessor experimentation
- Resource mediation
- Security architecture

## Phase 5 - Rev E

Objectives:

- Accelerator framework
- Capability broker
- Heterogeneous compute support

## Documentation Goals

- Hardware specifications
- Register maps
- Firmware interfaces
- Security model
- Software architecture

## Long-Term Vision

Create a durable experimental workstation platform capable of supporting operating-system research, FPGA development, secure computing experimentation, and heterogeneous compute research.
