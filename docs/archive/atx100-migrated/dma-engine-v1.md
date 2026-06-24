# ATARIX DMA Engine v1

## Status

Draft specification for the ATARIX DMA subsystem.

This document defines DMA operation, descriptor formats, ownership rules, capability integration, fault handling, interrupt behavior, and the distinction between W65C816-native transfers and fabric/object-space transfers.

## Purpose

The DMA engine provides controlled movement of data between CPU-local memory, memory services, devices, FPGA services, storage services, and future accelerators.

Unlike traditional bus-mastering systems, DMA is not automatically trusted.

DMA operations require authorization.

## Data Model

DMA descriptors follow:

```text
docs/data-model-and-endianness.md
```

All multi-byte fields are little-endian.

The W65C816 uses u24 CPU-local addresses, but ATARIX DMA must also support u64 resource identifiers, u64 offsets, u64 object sizes, and future 32-bit / 64-bit CPU participants.

## Design Goals

1. Eliminate unrestricted DMA.
2. Permit future accelerators.
3. Support mailbox-driven operation.
4. Support capability validation.
5. Remain practical for a W65C816-based system.
6. Permit future FPGA enforcement.
7. Avoid treating u24 CPU-local addresses as universal system addresses.
8. Support large memory-service and storage-service objects without changing the core security model.

## DMA Architecture

The DMA engine is initially implemented as an FPGA fabric service.

The engine performs transfers on behalf of authorized requestors.

Requestors may include:

```text
W65C816 CPU Card
Future 32-bit CPU Card
Future 64-bit CPU Card
Supervisor
Network Service
Storage Service
Memory Service
Accelerator
Future Devices
```

## DMA Channels

Initial implementation:

```text
4 logical channels
```

Future revisions may increase channel count.

## Transfer Types

Supported transfers:

```text
CPU-Local Memory -> CPU-Local Memory
CPU-Local Memory -> Device
Device -> CPU-Local Memory
Memory Object -> CPU-Local Window
CPU-Local Window -> Memory Object
Memory Object -> Device
Device -> Memory Object
Memory Object -> Accelerator
Accelerator -> Memory Object
Memory Object -> Memory Object
```

Unsupported transfers must return an error.

## Descriptor Classes

ATARIX defines two descriptor classes:

```text
Native Descriptor
    Compact descriptor for W65C816-native u24 CPU-local transfers.

Fabric Descriptor
    Extended descriptor for fabric resources, memory objects, storage extents, accelerators, and future CPU cards.
```

Native descriptors are useful for early bring-up and local CPU-card transfers.

Fabric descriptors are required for large memory, storage, accelerator, and future multi-processor transfers.

## Native Descriptor Format

Version 1 native descriptor:

```text
Offset  Size  Field
+00     1     Descriptor Type = $01
+01     1     Flags
+02     1     Channel
+03     1     Priority
+04     3     Source CPU Address, u24
+07     3     Destination CPU Address, u24
+0A     4     Length, u32
+0E     2     Reserved
```

Descriptor size:

```text
16 bytes
```

Native addresses are W65C816 CPU-local addresses.

Native descriptor addresses are not universal ATARIX fabric addresses.

## Fabric Descriptor Format

Version 1 fabric descriptor:

```text
Offset  Size  Field
+00     1     Descriptor Type = $02
+01     1     Flags
+02     1     Channel
+03     1     Priority
+04     8     Source Resource ID, u64
+0C     8     Source Offset, u64
+14     8     Destination Resource ID, u64
+1C     8     Destination Offset, u64
+24     4     Length, u32 or extended-length escape
+28     8     Capability Reference, u64
+30     8     Transaction ID, u64
```

Descriptor size:

```text
56 bytes
```

The source and destination resource IDs may refer to:

- CPU-local memory apertures.
- Memory-service objects.
- Storage-service extents.
- Device buffers.
- Accelerator buffers.
- Fabric-managed queues.

## Extended Length Rule

The fabric descriptor length field normally uses u32.

```text
Length != $FFFFFFFF
    Length is encoded directly as u32.

Length == $FFFFFFFF
    Actual length is supplied by an associated extended descriptor or descriptor-chain record using u64.
```

Formats expected to represent persistent large objects should use u64 sizes in the owning object metadata.

