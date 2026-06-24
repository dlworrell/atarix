# Chapter 7: Fabric-Centric Computing

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0001-fabric-centric-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0001-fabric-centric-architecture.md -->
### Integrated source: `docs/adr/ADR-0001-fabric-centric-architecture.md`

# ADR-0001 Fabric-Centric Architecture

Status: Accepted

Date: 2026-06-11

## Context

The project must support future processor cards, capability-mediated services, and a workstation-class expansion model.

## Decision

The W65C816 local bus remains local to the CPU card.

All system-wide communication occurs through the Fabric Northbridge.

## Consequences

Benefits:

- Heterogeneous processor support
- Stable service model
- Improved observability
- Easier DMA integration

Costs:

- Additional latency
- More FPGA complexity
- More firmware infrastructure

## Future Revisions

Refine mailbox, DMA, and service contracts.

<!-- AEMS-MIGRATED-SOURCE: docs/fabric-controller-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/fabric-controller-architecture-v1.md -->
### Integrated source: `docs/fabric-controller-architecture-v1.md`

# Fabric Controller Architecture v1

## Purpose

The Fabric Controller is the architectural center of ATARIX.

It provides transport, routing, discovery support, DMA services, interrupt routing, mailbox transport, memory services, and future multi-processor coordination.

The Fabric Controller is implemented initially using the ULX3S ECP5 FPGA platform.

## Design Philosophy

ATARIX is not a shared-bus computer.

The W65C816 local bus remains local to the CPU card.

System-wide communication occurs through the FPGA fabric.

```text
CPU Card
    <-> Fabric Bridge
    <-> Fabric Controller
    <-> Services
```

## Architectural Responsibilities

The Fabric Controller shall provide:

- Endpoint routing
- Service routing
- Mailbox transport
- DMA services
- Interrupt routing
- Memory services
- Resource discovery support
- Multi-processor coordination
- Diagnostic instrumentation

## Architectural Planes

### Management Plane

Implemented by the RP2350 supervisor.

Functions:

- Reset
- Watchdog
- RTC
- Health monitoring
- Recovery
- Fault logging

### Discovery Plane

Implemented through Discovery ROM structures.

Functions:

- Device discovery
- Service publication
- Resource publication
- Capability publication

### Data Plane

Implemented by the Fabric Controller.

Functions:

- DMA
- Mailboxes
- Memory access
- Service communication
- Accelerator access

## Endpoint Model

Every service exposed to the system receives an endpoint identifier.

```text
Endpoint ID
    -> Mailbox
    -> Capabilities
    -> Resources
```

Endpoints may be backed by:

- Physical cards
- FPGA logic
- Supervisor services
- Future CPU cards

## Mailbox Routing

The Fabric Controller routes mailbox traffic.

```text
Request
    -> Fabric Router
    -> Destination Endpoint

Response
    -> Fabric Router
    -> Source Endpoint
```

Mailbox protocol details are defined separately.

## DMA Engine

The Fabric Controller owns DMA execution.

Responsibilities:

- Memory transfers
- Storage transfers
- Network transfers
- Accelerator transfers
- Scatter/gather support

The DMA engine should reduce CPU involvement in large data movement.

## Interrupt Router

Interrupt sources are abstracted from physical hardware.

```text
Interrupt Source
    -> Fabric Router
    -> CPU Target
```

This enables future multi-processor systems.

## Memory Services

The Fabric Controller provides memory service abstractions.

Memory resources may exist:

- On CPU cards
- On memory cards
- Within FPGA-attached resources

The service model allows future expansion without exposing implementation details.

## Service Directory

The Fabric Controller maintains a runtime directory of services.

Example:

```text
Storage Service
DMA Service
Network Service
Framebuffer Service
Memory Service
```

Operating systems query the directory rather than assuming slot locations.

## Multi-Service Devices

One physical device may expose multiple services.

Example:

```text
Storage Card

    Storage Service
    DMA Service
    Compression Service
```

Each service receives a unique endpoint.

## Multi-Processor Direction

Future processors may include:

- W65C816
- 68000-family
- RISC-V

The Fabric Controller serves as the common communication substrate.

## Diagnostic Instrumentation

The Fabric Controller should expose:

- Traffic counters
- DMA statistics
- Mailbox statistics
- Interrupt statistics
- Error counters
- Trace facilities

System observability is a primary design goal.

