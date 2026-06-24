# ATARIX Backplane Specification

## Purpose

This document defines the first-pass active-backplane architecture for ATARIX.

The backplane is intended to support modular CPU cards, service modules, FPGA-mediated routing, and future multiprocessor experimentation.

## Design Goals

1. Support a single CPU system initially.
2. Permit future multi-CPU experiments.
3. Avoid requiring every slot to observe every signal.
4. Allow FPGA-assisted arbitration.
5. Support recovery and fault isolation.

## Conceptual Slot Types

### CPU Slot

Hosts:

- W65C816 CPU card
- Future experimental processor modules

### Service Slot

Hosts:

- Networking
- Storage
- Supervisor functions
- Debug hardware

### Fabric Slot

Hosts:

- FPGA fabric modules
- DMA services
- Discovery services

### Accelerator Slot

Hosts:

- ARM modules
- FPGA accelerators
- Future heterogeneous compute devices

## Logical Signal Groups

### Power

Candidate rails:

- +3.3V
- +5V
- Ground
- Auxiliary standby power

### Clocking

Signals:

- System clock
- Reference clock
- Debug clock
- Clock-enable support

### Reset

Signals:

- Global reset
- Slot reset
- Recovery reset

### Interrupts

Candidate structure:

- Interrupt request lines
- Fabric-mediated interrupt routing
- Prioritized delivery

### Mailboxes

Mailbox services support:

- CPU-to-CPU communication
- CPU-to-device communication
- Supervisor messaging

### Discovery

Each slot should expose:

- Vendor identifier
- Device identifier
- Capability description
- Firmware version

## Arbitration Model

Early revisions may use centralized arbitration.

Future revisions may support:

- Bus ownership tracking
- Mailbox ownership
- DMA ownership
- Capability-controlled access

## DMA Considerations

DMA should eventually be mediated through the FPGA fabric.

The fabric should be capable of:

- Granting DMA rights
- Revoking DMA rights
- Detecting violations
- Logging faults

## Recovery Requirements

A failed module should not require complete system replacement.

Desired capabilities:

- Slot isolation
- Supervisor diagnostics
- Independent reset control
- Recovery firmware support

## Open Questions

- Connector family
- Pin count
- Mechanical card dimensions
- Maximum slot count
- Voltage distribution strategy
- Whether CPU slots are electrically identical to accelerator slots
- Fabric topology for Rev C and beyond