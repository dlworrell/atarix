# ATARIX Mailbox Protocol v1

## Status

Draft protocol specification for communication among the W65C816 host, supervisor controller, FPGA fabric, service devices, future CPU cards, and accelerator modules.

## Purpose

Mailboxes provide a simple, debuggable message-passing mechanism before the system has a full operating system, driver framework, or high-level IPC layer.

The protocol must support:

- CPU-to-supervisor requests.
- CPU-to-fabric requests.
- CPU-to-device requests.
- CPU-to-CPU messages in later revisions.
- Accelerator command submission in later revisions.
- Fabric-resource and memory-object references that are not limited to W65C816 u24 addresses.

## Data Model

Mailbox fields follow:

```text
docs/data-model-and-endianness.md
```

All multi-byte fields are little-endian.

The W65C816 may use u24 CPU-local addresses, but mailbox payloads may carry u32 and u64 fields for fabric resources, object identifiers, transaction identifiers, timestamps, and large offsets.

## Design Goals

1. Keep Rev A and Rev B message handling simple.
2. Make every message observable with a monitor command.
3. Avoid hidden side effects.
4. Permit future expansion without changing the basic message model.
5. Support explicit status and fault reporting.
6. Avoid treating u24 CPU-local addresses as universal ATARIX addresses.

## Compact Message Header

Rev A may use a compact header for small messages.

```text
Offset  Size  Field
+00     1     Protocol Version
+01     1     Message Type
+02     1     Source ID
+03     1     Destination ID
+04     1     Flags
+05     1     Sequence Number
+06     2     Payload Length, u16
```

The compact header is intended for small control messages and early firmware bring-up.

If the payload exceeds the u16 length limit, the message must use the extended header form.

## Extended Message Header

Extended messages use a larger header.

```text
Offset  Size  Field
+00     1     Protocol Version
+01     1     Header Length
+02     1     Message Type
+03     1     Flags
+04     1     Source ID
+05     1     Destination ID
+06     2     Reserved
+08     8     Transaction ID, u64
+10     4     Payload Length, u32
+14     8     Capability Reference, u64, optional or zero
```

The extended header supports:

- u32 payload lengths.
- u64 transaction IDs.
- u64 capability references.
- future 32-bit and 64-bit processors.
- fabric-resource and memory-object operations.

If a future message format needs payloads larger than u32, it should use the extended-length rule from `docs/data-model-and-endianness.md` or define a new versioned message type.

## Length Policy

Recommended length usage:

```text
Small local messages:       compact u16 payload length
Extended command messages:  u32 payload length
Large objects:              u64 size/offset inside payload
```

Mailbox messages should not directly carry enormous payloads when a memory object or DMA buffer reference is more appropriate.

Large data movement should use DMA or memory-service objects, not mailbox payloads.

## Message Types

```text
$00 Null / No Operation
$01 Ping
$02 Status Request
$03 Status Response
$04 Command Request
$05 Command Response
$06 Fault Report
$07 Discovery Request
$08 Discovery Response
$09 DMA Request
$0A DMA Response
$0B Accelerator Command
$0C Accelerator Completion
$0D Memory Object Request
$0E Memory Object Response
$0F Capability Request
$10 Capability Response
$80-$FF Vendor / Experimental
```

## Flags

```text
Bit 0 Response Required
Bit 1 Response Message
Bit 2 Error
Bit 3 Busy
Bit 4 Uses Extended Header
Bit 5 Carries Capability Reference
Bit 6 Carries Object Reference
Bit 7 Reserved
```

## Device Identifiers

Initial logical IDs:

```text
$00 Broadcast / Reserved
$01 Primary W65C816 CPU
$02 Supervisor Controller
$03 FPGA Fabric
$04 UART Service
$05 Network Service
$06 Storage Service
$07 Timer Service
$08 DMA Engine
$09 Memory Service
$0A-$1F Reserved System Devices
$20-$7F Slot Devices
$80-$FF Experimental / Vendor Defined
```

These IDs are compact routing identifiers, not persistent global object IDs.

Persistent or long-lived objects should use u64 identifiers in the message payload.

## Register Interface

Mailbox registers are defined in:

```text
docs/register-map-v1.md
```

Base address:

```text
$00D200
```

Required registers:

```text
+00 Mailbox Status
+01 Mailbox Control
+02 Message Length Low
+03 Message Length High
+04 Source ID
+05 Destination ID
+06 Message Type
+07 Flags
```

Future register blocks may expose extended-message registers or descriptor queues.

## Transfer Model

A minimal mailbox transaction follows this pattern:

1. Sender checks that mailbox is not busy.
2. Sender writes message header and payload.
3. Sender sets the submit bit.
4. Receiver observes pending status or interrupt.
5. Receiver reads and acknowledges the message.
6. Receiver optionally writes a response message.

## Status Values

```text
$00 Idle
$01 Pending
$02 Busy
$03 Complete
$04 Error
$05 Invalid Destination
$06 Invalid Message Type
$07 Payload Too Large
$08 Permission Denied
$09 Invalid Capability
$0A Invalid Object Reference
```

## Fault Handling

Fault reports should include:

- Fault source.
- Fault class.
- Related sequence number or transaction ID.
- Optional diagnostic code.
- Optional u64 timestamp.
- Optional u64 capability or object reference.

## Capability Interaction

Mailbox messages that request DMA, privileged register access, accelerator execution, memory-object mapping, or fabric reconfiguration must carry or reference an explicit capability grant.

Capability references should be u64.

## Open Questions

- Maximum Rev B compact payload size.
- Whether payload storage is register-backed or RAM-backed.
- Whether sequence numbers are per-source or global.
- Whether extended transaction IDs replace compact sequence numbers in later revisions.
- Whether mailbox delivery is interrupt-driven, polled, or both.
- Final encoding for capability references in compact messages.