# ATARIX Capability Model

## Status

Draft security architecture specification.

This document defines the capability system used to control access to memory, DMA, mailboxes, devices, FPGA services, and future accelerators.

## Design Philosophy

ATARIX assumes that attachment does not imply trust.

A device, accelerator, CPU card, or service module does not automatically receive unrestricted access merely because it is physically connected.

Instead, access is granted through explicit capabilities.

## Goals

1. Prevent unrestricted DMA.
2. Permit safe accelerator integration.
3. Support multi-CPU experimentation.
4. Enable revocation.
5. Remain implementable on a W65C816 platform.
6. Avoid requiring heavyweight cryptography for local operation.

## Definition

A capability is a structured authorization record describing:

- Who owns the capability.
- What resource is controlled.
- What operations are permitted.
- How long the grant remains valid.
- Whether the grant can be delegated.

## Capability Classes

### Memory Capability

Controls access to a memory range.

Permissions:

```text
Read
Write
Execute
```

### DMA Capability

Controls DMA engine access.

Permissions:

```text
Read Memory
Write Memory
Bidirectional
```

### Mailbox Capability

Controls mailbox endpoints.

Permissions:

```text
Send
Receive
Administrative
```

### Device Capability

Controls access to device registers.

Examples:

```text
UART
Network Controller
Storage Controller
```

### Fabric Capability

Controls FPGA services.

Examples:

```text
Discovery Access
Interrupt Configuration
DMA Configuration
```

### Accelerator Capability

Controls execution on accelerator resources.

Examples:

```text
Submit Job
Read Results
Manage Queue
```

## Capability Record Format

Initial structure:

```text
Offset  Size  Field
+00     2     Capability Class
+02     2     Capability Flags
+04     4     Owner Identifier
+08     4     Resource Identifier
+0C     4     Expiration
+10     4     Parent Capability
+14     4     Reserved
```

Minimum size:

```text
24 bytes
```

## Capability Flags

```text
Bit 0 Read
Bit 1 Write
Bit 2 Execute
Bit 3 Configure
Bit 4 Delegate
Bit 5 Revoke
Bit 6 Persistent
Bit 7 Audit Required
```

## Ownership Model

Capabilities belong to an owner.

Owner types:

```text
CPU
Supervisor
Fabric Service
Network Service
Storage Service
Accelerator
```

## Delegation

Capabilities may optionally be delegated.

Delegated capabilities:

- Cannot exceed the authority of the parent capability.
- Maintain a reference to the parent grant.
- Can be revoked by the parent owner.

## Revocation

Revocation is a core architectural feature.

A revoked capability becomes invalid immediately.

Future implementations may use:

- Capability tables
- Generation counters
- Revocation lists

## Memory Grants

Example:

```text
Owner: DMA Engine
Resource: Buffer A
Permissions: Read + Write
Length: 4096 bytes
Expiration: Job Completion
```

## DMA Grants

DMA operations require:

1. DMA capability.
2. Memory capability.
3. Valid ownership chain.

The DMA engine should reject requests lacking proper authorization.

## Mailbox Grants

Mailbox endpoints may require explicit authorization.

Examples:

```text
CPU -> Supervisor
CPU -> Fabric
CPU -> Accelerator
```

## Accelerator Grants

Accelerators should never receive unrestricted memory visibility.

Instead:

1. Job submitted.
2. Memory buffers granted.
3. Execution authorized.
4. Results returned.
5. Capabilities revoked.

## Discovery Integration

Discovery records may advertise:

- Required capability classes.
- Capability restrictions.
- Administrative requirements.

See:

```text
docs/discovery-rom-format.md
```

## Audit Events

Future systems may log:

- Capability creation.
- Capability delegation.
- Capability revocation.
- Capability violations.

## Future Hardware Assistance

Future FPGA revisions may assist with:

- DMA validation.
- Capability lookup.
- Access enforcement.
- Fault reporting.

The FPGA acts as an enforcement assistant rather than a trusted authority.

## Design Rule

The supervisor remains the root authority.

The FPGA assists enforcement.

Devices receive only the minimum authority necessary to perform their tasks.

## Open Questions

- Final capability table structure.
- Persistence across reboot.
- Distributed capability ownership.
- Hardware acceleration of validation.
- Formal capability serialization format.
- Cryptographic signing requirements for remote services.