# Discovery ROM and Slot Architecture v1

## Purpose

This document expands the card-discovery model used by ATARIX and records the current architectural direction discussed during design reviews.

The goal is that software binds to capabilities and services rather than physical slot numbers.

## Design Inspiration

Architectural influences include:

- Apple II slot discovery concepts
- NuBus declaration ROMs
- Open Firmware device descriptions
- PCI capability discovery
- Modern service-oriented fabrics

## Core Rule

Applications and operating systems should not depend on:

- Slot numbers
- Backplane position
- Fixed interrupt assignments
- Fixed memory windows

Instead they should bind to discovered services.

## Discovery ROM

Every intelligent card should expose a Discovery ROM.

Conceptually:

```text
Card Inserted
    -> Discovery ROM Read
    -> Capability Enumeration
    -> Resource Enumeration
    -> Service Registration
    -> Runtime Binding
```

## Discovery Information

Recommended fields:

- Vendor identifier
- Device identifier
- Card class
- Card revision
- Firmware revision
- Capability list
- Resource descriptors
- Service endpoints
- DMA requirements
- Interrupt requirements
- Diagnostic information

## Example Classes

Examples include:

- CPU card
- Memory card
- Storage card
- Network card
- Framebuffer card
- Accelerator card
- Supervisor card
- Development card

## Service Registration

A discovered card registers services with the Fabric Northbridge.

Example:

```text
Network Card
    -> Network Service
    -> DMA Capability
    -> Interrupt Endpoint
```

The operating system consumes the resulting service directory.

## Resource Descriptors

Resources should be described rather than hard-coded.

Examples:

- Mailbox endpoints
- DMA channels
- Interrupt vectors
- Memory windows
- MMIO regions
- Shared buffers
- Boot services

## Boot-Time Enumeration

Firmware should:

1. Enumerate every card.
2. Validate Discovery ROM structures.
3. Register capabilities.
4. Build the service directory.
5. Publish a Boot Information Block.
6. Transfer control to the loader or kernel.

## Hot-Plug Considerations

Hot-plug is not required for Rev A.

However, the discovery model should not prevent future support for:

- card insertion
- card removal
- service re-registration
- capability updates

## Supervisor Integration

The supervisor processor maintains an independent view of hardware health.

Examples:

- slot power status
- thermal alarms
- watchdog events
- fault history
- recovery operations

The supervisor may assist discovery and diagnostics but should not become the sole source of hardware truth.

## Long-Term Goal

The long-term goal is a self-describing workstation fabric in which hardware announces capabilities, firmware constructs a service directory, and operating systems bind to services rather than buses, addresses, or slots.
