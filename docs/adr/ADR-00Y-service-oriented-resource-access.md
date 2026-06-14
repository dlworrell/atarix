# ADR-00Y: Service-Oriented Resource Access

Status: Accepted

## Context

ATARIX is built around a Fabric-mediated architecture, a Supervisor-owned trust model, Discovery, Directory, and capability-based authorization.

As hardware and software subsystems are added, the project must avoid drifting into a conventional model where software directly reaches into physical addresses, device registers, or backplane resources.

Direct access would undermine the security model, make hardware implementation details visible as software contracts, and create bypass paths around capability policy.

## Decision

ATARIX is service-addressed, not address-addressed.

All hardware and protected resources are accessed through named services, discovered through Discovery, resolved through Directory, mediated by Fabric, and authorized by Capability Policy.

No software component may treat physical addresses, raw registers, device locations, or known handles as authority.

## Core Rules

```text
No direct hardware access.
No raw address access.
No bypass around Fabric abstraction.
No bypass around Capability Policy.
```

The required access path is:

```text
Discovery
    -> Directory
    -> Service Handle
    -> Capability Policy
    -> Fabric Enforcement
    -> Resource Access
```

## Invariants

### No Direct Hardware Access

Software shall not directly access hardware resources.

All protected hardware access shall occur through Fabric-mediated services.

This applies to:

```text
Applications
Services
Drivers
Kernel subsystems
Fabric clients
```

Only the implementation of a service may know the underlying physical hardware details required to fulfill that service contract.

### Addresses Are Not Interfaces

Physical addresses are implementation details.

Service contracts are interfaces.

A UART, block device, mailbox, framebuffer, timer, or accelerator may be implemented:

```text
Inside FPGA logic
On a local CPU card
On a peripheral card
Behind a bridge
As a virtual service
On a future remote node
```

The caller shall interact with the service contract, not the physical placement.

### Discovery Is Not Authority

Discovery advertises existence.

Discovery answers:

```text
What exists?
```

Discovery does not answer:

```text
May I use it?
```

### Directory Is Not Authority

Directory describes and resolves resources.

Directory answers:

```text
Where is it?
How is it named?
What service contract does it expose?
```

Directory does not answer:

```text
May I use it?
```

### Knowledge Is Not Authority

The following do not grant authority:

```text
Knowing a physical address
Knowing a register offset
Knowing a handle
Knowing a service name
Enumerating Discovery
Resolving Directory
```

Only successful capability evaluation grants authority.

### Fabric Is the Enforcement Point

Supervisor owns trust.

Fabric enforces trust.

Every protected operation must ultimately pass through:

```text
Capability Policy
    -> Fabric Enforcement
    -> Resource Access
```

## Consequences

This decision intentionally rejects code patterns such as:

```c
#define UART_BASE 0xF0000000u
*(volatile unsigned int *)(UART_BASE + 0x10u) = value;
```

ATARIX callers should instead use service-oriented access patterns:

```text
lookup service
obtain handle
present capability
submit request
receive response
```

The exact C API for this flow will be defined by later Directory and service-call sprints.

## Security Consequences

Possession of an address is not authority.

Possession of a handle is not authority.

Directory lookup is not authority.

Discovery enumeration is not authority.

Only a valid capability, evaluated by policy and enforced by Fabric, authorizes action.

## Hardware Validation Requirements

Hardware Validation Sprint 1 and later bench tests shall prove that:

```text
Discovery only        -> no authority
Directory only        -> no authority
Known handle only     -> no authority
Known address only    -> no authority
Valid capability      -> evaluate through policy
Invalid capability    -> deny
Revoked capability    -> deny
Expired capability    -> deny
```

Bench validation should include attempts to bypass the Fabric service path and confirm that those bypasses fail.

## Relationship to Other ADRs

This ADR depends on the Capability Evaluation Model.

It will constrain the Directory Model, Fabric Service Model, Memory Services, DMA, Storage, Networking, GPU services, and SMP integration.

## Result

ATARIX resources are accessed as services, not raw addresses.

The system architecture preserves abstraction, portability, and capability-mediated security by prohibiting direct hardware access as a software contract.
