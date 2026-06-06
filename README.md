# ATARIX

ATARIX is a modular cyberdeck-oriented experimental workstation platform built around the W65C816, a staged FPGA system fabric, a supervisor microcontroller, and a secure heterogeneous accelerator model.

## Vision

ATARIX combines ideas from:

- W65C816 homebrew computing
- Vega816 SMP experimentation
- Apple NuBus-style device discovery
- Sun UPA-style heterogeneous compute fabrics
- Modern capability-based security models
- FPGA-mediated resource management

The objective is to create a long-lived platform for:

- Operating system research
- Hardware experimentation
- Secure computing architectures
- FPGA development
- Retro-modern workstation design

## Development Roadmap

### Rev A
- Single W65C816
- ROM
- SRAM
- UART
- GPIO
- Discrete address decoding
- Debug clock and monitor

### Rev B
- Modular CPU cards
- Active backplane preparation
- Shared services
- W5500 networking
- Golden ROM and recovery

### Rev C
- FPGA as programmable chipset
- Address decode
- Interrupt routing
- Mailboxes
- DMA foundations

### Rev D
- Dual-CPU operation
- MMU concepts
- DMA mediation
- Ring security architecture

### Rev E
- Accelerator framework
- ARM compute modules
- FPGA accelerators
- Capability broker

## Core Principles

1. Modular design
2. Observable and debuggable hardware
3. Incremental development
4. Capability-based expansion
5. Devices are not trusted by default
6. Long service life

## Influences

- W65C816
- Vega816
- BB816
- Apple NuBus
- Sun UPA
- Lattice ECP5

## Status

Architectural design and planning phase.