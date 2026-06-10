# ATARIX DMA Engine v1

## Status

Draft specification for the ATARIX DMA subsystem.

This document defines DMA operation, descriptor formats, ownership rules, capability integration, fault handling, and interrupt behavior.

## Purpose

The DMA engine provides controlled high-speed movement of data between memory, devices, FPGA services, and future accelerators.

Unlike traditional bus-mastering systems, DMA is not automatically trusted.

DMA operations require authorization.

## Design Goals

1. Eliminate unrestricted DMA.
2. Permit future accelerators.
3. Support mailbox-driven operation.
4. Support capability validation.
5. Remain practical for a W65C816-based system.
6. Permit future FPGA enforcement.

## DMA Architecture

The DMA engine is initially implemented as an FPGA service.

The engine performs transfers on behalf of authorized requestors.

Requestors may include:

```text
CPU
Supervisor
Network Service
Storage Service
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
Memory -> Memory
Memory -> Device
Device -> Memory
Memory -> Accelerator
Accelerator -> Memory
```

Unsupported transfers must return an error.

## Descriptor Format

Version 1 descriptor:

```text
Offset  Size  Field
+00     3     Source Address
+03     3     Destination Address
+06     3     Length
+09     1     Flags
+0A     1     Channel
+0B     1     Priority
+0C     4     Capability Reference
```

Descriptor size:

```text
16 bytes
```

Addresses are 24-bit.

## Descriptor Flags

```text
Bit 0 Generate Completion Interrupt
Bit 1 Verify Capability
Bit 2 Reserved
Bit 3 Reserved
Bit 4 Reserved
Bit 5 Reserved
Bit 6 Reserved
Bit 7 Reserved
```

## Capability Validation

DMA operations require:

1. Valid DMA capability.
2. Valid memory capability.
3. Matching ownership chain.

Failure of any validation step causes immediate rejection.

## DMA Lifecycle

### Step 1

Requestor creates descriptor.

### Step 2

Descriptor submitted through mailbox service.

### Step 3

DMA engine validates capabilities.

### Step 4

DMA engine validates memory ranges.

### Step 5

Transfer executes.

### Step 6

Completion status recorded.

### Step 7

Interrupt generated if requested.

## Register Interface

See:

```text
docs/register-map-v1.md
```

DMA block base:

```text
$00D300
```

Registers:

```text
+00 Control
+01 Status
+02 Source Low
+03 Source High
+04 Destination Low
+05 Destination High
+06 Length Low
+07 Length High
```

Future revisions may use descriptor queues instead of direct registers.

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
```

Denied:

```text
Network Service -> Arbitrary Kernel Memory
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
Capability Reference
Source Address
Destination Address
Fault Code
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
```

## Design Rule

DMA exists to move data.

DMA does not confer authority.

Authority is provided only through capabilities.

## Open Questions

- Final descriptor queue design.
- Maximum transfer size.
- Channel scheduling algorithm.
- Capability lookup mechanism.
- Hardware enforcement strategy.
- Accelerator-specific DMA extensions.