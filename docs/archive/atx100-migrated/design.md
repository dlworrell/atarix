# ATARIX Architecture Design Document

## Purpose

ATARIX is a modular cyberdeck-oriented experimental workstation platform built around the W65C816, a staged FPGA system fabric, a supervisor microcontroller, and a secure heterogeneous accelerator model.

The project is intended to support long-lived experimentation in operating systems, FPGA-based system architecture, capability-oriented security, heterogeneous computing, and retro-modern workstation design.

## Design Philosophy

The system is intentionally staged. Each revision must produce a useful, testable, and debuggable computer rather than a dependency on future hardware.

Guiding principles:

1. Observable hardware.
2. Incremental complexity.
3. Recovery before optimization.
4. Capability-based expansion.
5. Explicit trust boundaries.
6. Long service life.

## System Architecture

The long-term architecture consists of:

- W65C816 CPU subsystem
- Supervisor microcontroller
- FPGA-mediated system fabric
- Active backplane
- Device discovery framework
- Capability broker
- Accelerator framework

The architecture evolves from a simple single-board computer into a modular workstation platform.

## Revision Roadmap

### Rev A

Single W65C816 bring-up platform.

Features:

- W65C816
- ROM
- SRAM
- UART
- GPIO
- Discrete address decode
- Monitor firmware

### Rev B

Modularization phase.

Features:

- CPU card preparation
- Shared services
- W5500 networking
- Golden ROM recovery
- Active backplane planning

### Rev C

FPGA chipset phase.

Features:

- Address decoding
- Interrupt routing
- Mailboxes
- DMA foundations
- Device windows

### Rev D

Protected multiprocessing phase.

Features:

- Dual CPU operation
- MMU concepts
- DMA mediation
- Security rings
- Fault containment

### Rev E

Accelerator phase.

Features:

- ARM compute modules
- FPGA accelerators
- Capability broker
- Secure command queues
- Mediated DMA

## Security Model

Devices are not trusted by default.

Capabilities describe:

- Resource ownership
- Allowed operations
- Address windows
- DMA permissions
- Revocation rules

The security architecture becomes increasingly important as multiprocessing and accelerators are introduced.

## Supervisor Controller

Responsibilities include:

- Reset sequencing
- Power monitoring
- Clock management
- Recovery functions
- FPGA configuration support
- Fault logging
- Board identity management

## FPGA Fabric

Early FPGA responsibilities:

- Address decode
- Interrupt routing
- Mailboxes
- Timers

Later responsibilities:

- DMA mediation
- Bus arbitration
- Device discovery
- Accelerator attachment
- Capability enforcement assistance

## Software Implications

Early software:

- ROM monitor
- Memory test
- Serial loader
- Device diagnostics

Later software:

- Capability broker
- Device registry
- DMA manager
- Multiprocessor services
- Accelerator runtime

## Current Status

Architectural design and planning phase.