# ATX-SPEC-005 Mailbox Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines Atarix mailbox semantics.

Mailboxes are the primary architectural mechanism for inter-object communication.

Mailbox transport does not grant authority.

A mailbox endpoint allows communication only under the authority, lifecycle, resource, ring, and capability rules applicable to the message and target object.

## Core Rule

```text
Connectivity is not trust.
```

The ability to send a message does not imply the ability to perform the requested operation.

## Mailbox As Object

A mailbox endpoint is an object or object-owned communication resource.

Mailbox endpoints have:

```text
Owner
Lifecycle state
Resource accounting
Authority requirements
Queue limits
Audit policy
Cleanup behavior
Versioned message format
```

## Authority

Mailbox authority may include:

```text
SEND
RECEIVE
OBSERVE
CONFIGURE
CLOSE
DELEGATE
```

Sending a message is separate from authorizing the operation requested by the message.

## Message Validation

A message must be validated for:

```text
Format version
Length
CRC or integrity field where applicable
Message type
Source identity label
Target object identity or binding
Ring policy
Capability authority
Lifecycle state
Resource availability
```

Malformed or unknown messages must fail closed for authority-bearing operations.

## Ring Boundaries

Cross-ring mailbox communication must be explicit and mediated.

A lower-privilege object must not gain authority by sending messages to a higher-privilege mailbox.

A higher-privilege object must not accidentally launder authority through a lower-privilege mailbox.

## Resource Accounting

Mailbox capacity, queue depth, buffer memory, DMA windows, and transport credits are resources.

Mailbox exhaustion must not bypass validation, policy, or audit requirements.

Mailbox resource use must be owned, accounted, bounded, and cleanable.

## Lifecycle

Mailbox endpoints may be:

```text
CREATED
ACTIVE
SUSPENDED
DRAINING
CLOSING
CLOSED
QUARANTINED
FAILED
```

Endpoint closure must clean up pending messages, resource allocations, capabilities, and directory bindings according to policy.

## Audit

Mailbox events that should be auditable include:

```text
Endpoint created
Endpoint closed
Message denied
Malformed message rejected
Cross-ring message accepted
Cross-ring message denied
Queue exhausted
Endpoint quarantined
Transport fault
```

Audit visibility does not imply control over mailbox endpoints.

## Error Handling

Mailbox errors must distinguish:

```text
Malformed message
Unknown message type
Unsupported version
Authorization denied
Target unavailable
Queue exhausted
Timeout
Transport failure
Target quarantined
```

Unknown mailbox state must be explicit.

## Supervisor Mailbox Boundary

Supervisor control interfaces are not ordinary operational mailboxes.

Operational fabric objects may request supervisor actions only through explicitly mediated interfaces.

Supervisor control remains isolated.

Supervisor audit export may be observable through an audit bridge.

Observation is not control.

## Bootstrap Mailboxes

Bootstrap mailboxes, if present, belong to the Bootstrap Security Model.

Bootstrap communication must not be treated as normal runtime authority.

Bootstrap authority is not runtime authority.

## Versioning

Mailbox messages must carry or imply a version.

Unknown versions must fail closed for authority-bearing operations unless explicit compatibility rules permit degraded interpretation.

## Required Future Work

- Define mailbox message wire format.
- Define mailbox endpoint identity.
- Define mailbox resource accounting structures.
- Define version negotiation in ATX-SPEC-012.
- Define policy interaction in ATX-SPEC-013.
- Define supervisor mailbox restrictions in ATX-SPEC-016.

## Summary

Mailboxes provide communication, not trust.

Every authority-bearing request still requires explicit validation.
