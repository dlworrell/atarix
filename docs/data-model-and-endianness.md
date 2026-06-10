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