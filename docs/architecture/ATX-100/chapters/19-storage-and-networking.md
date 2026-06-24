# Chapter 19: Storage and Networking

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/cpu-local-dma-memory-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/cpu-local-dma-memory-v1.md -->
### Integrated source: `docs/cpu-local-dma-memory-v1.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/data-model-and-endianness.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/data-model-and-endianness.md -->
### Integrated source: `docs/data-model-and-endianness.md`

# ATARIX Data Model and Endianness

## Status

Draft architecture specification.

This document defines the binary data model for ATARIX architecture-defined structures, including byte order, integer widths, address widths, length fields, identifiers, timestamps, and future 32-bit / 64-bit expansion rules.

## Purpose

ATARIX begins with a W65C816 CPU, but the system architecture is larger than the CPU's native execution width.

The W65C816 is an 8/16-bit processor with a 24-bit physical address space. ATARIX also includes an FPGA fabric, mailbox protocols, DMA descriptors, discovery records, supervisor logs, memory-service objects, network boot images, and future 32-bit / 64-bit processors or accelerators.

Those structures must not be limited to a 16-bit or 24-bit worldview.

## Design Rule

CPU width is not system width.

CPU-local address width is not fabric address width.

The W65C816 may execute 8-bit and 16-bit code and use 24-bit native addresses, but ATARIX architecture-defined binary formats may use 8-bit, 16-bit, 24-bit, 32-bit, and 64-bit fields where appropriate.

## Byte Order

ATARIX architecture-defined binary structures use little-endian byte order.

This applies to:

- Discovery records.
- Mailbox headers.
- DMA descriptors.
- Capability records.
- Boot image headers.
- Fault logs.
- Supervisor records.
- Memory-service objects.
- Fabric registers wider than 8 bits.
- Future accelerator command blocks.

## Integer Widths

Standard integer widths:

```text
u8   Unsigned 8-bit integer
u16  Unsigned 16-bit integer, little-endian
u24  Unsigned 24-bit integer, little-endian
u32  Unsigned 32-bit integer, little-endian
u64  Unsigned 64-bit integer, little-endian
```

Signed integers should be avoided in hardware-facing protocols unless explicitly required.

## Address Classes

ATARIX distinguishes several address classes.

```text
W65C816 native address      u24
Future 32-bit CPU address   u32
Future 64-bit CPU address   u64
Fabric register offset      u32
Fabric resource address     u64
Memory object identifier    u64
Memory object offset        u64
Persistent storage offset   u64
```

The existence of u24 CPU-local addresses must not constrain the ATARIX fabric, memory-service architecture, or future processor cards.

## 24-Bit CPU-Local Addresses

The W65C816 directly uses 24-bit physical addresses.

Native W65C816-visible addresses should use:

```text
u24 cpu_address
```

Examples:

- CPU local memory windows.
- Rev A memory map addresses.
- CPU-visible I/O apertures.
- Short descriptors targeting native W65C816 CPU space.

A u24 address is encoded least-significant byte first.

A u24 field is not a universal ATARIX address.

## Fabric Addresses and Resource References

ATARIX fabric resources should not be limited to u24 addressing.

For fabric-level addressing, prefer:

```text
Address Space ID: u32 or u64
Resource ID:      u64
Offset:           u64
Length:           u32 or u64, depending on resource class
```

This allows a W65C816 CPU card, future 32-bit CPU card, future 64-bit CPU card, memory service, storage service, and accelerator service to participate in the same system without forcing all participants through the W65C816 native address model.

## 32-Bit Fields

Use u32 for fields that may exceed 64 KiB but do not need server-scale addressing.

Recommended u32 uses:

- DMA transfer length in v1 descriptors.
- Mailbox payload length for extended messages.
- Image length in boot headers.
- Checksums.
- Sequence numbers.
- Performance counters that may wrap acceptably.
- Fabric register offsets.
- Window sizes up to 4 GiB.
- Future 32-bit CPU local addresses.

A u32 length has a maximum representable value of 4,294,967,295 bytes.

That is adequate for many Rev A through Rev C structures, but it is not enough for future 16 GiB, 512 GiB, or larger memory-service objects.

## 64-Bit Fields

Use u64 for fields that must survive future large-memory and long-running-system requirements.

Recommended u64 uses:

- Capability identifiers.
- Memory object identifiers.
- Large memory object sizes.
- 64-bit timestamps.
- Fault-log monotonic counters.
- Persistent object IDs.
- Large byte offsets.
- Long-running performance counters.
- Future accelerator job IDs.
- Future 64-bit CPU local addresses.
- Fabric resource addresses.

