# ATARIX CPU Card Specification

## Purpose

The CPU card architecture separates processor implementation from platform infrastructure.

This permits experimentation with multiple CPU designs while preserving a stable backplane.

## Initial CPU Card

Processor:

- W65C816S

Responsibilities:

- Execute operating-system code
- Manage local execution state
- Interface with system fabric

## CPU Card Interfaces

### Power

Candidate rails:

- +5V
- +3.3V
- Ground

### Clock

Signals:

- System clock
- Clock enable
- Optional debug clock

### Reset

Signals:

- Global reset
- Local reset
- Recovery reset

### Interrupts

Signals:

- IRQ
- NMI
- Fabric-generated events

### Mailboxes

Mailbox channels support:

- Supervisor communication
- Interprocessor communication
- Service requests

## Local Resources

Potential resources:

- Local SRAM
- Debug registers
- Board identification
- Configuration EEPROM

## Multiprocessor Expansion

Future CPU cards may support:

- Dual-CPU configurations
- Shared-memory experiments
- Message-passing experiments

## Design Objectives

- Simplicity
- Serviceability
- Testability
- Stable platform interface
- Forward compatibility