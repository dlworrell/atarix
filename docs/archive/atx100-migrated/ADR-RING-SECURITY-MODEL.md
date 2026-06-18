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

## Proposed Rings

```text
Ring -2  Hardware Safety / Supervisor Authority
Ring -1  Fabric / Platform Firmware Authority
Ring  0  Kernel / Executive Authority
Ring  1  Driver / Service Authority
Ring  2  User Service Authority
Ring  3  Application Authority
```

Negative rings are platform authority rings outside the normal CPU operating-system privilege model.

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
- hardware-facing software services

Ring 1 components may receive delegated capabilities.

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
