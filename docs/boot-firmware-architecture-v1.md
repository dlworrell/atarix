# Boot Firmware and BIOS Architecture v1

## Purpose

This document defines the firmware, BIOS, and boot architecture for ATARIX systems.

The goals are:

- Deterministic system bring-up
- Hardware discovery
- Service discovery
- Diagnostic visibility
- Reliable kernel loading
- Independence from filesystems during early development

The firmware architecture is designed around the ATARIX Fabric Northbridge model.

## Architectural Overview

```text
W65C816 CPU Card
        ↓
Boot ROM
        ↓
Firmware / BIOS
        ↓
Fabric Northbridge Discovery
        ↓
Service Directory Construction
        ↓
Boot Service Selection
        ↓
Bootloader
        ↓
Operating System
```

The firmware is responsible for discovering hardware and constructing the initial service directory used by the operating system.

## Boot Sequence

```text
RESET
  -> ROM reset vector
  -> CPU initialization
  -> Native-mode transition
  -> Local SRAM initialization
  -> Fabric Northbridge initialization
  -> Discovery ROM enumeration
  -> Service directory construction
  -> BIOS service initialization
  -> ROM monitor availability
  -> Boot service selection
  -> Kernel loading
  -> Kernel handoff
```

## CPU Initialization

The W65C816 starts in emulation mode after reset.

Firmware shall:

1. Disable interrupts.
2. Clear decimal mode.
3. Enter native mode.
4. Configure stack pointer.
5. Configure direct page.
6. Initialize interrupt vectors.
7. Initialize console services.
8. Initialize Fabric Northbridge access.

## BIOS Responsibilities

- Hardware initialization
- CPU-local SRAM initialization
- Fabric Northbridge initialization
- Discovery ROM enumeration
- Service directory construction
- Console services
- Block device services
- Interrupt management
- Diagnostic monitor support
- Kernel loading support

## Fabric Northbridge Integration

The Fabric Northbridge is the architectural center of the system.

Firmware communicates with discovery services, service directory services, memory services, mailbox services, DMA services, and boot services.

All hardware is discovered dynamically.

## Discovery Phase

Each expansion card publishes a Discovery ROM.

```text
Enumerate Card
    -> Validate Discovery ROM
    -> Enumerate Services
    -> Record Resources
    -> Register Service Endpoints
```

The result is a runtime Service Directory.

## Service Directory

Example service classes:

- Console Service
- DMA Service
- Storage Service
- Network Service
- Framebuffer Service
- Memory Service
- Supervisor Service

Operating systems bind to services rather than physical slots.

## ROM Monitor

Recommended commands:

```text
M addr len       memory dump
E addr value     memory edit
S                list services
D                list devices
B                boot
R                reset
```

## Boot Services

Examples:

- Local Storage Boot Service
- Network Boot Service
- Recovery Boot Service

## Kernel Image Header

```c
struct atarix_kernel_header {
    char magic[8];
    uint16_t header_size;
    uint16_t flags;
    uint32_t load_addr;
    uint32_t entry_addr;
    uint32_t image_size;
    uint32_t checksum;
};
```

## Kernel Handoff

Before transferring control:

- Stack shall be valid.
- Direct page shall be valid.
- Service Directory shall be available.
- Fabric Northbridge shall be initialized.
- Boot Information Block shall be available.

## Boot Information Block

Provides:

- Firmware Version
- Service Directory Pointer
- Discovery Information
- Memory Service Information
- Boot Device Information
- Diagnostic Information

## Design Principles

1. Keep firmware simple.
2. Enumerate hardware before boot.
3. Discover services before loading the kernel.
4. Avoid filesystem dependencies during early bring-up.
5. Prefer raw block loading initially.
6. Keep policy in the operating system.
7. Keep mechanism in firmware.
8. Treat the Fabric Northbridge as the system integration point.
9. Make every stage observable and debuggable.
10. Support future heterogeneous processor cards.
