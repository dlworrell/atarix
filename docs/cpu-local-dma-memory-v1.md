# CPU-Local DMA Memory Architecture v1

## Purpose

This document defines the role of CPU-local DMA memory on the ATARIX W65C816 CPU card.

CPU-local DMA memory is a staging and isolation layer between the W65C816 local bus and the Fabric Northbridge. It allows the CPU card to exchange data with fabric services without exposing arbitrary CPU-local memory to system-wide DMA.

## Architectural Position

```text
W65C816 CPU
   |
Local CPU Bus
   |
CPU-local SRAM / ROM
   |
CPU-local DMA Buffer Memory
   |
CPU Card DMA Engine / Shim
   |
Fabric Northbridge Mailbox + DMA Interface
   |
System Services
```

The Fabric Northbridge should not DMA directly into arbitrary W65C816 memory.

Instead:

1. The CPU allocates or exposes local DMA buffers.
2. The CPU-card DMA shim owns transfers between those buffers and the Fabric Northbridge.
3. The Fabric Northbridge sees a controlled endpoint rather than the raw W65C816 local bus.
4. Firmware or the kernel copies, maps, or promotes data from local DMA memory into normal execution data structures.

## Design Rule

CPU-local SRAM is for execution.

CPU-local DMA memory is for exchange.

## Responsibilities

CPU-local SRAM is intended for:

- kernel execution
- interrupt handlers
- direct page
- stack
- normal local buffers
- deterministic execution

CPU-local DMA memory is intended for:

- fabric ingress buffers
- fabric egress buffers
- mailbox payload staging
- block I/O staging
- network packet buffers
- framebuffer command staging
- service replies
- diagnostic capture buffers

## Why Local DMA Memory Exists

CPU-local DMA memory provides:

- bounce buffering
- bus-width adaptation
- timing decoupling
- a protection boundary
- deterministic CPU-local SRAM behavior
- a clean interface to the Fabric Northbridge
- a way to avoid forcing the W65C816 into a modern shared-memory bus model

## DMA Ownership

The CPU card owns its local DMA memory.

The Fabric Northbridge may request transfers to or from CPU-local DMA buffers, but should not directly master the W65C816 execution memory space.

The CPU-card DMA shim mediates:

- buffer readiness
- transfer direction
- transfer size
- alignment
- completion status
- error reporting
- interrupt or mailbox completion notification

## Service Directory Exposure

The CPU card should advertise its DMA capability through the Service Directory.

A CPU-card DMA service descriptor should include:

```text
service_class          CPU_LOCAL_DMA
buffer_base            CPU-local address or handle
buffer_size            bytes
alignment              bytes
max_transfer_size      bytes
supported_directions   fabric-to-cpu, cpu-to-fabric, bidirectional
coherency_rules        explicit flush/invalidate unless otherwise stated
completion_model       interrupt, mailbox, polling, or mixed
```

The operating system should bind to this service rather than assuming a fixed memory address.

## Mailbox Integration

Mailbox messages should carry DMA descriptors rather than large payloads whenever practical.

Example DMA request:

```text
DMA_REQUEST
    source_service
    destination_service
    local_buffer_handle
    length
    direction
    flags
    transaction_id
```

Small control messages may remain inline in mailbox payloads, but bulk data should use DMA descriptors.

## Boot Information Block Integration

Firmware should publish CPU-local DMA information in the Boot Information Block.

Draft structure:

```c
struct atarix_dma_local_info {
    uint32_t dma_buffer_base;
    uint32_t dma_buffer_size;
    uint16_t dma_alignment;
    uint16_t dma_flags;
};
```

The kernel must validate this information before using CPU-local DMA buffers.

## Memory Hierarchy

The CPU-local DMA layer extends the ATARIX memory hierarchy:

```text
L0  CPU Registers
L1  Direct Page / Stack
L2  CPU-local SRAM
L3  CPU-local DMA Buffer Memory
L4  Fabric Memory Services
L5  Persistent Storage, Network, and Accelerators
```

## Rev A Policy

For Rev A, CPU-local DMA memory should be simple, explicit, and observable.

Recommended constraints:

- fixed-size DMA buffer region
- firmware-visible descriptor table
- explicit ownership bits
- explicit completion status
- logic-analyzer-friendly control signals where possible
- no implicit coherency
- no direct DMA into arbitrary kernel structures

## Long-Term Direction

Later revisions may support richer DMA windows, scatter/gather lists, or capability-checked buffer handles.

Those features should extend the CPU-local DMA service model rather than bypassing it.