## Design Goals

1. Fabric-centric architecture.
2. Service-oriented communication.
3. DMA-first transport.
4. Slot-independent discovery.
5. Capability-mediated access.
6. Hardware abstraction through services.
7. Future heterogeneous CPU support.
8. Extensive observability and diagnostics.

<!-- AEMS-MIGRATED-SOURCE: docs/fabric-service-model-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/fabric-service-model-v1.md -->
### Integrated source: `docs/fabric-service-model-v1.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/nubus-upa-influences-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/nubus-upa-influences-v1.md -->
### Integrated source: `docs/nubus-upa-influences-v1.md`

# NuBus and UPA Architectural Influences

## Purpose

This document explains how ATARIX incorporates concepts inspired by Apple NuBus systems and Sun Ultra Port Architecture (UPA) systems while remaining a distinct architecture.

ATARIX is not electrically compatible with either NuBus or UPA.

The project borrows architectural ideas rather than bus implementations.

## Design Summary

```text
NuBus
    -> Discovery

UPA
    -> Transport

Supervisor Plane
    -> Management

ATARIX
    -> Integrated Architecture
```

## NuBus Influence

NuBus contributed the concept of self-describing hardware.

Key concepts adopted by ATARIX:

- Card self-identification
- Device discovery
- Resource publication
- Slot-independent configuration
- Operating-system enumeration

### NuBus Mapping

| NuBus | ATARIX |
|---------|---------|
| Declaration ROM | Discovery ROM |
| Slot Manager | Firmware Discovery Service |
| Board ID | Vendor ID + Device ID |
| Resource Directory | Resource Descriptors |
| Slot Enumeration | Fabric Enumeration |

## UPA Influence

UPA contributed the concept of high-performance transport separated from configuration and discovery.

Key concepts adopted by ATARIX:

- Service-oriented communication
- High-bandwidth transport paths
- DMA-centric operation
- Accelerator-oriented architecture
- Separation of control and data movement

### UPA Mapping

| UPA | ATARIX |
|------|---------|
| UPA Port | Fabric Endpoint |
| Graphics Accelerator | Fabric Service |
| Memory Service | Memory Service |
| DMA Transport | DMA Engine |
| High-Speed Interconnect | ECP5 Fabric |

## Three-Plane Architecture

ATARIX intentionally separates management, discovery, and transport.

### Management Plane

Implemented by the RP2350 supervisor.

Responsibilities:

- Power control
- Reset control
- RTC
- Watchdog
- Health monitoring
- Fault logging
- Recovery

### Discovery Plane

Implemented through Discovery ROM structures and firmware enumeration.

Responsibilities:

- Device identification
- Resource publication
- Capability publication
- Boot-service discovery

### Data Plane

Implemented by the FPGA fabric.

Responsibilities:

- DMA
- Memory services
- Storage services
- Networking
- Video
- Accelerators
- Mailboxes

## Service-Oriented Design

Traditional retrocomputers often expose physical devices directly.

ATARIX instead prefers service exposure.

Example:

```text
Storage Card
    -> Storage Service
    -> DMA Service
    -> Compression Service
```

The operating system interacts with services rather than assuming fixed hardware layouts.

## Discovery ROM Philosophy

The Discovery ROM is intended to combine concepts from:

```text
NuBus Declaration ROM
+
Open Firmware Device Tree
+
PCI Capability Lists
+
Fabric Resource Descriptors
=
ATARIX Discovery ROM
```

## Fabric-Centric Architecture

Traditional systems:

```text
CPU
  <-> Shared Bus
  <-> Devices
```

ATARIX:

```text
CPU Card
    <-> Fabric Bridge
    <-> ECP5 Fabric
    <-> Services
```

The fabric becomes the primary transport mechanism.

## Architectural Goals

1. Self-describing hardware.
2. Slot-independent configuration.
3. Service-oriented communication.
4. DMA-first design.
5. Capability-mediated access.
6. Separation of management, discovery, and transport.
7. Support for heterogeneous future CPU cards.
8. Support for FPGA-defined services and accelerators.

## Historical Lineage

```text
Apple II
    -> Physical expansion slots

NuBus
    -> Self-describing hardware

Sun UPA
    -> Service-oriented high-performance transport

Modern Service Processors
    -> Independent management plane

ATARIX
    -> Combination of all four approaches
```
