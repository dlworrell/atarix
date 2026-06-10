# ATARIX Operating System Architecture

## Goals

The operating system should evolve with the hardware rather than being designed in isolation.

The system begins as monitor firmware and grows into a workstation operating system.

## Boot Flow

### Stage 0

- Hardware reset
- Supervisor initialization
- ROM selection

### Stage 1

- ROM monitor startup
- Memory test
- Console initialization

### Stage 2

- Program loader
- Device discovery
- Service initialization

### Stage 3

- Kernel startup
- Driver registration
- Scheduler initialization

## Kernel Services

Candidate services:

- Memory management
- Interrupt management
- Mailboxes
- Device registry
- Timer services

## Capability Broker

Responsibilities:

- Resource grants
- DMA authorization
- Device permissions
- Revocation support

## Driver Model

Drivers should register through a discovery framework.

Each driver should describe:

- Device identity
- Capabilities
- Resource requirements

## Multiprocessor Services

Future support includes:

- Message passing
- Shared-memory coordination
- Synchronization primitives

## Accelerator Runtime

Future support includes:

- Accelerator discovery
- Command queues
- Memory grants
- Completion notification

## Long-Term Goal

A modular workstation operating system capable of supporting secure experimentation across CPUs, FPGA services, and heterogeneous accelerators.