# ATARIX Simulator Framework

Status: Skeleton

## Purpose

The ATARIX simulator is a reference implementation of the architecture, not a shortcut around it.

The simulator exists to validate Supervisor, Fabric, mailbox, service, resource, and capability behavior before physical RP2350, ECP5, or W65C816 hardware is available.

## Doctrine

Simulator security must equal hardware security.

The simulator must preserve:

```text
Mailbox validation
Capability checks
Service boundaries
Resource boundaries
No direct hardware access
No address-based authority
Observable failures
Deny by default
```

## Initial Virtual Topology

The first virtual topology contains exactly two nodes:

```text
+----------------+
| Supervisor Sim |
| Node 0x0000    |
+--------+-------+
         |
         | Mailbox
         v
+----------------+
| Fabric Sim     |
| Node 0x0001    |
+----------------+
```

## Node ID Ranges

Reserved simulator and system node ranges:

```text
0x0000          Supervisor
0x0001-0x00FF   Fabric infrastructure
0x0100-0x01FF   CPU nodes
0x0200-0x02FF   Storage services
0x0300-0x03FF   Display services
0x0400-0x04FF   Network services
0x0500-0x05FF   GPIO and peripheral services
0xFF00-0xFFFF   Test and simulator nodes
```

## Initial Message Scope

Phase 2B supports only:

```text
PING
PONG
VERSION_QUERY
VERSION_REPLY
```

No Discovery.

No Directory.

No Capability transport.

No device models.

No resources.

No memory maps.

## Required Message Path

All virtual traffic must traverse the mailbox validator.

Forbidden:

```text
Direct node mutation
Direct mailbox injection after validation point
Direct resource access
Direct register/address shortcuts
```

Required:

```text
Supervisor Sim
    -> Mailbox Protocol
    -> Fabric Sim
    -> Mailbox Protocol
    -> Supervisor Sim
```

## Success Criteria

Initial simulator success is:

```text
Supervisor sends PING.
Fabric receives PING.
Fabric replies PONG.
Supervisor receives PONG.
```

and:

```text
Supervisor sends VERSION_QUERY.
Fabric receives VERSION_QUERY.
Fabric replies VERSION_REPLY.
Supervisor receives VERSION_REPLY.
```

Both flows must validate header, CRC, sequence, and message type.

## Result

The simulator framework gives ATARIX a virtual hardware path before physical hardware exists.

The simulator is intentionally small at first so transport correctness and architecture compliance are proven before additional subsystems are added.
