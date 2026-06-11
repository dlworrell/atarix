# Fabric Service Model v1

## Purpose

This document defines the service-oriented architecture used by ATARIX.

The goal is to decouple operating-system interfaces from physical hardware implementation details.

Applications and operating-system drivers interact with services.

Services may be implemented by:

- Physical cards
- FPGA fabric logic
- Supervisor firmware
- Future processor cards

## Core Principle

```text
Driver
   -> Service

Service
   -> Fabric Endpoint

Fabric Endpoint
   -> Hardware Implementation
```

The operating system should not assume a specific slot number or physical implementation.

## Service Classes

### Core Services

- Console
- Timer
- RTC
- Interrupt Controller
- DMA Engine

### Memory Services

- RAM
- Persistent Memory
- Cache Services

### Storage Services

- Block Storage
- Boot Services
- Filesystem Services

### Network Services

- Ethernet
- Time Services
- Network Boot Services

### Graphics Services

- Framebuffer
- 2D Acceleration
- 3D Acceleration

### Compute Services

- FPGA Compute
- DSP Services
- AI Acceleration

### Management Services

- Supervisor
- Health Monitoring
- Power Control
- Fault Logging

## Service Descriptor

```c
struct atarix_service_descriptor {
    uint16_t service_class;
    uint16_t service_type;

    uint16_t version_major;
    uint16_t version_minor;

    uint32_t endpoint_id;

    uint32_t capabilities;
    uint32_t resource_handle;

    uint32_t mailbox_rx;
    uint32_t mailbox_tx;
};
```

## Discovery Relationship

Discovery ROMs publish services.

```text
Discovery ROM
    -> Service Descriptors

Firmware
    -> Service Directory

Operating System
    -> Service Binding
```

## Service Directory

Firmware builds a service directory during initialization.

The operating system may query:

```text
Find Storage Services
Find Network Services
Find Display Services
Find DMA Services
```

rather than searching for specific slots.

## Mailbox Communication

Services communicate through mailbox interfaces.

```text
Request
    -> Mailbox

Fabric Routing

Response
    -> Mailbox
```

Mailbox protocols are defined separately.

## DMA-Oriented Operation

Services should prefer DMA-based transfers.

The CPU should coordinate transfers rather than moving large volumes of data directly.

## Multi-Service Devices

A single physical card may expose multiple services.

Example:

```text
Storage Card

    Storage Service
    DMA Service
    Compression Service
```

The operating system binds to services individually.

## Fabric-Native Services

Some services may exist entirely inside the FPGA fabric.

Example:

```text
DMA Engine
Interrupt Router
Mailbox Router
Memory Service
```

These services need not correspond to physical cards.

## Future Multi-CPU Support

The service model supports future heterogeneous processor cards.

Examples:

```text
W65C816 CPU Card
68K CPU Card
RISC-V CPU Card
```

All processors communicate with services through the same architectural model.

## Design Goals

1. Hardware independence.
2. Service-oriented discovery.
3. DMA-first operation.
4. Fabric-centric communication.
5. Future multi-processor support.
6. Capability-mediated access control.
7. Clean separation of implementation and interface.
