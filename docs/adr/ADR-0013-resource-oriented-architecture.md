# ADR-0013: Resource-Oriented Architecture

Status: Accepted

Date: 2026-06-10

## Context

Early revisions of ATARIX are based on a W65C816 CPU card, but the long-term architecture includes memory services, storage services, accelerators, multiple processor types, DMA services, and capability-mediated access.

A fixed-address model centered on a single CPU address space does not scale well to those goals.

## Decision

The primary ATARIX abstraction is a resource.

Resources are represented through identifiers, offsets, windows, services, and capabilities.

CPU-local addresses remain valid within a CPU card but are not the universal architectural abstraction.

## Resource Examples

- Memory objects.
- Storage objects.
- Mailbox endpoints.
- DMA channels.
- Interrupt sources.
- Device windows.
- Accelerator queues.
- Discovery records.
- Supervisor services.

## Evidence

The architecture already uses:

- u64 resource identifiers.
- u64 object identifiers.
- Capability-mediated access.
- Window mapping.
- Fabric descriptors.

## Alternatives Considered

- Flat memory-map-centric architecture.
- CPU-address-centric architecture.
- Per-device ad hoc addressing schemes.

## Consequences

Pros:

- Easier support for future CPU cards.
- Easier support for large memory systems.
- Cleaner DMA architecture.
- Better compatibility with capability-based security.
- Supports distributed and heterogeneous resources.

Cons:

- Additional indirection.
- More metadata and discovery infrastructure.

## Design Rule

Resources, objects, services, and capabilities are primary.

Fixed CPU memory addresses are implementation details of a particular processor card.