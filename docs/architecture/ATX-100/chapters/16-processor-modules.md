# Chapter 16: Processor Modules

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-005-Evolution-Strategy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-005-Evolution-Strategy.md -->
### Integrated source: `docs/design/ATX-DESIGN-005-Evolution-Strategy.md`

# ATX-DESIGN-005: Evolution Strategy

## Purpose

This document defines how Atarix evolves without losing architectural coherence.

## Principle

Processor modules, implementation technologies, and performance mechanisms may change. Architectural invariants require explicit review before change.

## Initial Evolution Rules

- New CPU cards should consume fabric services rather than redefine them.
- New services require specifications and review.
- Stable architectural concepts migrate into ATX-100.
- Superseded working documents are archived for historical reference.
- AEMS should record lifecycle transitions.

<!-- AEMS-MIGRATED-SOURCE: docs/hardware.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/hardware.md -->
### Integrated source: `docs/hardware.md`

# ATARIX Hardware Architecture

## Overview

ATARIX is intended to evolve from a simple W65C816 single-board computer into a modular workstation architecture built around CPU cards, an active backplane, FPGA-mediated services, and heterogeneous compute modules.

## Processor Strategy

### Initial Processor

- W65C816S
- Native mode operation
- Banked memory model
- Observable external bus

### Future Processor Modules

- Single CPU card
- Dual CPU experimentation
- SMP research configurations
- FPGA-assisted interprocessor communication

## Backplane Architecture

The long-term goal is an active backplane rather than a passive bus.

Responsibilities:

- Slot identification
- Arbitration
- Interrupt distribution
- Discovery services
- DMA mediation
- Recovery isolation

## FPGA Strategy

Initial target:

- Lattice ECP5 family
- ULX3S development platform during early development

Early responsibilities:

- Address decode
- Interrupt routing
- Mailboxes
- Timers

Later responsibilities:

- DMA control
- Fabric services
- Accelerator attachment
- Capability enforcement assistance

## Supervisor Controller

Responsibilities:

- Power sequencing
- Reset management
- Recovery support
- Configuration storage
- FPGA loading
- Fault logging

## Memory Architecture

Early revisions:

- ROM
- SRAM

Future revisions:

- Expanded memory windows
- DMA-visible regions
- Capability-controlled mappings

## Networking

Initial networking:

- W5500 Ethernet controller

Future networking:

- Fabric-aware services
- Remote management
- Distributed experimentation

## Design Objectives

- Serviceability
- Observability
- Recoverability
- Incremental complexity
- Long service life
