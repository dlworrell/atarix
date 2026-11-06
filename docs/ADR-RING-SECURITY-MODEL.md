<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:d24b06e0-423c-4a5e-9e0c-a46df558f0d8
-->

# ADR: Ring Security Model

Status: Accepted

## Context

ATARIX separates identity, enumeration, trust, capabilities, directory services, and transport.

That separation prevents discovery from becoming authorization, but the platform still needs a clear privilege model that explains which agents may validate, enumerate, authorize, configure, and operate system resources.

The ring security model provides that privilege structure.

## Decision

ATARIX uses a ring-style security model across firmware, supervisor services, fabric services, operating system services, and applications.

The ring model applies to:

- Management Fabric
- Enumeration Fabric
- Transport Fabric
- Identity EEPROM validation
- Discovery ROM enumeration
- Capability Records
- Service Directory
- Instrumentation services
- Provisioning and recovery
- CPU cards and future SMP systems

## Core Rule

```text
Enumeration describes.
Capabilities authorize.
Rings constrain authority.
```

A discovered service is not automatically usable.

A valid capability is not automatically usable from every privilege level.

## Rings

This ADR is the canonical definition of the ATARIX rings. Code
(`include/atarix/rings.h`), wire formats (`capability-record-format-v1.md`)
and every other document must use these numbers, names and domains.

ATARIX has two kinds of ring and one containment state:

- **Platform rings (−2, −1)** hold platform authority outside the CPU
  operating system. No software on a CPU card runs in them or can reach them.
- **Operating-system rings (0–3)** are the four CPU execution rings.
- **Quarantine (4)** is not an execution ring. It marks a contained subject
  that may not request anything.

Lower numbers are more privileged. A ring check only constrains authority; it
never grants it. A capability is always required as well.

| Ring | Code name | Wire | Domain | Holders | Must never |
|---|---|---|---|---|---|
| −2 | `ATARIX_RING_SUPERVISOR` | `0xFFFE` | Hardware safety and supervisor authority: reset, power, watchdog, identity validation, quarantine, recovery, supervisor audit | RP2350 supervisor | depend on a CPU, the OS, transport or directory services to recover |
| −1 | `ATARIX_RING_FABRIC` | `0xFFFF` | Fabric and platform firmware: enumeration, fabric configuration, transport setup, service binding support | ECP5 fabric controller, backplane BIOS, platform firmware | grant authority by enumerating something |
| 0 | `ATARIX_RING_KERNEL` | `0x0000` | Kernel / executive: memory, scheduling, driver loading, capability enforcement, transport use | Operating-system kernel on a CPU card | supersede supervisor safety authority |
| 1 | `ATARIX_RING_DRIVER` | `0x0001` | Drivers and hardware-facing privileged services, with narrow, delegated device capabilities | Device drivers, accelerator and display services (for example `SERVICE_GPU_COMPUTE`), kernel-adjacent daemons | hold broad authority by default |
| 2 | `ATARIX_RING_USER_SERVICE` | `0x0002` | User services: filesystem, network, desktop and session services with explicit service capabilities | User-space service processes | touch hardware directly |
| 3 | `ATARIX_RING_APPLICATION` | `0x0003` | Applications, scripts and user tools | Ordinary programs | use hardware except through a lower ring |
| 4 | `ATARIX_RING_QUARANTINE` | `0x0004` | Containment state, not an execution ring | Any subject the supervisor has contained | request anything; only ring −2 may act on it |

Wire value `0x8000` (`ATARIX_RING_WIRE_INVALID`) is not a ring. Code returns
it for an out-of-range ring so that no capability range can match (fail
closed). No capability may name ring 4 or `0x8000`.

### Choosing between ring 1 and ring 2

Put a service in ring 1 when it operates hardware or holds device, DMA or
interrupt capabilities. Put it in ring 2 when it only uses other services.
The display card's GPU compute service is ring 1; a filesystem service that
reaches storage only through the storage driver is ring 2.

### Amendment record

| Date | Change |
|---|---|
| 2026-11-06 | Added the ring table, the code names and wire values, and ring 4 (quarantine), which code already used but this ADR did not define. Named ring 2 "user service" in code (`ATARIX_RING_USER_SERVICE`; `ATARIX_RING_SERVICE` kept as a deprecated alias). Resolved the conflicting ring 1/2 definitions in `security.md` and `capability-model.md` in favor of this ADR. See Architecture Review 004. |

## Ring -2: Hardware Safety / Supervisor Authority

Primary authority:

```text
RP2350 Supervisor
```

Responsibilities:

- safety policy
- reset control
- health monitoring
- identity validation
- provisioning authorization
- recovery entry
- fault logging
- card power/health policy
- engineering console access