A u64 length has a maximum representable value of 18,446,744,073,709,551,615 bytes.

This is sufficient for the long-term ATARIX goal of memory services that may eventually expose 16 GiB, 512 GiB, or larger memory resources.

## 32-Bit vs 64-Bit Length Policy

ATARIX should not use one length size everywhere.

Different structures should choose lengths based on expected scale and parsing cost.

### Use u16 length fields for small local records

Examples:

```text
Small TLV records
Short names
Small local descriptors
Compact firmware tables
```

### Use u32 length fields for transfer and image sizes

Examples:

```text
DMA transfer length
Boot image length
Mailbox extended payload length
Fabric buffer size
```

### Use u64 length fields for persistent or large-memory resources

Examples:

```text
Memory service object size
File-like object size
Large accelerator buffer size
Large byte offset
Persistent storage extent size
```

## Extended Length Encoding

Where a v1 format begins with a u32 length but may later need larger sizes, the format should reserve an escape value.

Recommended rule:

```text
u32 length != $FFFFFFFF
    length is encoded directly as u32

u32 length == $FFFFFFFF
    actual length follows as u64
```

This allows simple Rev A parsers to handle normal-sized objects while preserving an upgrade path for large objects.

Formats that are expected to represent large memory or storage resources from the beginning should use u64 directly and should not use the escape form.

## Alignment

Architecture-defined structures should prefer natural alignment where practical:

```text
u16 fields aligned to 2-byte boundaries
u32 fields aligned to 4-byte boundaries when practical
u64 fields aligned to 8-byte boundaries when practical
```

However, W65C816 firmware must not assume that external binary structures are naturally aligned unless the structure specification explicitly guarantees it.

## Reserved Fields

All reserved fields must be written as zero and ignored when read unless a later specification defines them.

This rule preserves forward compatibility.

## Versioning

Binary formats must include either:

- An explicit version field, or
- A parent structure that defines the version.

Major version changes may break compatibility.

Minor version changes should be backward-compatible.

Unknown optional fields should be ignored.

Unknown mandatory fields should cause parsing failure.

## Register Widths

Fabric registers may expose 8-bit, 16-bit, 32-bit, or 64-bit values.

If a register wider than the CPU's convenient access width is exposed, the register block must define:

- Access order.
- Snapshot behavior.
- Whether reads are latched.
- Whether writes commit on final byte.
- Whether partial writes are legal.

This is especially important for:

- 64-bit counters.
- Timestamps.
- Large address registers.
- Capability IDs.

## Timestamps

Architecture-level timestamps should use u64.

Preferred timestamp model:

```text
u64 seconds since epoch
```

or, where higher precision is required:

```text
u64 monotonic tick counter
```

The exact timestamp encoding must be specified by the owning subsystem.

The RTC/NTP system should not rely on 32-bit time fields for long-lived operation.

## Capability IDs

Capability identifiers should use u64.

Rationale:

- Avoids short ID exhaustion.
- Supports persistent logging.
- Supports revocation tables.
- Supports future multi-card and multi-service deployments.

## Memory Service Objects

Memory service objects should use u64 identifiers and u64 sizes.

The W65C816 CPU may map these objects through 16 KiB, 32 KiB, or 64 KiB windows, but the object itself may be much larger than the CPU address space.

A future 32-bit or 64-bit CPU card may map larger portions of the same object through a different address aperture without changing the object model.

Example:

```text
Memory Object ID: u64
Object Size:      u64
Window Offset:    u64
Window Length:    u32 or u16, depending on aperture
```

## Design Principle

Use small fields where the domain is naturally small.

Use large fields where future capacity, persistence, timestamps, or external resources are involved.

Do not force all structures into 16-bit fields merely because the first CPU is a W65C816.

Do not treat u24 as the universal ATARIX address width merely because the first CPU is a W65C816.

Do not force all structures into 64-bit fields when a small local structure would be simpler and more efficient.

## Related Documents

```text
docs/discovery-rom-format.md
docs/mailbox-protocol-v1.md
docs/dma-engine-v1.md
docs/capability-model.md
docs/netboot-ntp-v1.md
docs/supervisor-card-v1.md
docs/address-space-architecture.md
```

## Open Questions

- Final timestamp epoch and precision.
- Whether some fabric counters should be 48-bit instead of 64-bit.
- Whether boot images should use u32 length plus u64 extended length or direct u64 length.
- Whether memory service windows should prefer fixed 64 KiB mappings.
- Exact ABI naming convention for signed types, if needed.
- Whether future distributed services require 128-bit object identifiers.
