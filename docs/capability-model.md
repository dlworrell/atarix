# ATARIX Capability Model

## Status

Draft security architecture specification.

This document defines the capability system used to control access to memory, DMA, mailboxes, devices, FPGA services, future 32-bit / 64-bit CPU cards, memory services, storage services, and accelerators.

## Design Philosophy

ATARIX assumes that attachment does not imply trust.

A device, accelerator, CPU card, or service module does not automatically receive unrestricted access merely because it is physically connected.

Instead, access is granted through explicit capabilities.

## Data Model

Capability records follow:

```text
docs/data-model-and-endianness.md
```

All multi-byte fields are little-endian.

Capability identifiers, resource identifiers, memory object identifiers, revocation identifiers, and persistent audit references should use u64.

The W65C816's u24 CPU-local address space must not constrain capability records or fabric-resource identity.

## Goals

1. Prevent unrestricted DMA.
2. Permit safe accelerator integration.
3. Support multi-CPU experimentation.
4. Enable revocation.
5. Remain implementable on a W65C816 platform.
6. Avoid requiring heavyweight cryptography for local operation.
7. Support fabric resources and memory objects larger than the W65C816 native address space.
8. Support future 32-bit and 64-bit CPU cards without changing the authority model.

## Definition

A capability is a structured authorization record describing:

- Who owns the capability.
- What resource is controlled.
- What operations are permitted.
- How long the grant remains valid.
- Whether the grant can be delegated.
- How it can be revoked or audited.

## Capability Classes

### Memory Capability

Controls access to a memory range, memory window, or memory-service object.

Permissions:

```text
Read
Write
Execute
Map Window
DMA Visible
```

### DMA Capability

Controls DMA engine access.

Permissions:

```text
Read Memory
Write Memory
Bidirectional
Submit Descriptor
Manage Queue
```

### Mailbox Capability

Controls mailbox endpoints.

Permissions:

```text
Send
Receive
Administrative
Privileged Command
```

### Device Capability

Controls access to device registers.

Examples:

```text
UART
Network Controller
Storage Controller
Timer Device
```

### Fabric Capability

Controls FPGA fabric services.

Examples:

```text
Discovery Access
Interrupt Configuration
DMA Configuration
Mailbox Routing
Fabric Register Access
```

### Memory Service Capability

Controls managed memory resources that may exceed CPU-local address sizes.

Examples:

```text
Allocate Object
Map Window
Read Object
Write Object
Pin Object For DMA
Revoke Mapping
```

### Storage Service Capability

Controls persistent storage resources.

Examples:

```text
Read Extent
Write Extent
Create Object
Delete Object
Snapshot Object
```

### Accelerator Capability

Controls execution on accelerator resources.

Examples:

```text
Submit Job
Read Results
Manage Queue
Grant DMA Window
```

## Capability Record Format v1

Version 1 capability record:

```text
Offset  Size  Field
+00     2     Capability Class, u16
+02     2     Capability Flags, u16
+04     8     Capability ID, u64
+0C     8     Owner ID, u64
+14     8     Resource ID, u64
+1C     8     Resource Offset, u64
+24     8     Resource Length, u64
+2C     8     Expiration Time or Monotonic Expiration, u64
+34     8     Parent Capability ID, u64, or zero
+3C     8     Revocation Generation, u64
+44     4     Reserved, must be zero
```

Minimum size:

```text
72 bytes
```

The larger record is intentional. Capabilities are security-critical and should not be constrained by early CPU-local addressing limits.

## Compact Capability References

Compact protocols may carry a u64 Capability ID rather than embedding the full record.

Examples:

```text
Mailbox extended header
DMA fabric descriptor
Fault log entry
Supervisor diagnostic record
```

The full capability record may live in:

- Kernel memory.
- Fabric memory.
- Supervisor-managed tables.
- Memory-service metadata.
- A future capability broker.

## Capability Flags

```text
Bit 0  Read
Bit 1  Write
Bit 2  Execute
Bit 3  Configure
Bit 4  Delegate
Bit 5  Revoke
Bit 6  Persistent
Bit 7  Audit Required
Bit 8  DMA Visible
Bit 9  Map Window
Bit 10 Privileged Operation
Bit 11 Supervisor Visible
Bits 12-15 Reserved
```

## Ownership Model

Capabilities belong to an owner.

