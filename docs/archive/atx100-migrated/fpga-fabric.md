# ATARIX FPGA Fabric Architecture

## Purpose

The FPGA fabric serves as the evolutionary bridge between a simple W65C816 computer and a modular workstation architecture.

The FPGA begins as programmable glue logic and gradually becomes a mediated system fabric.

## Initial Platform

Development target:

- Lattice ECP5
- ULX3S development board

Reasons:

- Open-source tooling
- Strong community support
- Adequate logic resources
- Long-term availability

## Rev C Responsibilities

### Address Decode

The FPGA replaces portions of discrete decode logic.

Responsibilities:

- Chip selects
- Address windows
- Device routing

### Interrupt Router

The FPGA centralizes interrupt management.

Capabilities:

- Interrupt prioritization
- Routing tables
- Masking
- Status reporting

### Mailboxes

Mailbox services support:

- CPU-to-device messaging
- CPU-to-CPU messaging
- Supervisor communication

### Timers

Fabric timers provide:

- System tick generation
- Benchmark timing
- Watchdog support

## DMA Engine

Future DMA support includes:

- Transfer descriptors
- Completion notification
- Access validation
- Fault reporting

DMA operations should eventually be capability mediated.

## Discovery Services

Each attached module should expose:

- Vendor ID
- Device ID
- Version information
- Capability records

The design is influenced by NuBus-style discovery concepts.

## Fabric Registers

Candidate blocks:

- System control
- Interrupt controller
- Mailboxes
- DMA control
- Discovery services
- Performance counters

## Future Responsibilities

- Bus arbitration
- Device isolation
- Capability enforcement assistance
- Accelerator attachment
- Fabric-level diagnostics

## Design Rule

The FPGA must remain documented.

Every register, mailbox, interrupt route, and DMA mechanism should be described by a stable specification.