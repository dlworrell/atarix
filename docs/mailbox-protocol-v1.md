# ATARIX Mailbox Protocol v1

## Status

Draft protocol specification for communication among the W65C816 host, supervisor controller, FPGA fabric, service devices, and future accelerator modules.

## Purpose

Mailboxes provide a simple, debuggable message-passing mechanism before the system has a full operating system, driver framework, or high-level IPC layer.

The protocol must support:

- CPU-to-supervisor requests
- CPU-to-fabric requests
- CPU-to-device requests
- CPU-to-CPU messages in later revisions
- Accelerator command submission in later revisions

## Design Goals

1. Keep Rev A and Rev B message handling simple.
2. Make every message observable with a monitor command.
3. Avoid hidden side effects.
4. Permit future expansion without changing the basic header layout.
5. Support explicit status and fault reporting.

## Message Header

All mailbox messages begin with a fixed header.

```text
Offset  Size  Field
+00     1     Protocol Version
+01     1     Message Type
+02     1     Source ID
+03     1     Destination ID
+04     1     Flags
+05     1     Sequence Number
+06     1     Payload Length Low
+07     1     Payload Length High
```

Payload length is little-endian.

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
$80-$FF Vendor / Experimental
```

## Flags

```text
Bit 0 Response Required
Bit 1 Response Message
Bit 2 Error
Bit 3 Busy
Bit 4 Uses Extended Payload
Bit 5 Reserved
Bit 6 Reserved
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
$09-$1F Reserved System Devices
$20-$7F Slot Devices
$80-$FF Experimental / Vendor Defined
```

## Register Interface

Mailbox registers are defined in `docs/register-map-v1.md`.

Base address:

```text
$00D200
```

Required registers:

```text
+00 Mailbox Status
+01 Mailbox Control
+02 Message Length
+03 Source ID
+04 Destination ID
+05 Message Type
```

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
```

## Fault Handling

Fault reports should include:

- Fault source
- Fault class
- Related message sequence number
- Optional diagnostic code

## Capability Interaction

In later revisions, mailbox messages that request DMA, privileged register access, accelerator execution, or fabric reconfiguration must carry or reference an explicit capability grant.

## Open Questions

- Maximum Rev B payload size
- Whether payload storage is register-backed or RAM-backed
- Whether sequence numbers are per-source or global
- Whether mailbox delivery is interrupt-driven, polled, or both
- Final encoding for capability references