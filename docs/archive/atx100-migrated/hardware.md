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