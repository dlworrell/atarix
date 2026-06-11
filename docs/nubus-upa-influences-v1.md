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
