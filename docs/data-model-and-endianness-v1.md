# Data Model and Endianness v1

## Purpose

This document defines the canonical ATARIX data model used by firmware, Discovery ROMs, Service Directory records, mailbox packets, DMA descriptors, Boot Information Blocks, Fabric CSRs, and future processor cards.

This document exists to prevent accidental coupling between the W65C816 CPU-local view of the machine and the wider Fabric Northbridge view of the system.

## Core Rules

```text
All ATARIX-defined binary structures are little-endian.

CPU Width != Fabric Width.
Address != Handle.
CPU-local SRAM is for execution.
CPU-local DMA memory is for exchange.
```

These rules apply unless a later specification explicitly states otherwise.

## Endianness

ATARIX structures are little-endian.

This includes:

- Discovery ROM records
- Identity EEPROM records
- capability records
- Service Directory records
- Boot Information Blocks
- mailbox headers
- mailbox payload descriptors
- DMA descriptors
- Fabric CSR fields
- kernel image headers
- monitor records
- provisioning records

The W65C816 is little-endian, and keeping the full architecture little-endian avoids unnecessary conversion in firmware and early bring-up code.

Future processor cards that use a different native byte order must adapt at the CPU-card boundary and present ATARIX-defined structures in little-endian form.

## CPU-Local Address Model

The W65C816 CPU-local address model is 24-bit.

Use type name:

```text
atarix_u24_cpu_addr
```

A CPU-local address refers only to memory visible in the local W65C816 CPU-card address space.

CPU-local addresses must not be used as Fabric addresses, service handles, DMA handles, or global resource identifiers.

## Fabric Address Model

Fabric addresses are 64-bit.

Use type name:

```text
atarix_u64_fabric_addr
```

A Fabric address refers to a fabric-visible memory or resource address, not a W65C816 CPU-local address.

The W65C816 does not directly expose or directly address the complete fabric address space.

## Resource Handles

Resource handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_resource_handle
```

A resource handle is not an address.

Handles may identify:

- services
- devices
- DMA buffers
- mailboxes
- capabilities
- boot resources
- provisioning records
- diagnostic streams

Software must not infer physical address layout from a handle value.

## Service Handles

Service handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_service_handle
```

The Service Directory maps discovered services to service handles. Operating systems and firmware components should bind to service handles rather than physical slots or fixed addresses.

## DMA Handles

DMA handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_dma_handle
```

A DMA handle represents an authorized DMA buffer, window, or transaction context.

DMA handles should be capability-checked and should not be interpreted as raw addresses.

## CPU-Local DMA Boundary

CPU-local DMA memory is the translation boundary between the W65C816 CPU-local world and the Fabric Northbridge world.

```text
W65C816 CPU-local address
    -> CPU-local DMA buffer
    -> CPU-card DMA shim
    -> Fabric address / service handle / DMA handle
```

The Fabric Northbridge should not DMA directly into arbitrary CPU-local SRAM.

## Alignment

Fabric-visible structures should be naturally aligned for 64-bit transport unless a compact ROM format explicitly requires otherwise.

Recommended defaults:

```text
16-bit fields  aligned to 2 bytes
32-bit fields  aligned to 4 bytes
64-bit fields  aligned to 8 bytes
records        aligned to 8 bytes where practical
```

Discovery ROM and Identity EEPROM formats may use compact layouts, but must define alignment explicitly.

## Integer Type Names

Preferred architectural type names:

```text
atarix_u8
atarix_u16
atarix_u24_cpu_addr
atarix_u32
atarix_u64
atarix_u64_fabric_addr
atarix_u64_resource_handle
atarix_u64_service_handle
atarix_u64_dma_handle
```

These names should be reflected in future C headers under `include/atarix/`.

## Binary Format Policy

New binary specifications should explicitly state:

- endianness
- field width
- alignment
- record version
- checksum or integrity mechanism
- whether a value is an address or a handle
- whether an address is CPU-local or fabric-visible

No binary format should use an unqualified word such as `address` when `cpu_local_address`, `fabric_address`, or `handle` is meant.

## Future Processor Cards

ATARIX must support future 32-bit and 64-bit processor cards without changing resource interfaces.

Therefore:

- service handles remain 64-bit
- DMA handles remain 64-bit
- fabric addresses remain 64-bit
- binary structures remain little-endian
- CPU-specific address sizes remain local to each CPU card

## Design Consequence

The W65C816 is a first-class ATARIX CPU node, but it does not define the width of the whole machine.

The Fabric Northbridge defines the system-level transport and resource model.