Ring -2 must remain useful when CPUs, operating systems, or transport services are unavailable.

## Ring -1: Fabric / Platform Firmware Authority

Primary authorities:

```text
ECP5 Fabric Controller
Backplane BIOS
Platform firmware
```

Responsibilities:

- enumeration support
- fabric configuration
- transport setup
- service binding support
- instrumentation routing
- early boot services
- platform discovery handoff

Ring -1 is below the operating system but above ordinary device consumers.

## Ring 0: Kernel / Executive Authority

Primary authority:

```text
Operating system kernel
```

Responsibilities:

- process management
- memory management
- driver loading
- service policy consumption
- transport use
- runtime capability enforcement

Ring 0 consumes platform services but does not supersede supervisor safety authority.

## Ring 1: Driver / Service Authority

Responsibilities:

- device drivers
- privileged runtime services
- kernel-adjacent daemons
- hardware-facing software services, including accelerator and display
  services such as `SERVICE_GPU_COMPUTE`

Ring 1 components may receive delegated capabilities. Those capabilities
should be narrow: tied to specific devices, buffers, mailboxes and interrupts.

## Ring 2: User Service Authority

Responsibilities:

- user-space services
- network daemons
- filesystem services
- desktop services
- application frameworks

Ring 2 components operate with explicit service capabilities.

## Ring 3: Application Authority

Responsibilities:

- ordinary applications
- scripts
- user tools

Ring 3 has no direct hardware authority unless explicitly mediated by higher rings.

## Ring 4: Quarantine (Containment State)

Ring 4 is not an execution ring. Code uses it to mark a subject the
supervisor has contained.

Rules:

- A quarantined subject may not request anything, from any ring.
- Only Ring -2 may act on a quarantined subject (inspect, recover, release).
- No capability may name Ring 4.

An endpoint can also be quarantined through its mailbox lifecycle state
(ATX-SPEC-005), without changing its ring. The CPU card test bench does this
when the supervisor quarantines a reported device.

## Relationship to Identity

Identity EEPROM validation occurs under Ring -2 authority.

Identity proves existence.

Identity does not grant operational authority.

## Relationship to Discovery ROM

Discovery ROM enumeration occurs under Ring -1 / Ring -2 observation depending on boot phase.

Discovery describes resources and services.

Discovery does not grant access.

## Relationship to Capabilities

Capabilities are interpreted in the context of ring authority.

A capability record should identify:

- operation
- target resource or service
- allowed ring or minimum ring
- delegation rules
- revocation rules where applicable

A capability valid for Ring -1 is not automatically valid for Ring 0 or Ring 3.

## Relationship to Directory Services

Directory services may inform policy and binding.

Directory services do not override Ring -2 safety authority.

Recovery must not depend on directory services.

## Relationship to Instrumentation

Instrumentation services may expose highly sensitive platform visibility.

Passive observation may be available to lower rings by policy.

Active stimulus, pattern generation, trigger routing, and destructive tests require high authority.

Recommended default:

```text
Passive counters: Ring 0 or delegated Ring 1/2
Trace capture: Ring 0 or delegated service
Pattern generation: Ring -2 / Ring -1 only unless explicitly authorized
Recovery instrumentation: Ring -2
```

## Relationship to Homebrew Cards

Homebrew cards may be identified and enumerated without being trusted.

Default policy:

```text
Homebrew identity: allowed to enumerate if valid
Homebrew services: restricted by capability and ring policy
Homebrew DMA: denied unless explicitly authorized
Homebrew boot authority: denied unless explicitly authorized
Homebrew supervisor authority: denied
```

## Boot-Time Authority Chain

The CPU card software test bench runs this chain before releasing the CPU
from reset, and checks each step's supervisor audit event
(`docs/verification/cpu-card-software-testbench.md`).

Recommended boot authority flow:

```text
Ring -2 Supervisor validates platform health and identity
    ↓
Ring -1 Platform firmware enumerates resources and services
    ↓
Capabilities are evaluated against trust and policy
    ↓
Ring 0 kernel receives authorized platform view
    ↓
Ring 1/2/3 receive delegated services
```

## Security Review Checklist

Every new specification should answer:

```text
What ring owns this function?
What ring may observe it?
What ring may configure it?
What ring may use it?
What ring may delegate it?
What ring may recover it?
```

## Related ADRs

- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- ADR-THREE-FABRIC-ARCHITECTURE.md
- ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md

## Related Specifications

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- cpu-observability-contract-v1.md
- future capability-record-format-v1.md
- future service-directory-format-v1.md

## Summary

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Transport operates.
```
