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