Owner identifiers should be u64.

Owner types:

```text
W65C816 CPU Card
Future 32-bit CPU Card
Future 64-bit CPU Card
Supervisor
Fabric Service
Network Service
Storage Service
Memory Service
Accelerator
```

A future secondary CPU card does not automatically receive Ring 0 or root authority merely because it is physically installed.

## Resource Model

Resources should be identified by u64 Resource IDs.

Resource examples:

```text
CPU-local aperture
Memory-service object
Storage extent
Mailbox endpoint
DMA channel
Interrupt source
Fabric register block
Accelerator queue
Device register window
```

Resource offsets and lengths should be u64 when referring to persistent or large resources.

Small local resources may use narrower fields in local protocols, but their capability records should remain u64-capable.

## Delegation

Capabilities may optionally be delegated.

Delegated capabilities:

- Cannot exceed the authority of the parent capability.
- Maintain a reference to the parent grant.
- Can be revoked by the parent owner.
- Should receive their own u64 Capability ID.

## Revocation

Revocation is a core architectural feature.

A revoked capability becomes invalid immediately.

Future implementations may use:

- Capability tables.
- Generation counters.
- Revocation lists.
- Per-resource revocation generations.
- Supervisor-visible quarantine records.

The Revocation Generation field allows stale cached capability references to be detected.

## Expiration

Expiration should use u64.

Possible encodings:

```text
u64 seconds since epoch
u64 monotonic tick deadline
u64 job-completion token
0 = no automatic expiration
```

The owning subsystem must specify which interpretation is used.

## Memory Grants

Example:

```text
Owner:       DMA Engine
Resource:    Memory Object #42
Offset:      0x0000000200000000
Length:      0x0000000000010000
Permissions: Read + Write + DMA Visible
Expiration: Job Completion
```

This example intentionally uses offsets larger than the W65C816 native u24 address space.

## DMA Grants

DMA operations require:

1. DMA capability.
2. Source-resource capability.
3. Destination-resource capability.
4. Valid ownership chain.
5. Ring/security policy allowing the request.

The DMA engine should reject requests lacking proper authorization.

See:

```text
docs/dma-engine-v1.md
```

## Mailbox Grants

Mailbox endpoints may require explicit authorization.

Examples:

```text
CPU -> Supervisor
CPU -> Fabric
CPU -> Accelerator
Service -> Memory Service
```

Mailbox messages that reference capabilities should use u64 Capability IDs.

See:

```text
docs/mailbox-protocol-v1.md
```

## Accelerator Grants

Accelerators should never receive unrestricted memory visibility.

Instead:

1. Job submitted.
2. Memory buffers granted.
3. Execution authorized.
4. Results returned.
5. Capabilities revoked.

Accelerator grants should be narrow, time-limited where practical, and auditable.

## Discovery Integration

Discovery records may advertise:

- Required capability classes.
- Capability restrictions.
- Administrative requirements.
- Resource ID requirements.
- Whether DMA access must be granted explicitly.

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
- Capability expiration.
- Capability use by DMA or accelerator engines.

Audit records should use u64 timestamps and u64 capability references.

## Future Hardware Assistance

Future FPGA revisions may assist with:

- DMA validation.
- Capability lookup.
- Access enforcement.
- Fault reporting.
- Revocation-generation checks.
- Resource-window bounds checking.

The FPGA acts as an enforcement assistant rather than the only trusted authority.

## Security Interaction With Rings

Privilege rings define execution authority.

Capabilities define resource authority.

Both are required.

Example:

```text
Ring 1 service + valid storage capability
    -> may operate assigned storage resource

Ring 3 program + no capability
    -> denied

Ring 2 driver + expired DMA capability
    -> denied
```

See:

```text
docs/security.md
```

## Design Rule

The supervisor remains the root management and recovery authority.

The FPGA assists enforcement.

Devices receive only the minimum authority necessary to perform their tasks.

Capability identity and resource identity are not limited by the W65C816 native address space.

## Open Questions

- Final capability table structure.
- Persistence across reboot.
- Distributed capability ownership.
- Hardware acceleration of validation.
- Formal capability serialization format.
- Cryptographic signing requirements for remote services.
- Whether 128-bit object identifiers will be required for distributed systems.
- Whether expiration uses wall-clock time, monotonic ticks, or both.