DMA transfer lengths may remain u32 for common-case transfers, because very large transfers should usually be split into bounded operations for scheduling, watchdog, fault isolation, and recovery.

## Descriptor Flags

```text
Bit 0 Generate Completion Interrupt
Bit 1 Verify Capability
Bit 2 Source Is CPU-Local Address
Bit 3 Destination Is CPU-Local Address
Bit 4 Uses Extended Length
Bit 5 Scatter/Gather Descriptor
Bit 6 Reserved
Bit 7 Reserved
```

## Capability Validation

DMA operations require:

1. Valid DMA capability.
2. Valid source-resource capability.
3. Valid destination-resource capability.
4. Matching ownership chain.
5. Ring/security policy allowing the request.

Failure of any validation step causes immediate rejection.

Capability references are u64.

## DMA Lifecycle

### Step 1

Requestor creates a native or fabric descriptor.

### Step 2

Descriptor is submitted through mailbox service or a descriptor queue.

### Step 3

DMA engine validates descriptor format.

### Step 4

DMA engine validates capabilities.

### Step 5

DMA engine validates source and destination ranges.

### Step 6

Transfer executes.

### Step 7

Completion status is recorded.

### Step 8

Interrupt or mailbox completion is generated if requested.

## Register Interface

See:

```text
docs/register-map-v1.md
```

DMA block base:

```text
$00D300
```

Minimal native-register interface:

```text
+00 Control
+01 Status
+02 Source Address Low
+03 Source Address Mid
+04 Source Address High
+05 Destination Address Low
+06 Destination Address Mid
+07 Destination Address High
+08 Length Byte 0
+09 Length Byte 1
+0A Length Byte 2
+0B Length Byte 3
```

Future revisions may use descriptor queues instead of direct registers.

Register blocks exposing 32-bit or 64-bit fields must define snapshot and commit semantics in `docs/register-map-v1.md`.

## Status Codes

```text
$00 Idle
$01 Pending
$02 Active
$03 Complete
$04 Invalid Descriptor
$05 Invalid Capability
$06 Invalid Address
$07 Access Denied
$08 Timeout
$09 Internal Error
$0A Unsupported Descriptor Type
$0B Invalid Resource ID
$0C Range Exceeds Resource
```

## Interrupt Behavior

DMA completion may generate:

```text
DMA Complete
DMA Fault
DMA Timeout
```

Interrupt routing is controlled through the fabric interrupt controller.

## Ownership Rules

A requestor may only submit DMA work for resources it controls.

Examples:

Allowed:

```text
Network Service -> Network Buffer
Memory Service -> Granted CPU Window
Accelerator -> Granted Result Buffer
```

Denied:

```text
Network Service -> Arbitrary Kernel Memory
Accelerator -> CPU-local Bank Zero
Unknown Device -> Memory Service Object
```

## Accelerator Integration

Accelerators never receive unrestricted DMA authority.

Instead:

1. Memory capability issued.
2. DMA capability issued.
3. Job executed.
4. Results returned.
5. Capabilities revoked.

## Fault Reporting

DMA faults should report:

```text
Requestor
Descriptor Type
Capability Reference, u64
Source Resource ID or CPU Address
Destination Resource ID or CPU Address
Length
Transaction ID, if present
Fault Code
Timestamp, if available
```

Fault reports should be accessible through mailbox services and supervisor diagnostics.

## Future Enhancements

Potential future features:

```text
Scatter/Gather
Descriptor Queues
Priority Scheduling
Rate Limiting
Capability Caching
FPGA Enforcement Tables
u64 Extended-Length Descriptor Records
IOMMU-like Resource Windows
```

## Design Rule

DMA exists to move data.

DMA does not confer authority.

Authority is provided only through capabilities.

Native u24 DMA is a compatibility and bring-up path, not the full ATARIX memory model.

## Open Questions

- Final descriptor queue design.
- Maximum single transfer size.
- Channel scheduling algorithm.
- Capability lookup mechanism.
- Hardware enforcement strategy.
- Accelerator-specific DMA extensions.
- Whether extended-length transfers should be required to split at watchdog-safe boundaries.
- Whether resource IDs are assigned by fabric, supervisor, or capability broker.