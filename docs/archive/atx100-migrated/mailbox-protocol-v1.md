# ATARIX Mailbox Protocol v1

Status: Baseline

## Purpose

The ATARIX Mailbox Protocol defines the first transport contract between Supervisor, Fabric, simulator components, and future hardware implementations.

The mailbox is not an authority mechanism. It transports requests and responses. Authority remains with Capability Policy and Fabric enforcement.

## Header Format

Mailbox v1 uses a fixed 16-byte header:

```c
struct atarix_mailbox_header_v1 {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
};
```

Current constants:

```text
Magic:              ATARIX_MAILBOX_MAGIC_V1
Header size:        16 bytes
Maximum payload:    4096 bytes
```

## Message Types

Mailbox v1 defines the following message types:

```text
INVALID
PING
PONG
VERSION_QUERY
VERSION_REPLY
STATUS_QUERY
STATUS_REPLY
READ_REGISTER
WRITE_REGISTER
RESET_REQUEST
FAULT_REPORT
DISCOVERY_RECORD_PUSH
CAPABILITY_EVALUATE
```

Unknown or invalid message types are rejected.

## Validation Order

Receivers validate mailbox traffic before dispatch.

Required validation order:

```text
Header present
Payload present when length is nonzero
Magic valid
Message type known
Length valid
Length matches payload buffer
CRC valid
Sequence accepted
Capability valid if required
Dispatch
```

Invalid traffic must not reach service dispatch.

## CRC Rules

The CRC covers the payload.

A CRC mismatch rejects the message.

CRC failure is observable and must return an explicit failure status.

## Length Rules

The header length must match the actual payload length.

Payload length must not exceed the protocol maximum.

Oversized payloads are rejected.

Malformed lengths are rejected.

## Sequence Rules

Mailbox sequences are tracked per receiver window.

Rules:

```text
First sequence accepted.
Exact duplicate rejected.
Older sequence rejected as out-of-order.
Sequence gaps rejected.
Next expected sequence accepted.
```

Sequence handling exists to detect replay, duplicate delivery, out-of-order delivery, and transport corruption.

## Dispatch Rules

Message dispatch occurs only after validation.

Preferred implementation for larger message sets is table-based dispatch:

```text
handler = dispatch_table[message_type]
```

Dispatch tables must not bypass validation.

## Security Rules

Mailbox routing does not grant authority.

The following are not authority:

```text
Valid mailbox header
Valid CRC
Known message type
Known sequence
Known endpoint
Known service handle
```

Capability Policy grants authority.

Protected operations must validate capabilities before resource access.

## Failure Rules

Failures must be observable.

Mailbox implementations must explicitly report:

```text
NULL input
Invalid header
Invalid length
CRC mismatch
Sequence error
Duplicate packet
Out-of-order packet
Unknown message type
Capability failure
```

Silent failure is not acceptable.

## Simulator Requirements

The simulator must follow the same mailbox rules as hardware.

Simulator implementations must reject malformed mailbox traffic in the same way future hardware-facing implementations are expected to reject it.

## Current Test Coverage

Mailbox v1 is covered by tests for:

```text
CRC validation
General validation
Sequence validation
Duplicate rejection
Out-of-order rejection
Unknown type rejection
Maximum payload handling
Oversized payload rejection
```

## Result

Mailbox Protocol v1 provides the first ATARIX transport contract.

It is intentionally small, deterministic, security-first, and suitable for simulation before hardware exists.
