# ATARIX Specification Roadmap

## Purpose

This document tracks the remaining specifications required to move ATARIX from architecture definition into implementation.

Guiding principle:

```text
Specification
    -> Header
    -> Implementation
    -> Test
```

Every specification should eventually produce:

- a public header under include/atarix/
- firmware and/or emulator code
- validation tests

---

# Architecture Synchronization

- [ ] architecture-timeline.md
- [ ] ADR-CPU-WIDTH-IS-NOT-FABRIC-WIDTH.md
- [ ] README normalization pass
- [ ] Archive synchronization pass

---

# Identity Layer

## Identity EEPROM v1

Status: Planned

Outputs:

- docs/identity-eeprom-v1.md
- include/atarix/identity_eeprom.h

Defines:

- board identity
- manufacturing identity
- serial numbering
- capability flags
- revision tracking

## Discovery ROM Format v1

Status: Planned

Outputs:

- docs/discovery-rom-format-v1.md
- include/atarix/discovery.h

Defines:

- device discovery
- service advertisement
- resource description

## Capability Record Format v1

Status: Planned

Outputs:

- docs/capability-record-format-v1.md
- include/atarix/capability.h

Defines:

- service capabilities
- permissions
- feature advertisement

---

# Discovery Layer

## Service Directory Format v1

Status: Planned

Outputs:

- docs/service-directory-format-v1.md
- include/atarix/service.h

Defines:

- discovered services
- service handles
- provider records

## Boot Information Block v1

Status: Planned

Outputs:

- docs/boot-information-block-v1.md
- include/atarix/bib.h

Defines:

- firmware-to-kernel handoff
- service directory location
- memory information
- boot information

---

# Transport Layer

## Mailbox Protocol v1

Status: Planned

Outputs:

- docs/mailbox-protocol-v1.md
- include/atarix/mailbox.h

Defines:

- request/reply transport
- transaction IDs
- DMA requests
- discovery traffic

## DMA Descriptor Format v1

Status: Planned

Outputs:

- docs/dma-descriptor-format-v1.md
- include/atarix/dma.h

Defines:

- DMA descriptors
- DMA handles
- completion records

## Fabric CSR Map v1

Status: Planned

Outputs:

- docs/fabric-csr-map-v1.md

Defines:

- software-visible fabric registers
- mailbox CSRs
- interrupt CSRs
- DMA CSRs

---

# Operations Layer

- [ ] backplane-bios-v1.md
- [ ] atarix-monitor-v1.md
- [ ] instrumentation-service-v1.md
- [ ] directory-service-v1.md
- [ ] factory-provisioning-v1.md
- [ ] cpu-validation-strategy-v1.md

---

# Source Tree Milestones

## Firmware

```text
src/firmware/monitor/
```

Goal:

```text
RESET
 -> UART
 -> Identity EEPROM
 -> Monitor Prompt
```

## Emulator

```text
src/emulator/fabric/
src/emulator/discovery/
src/emulator/mailbox/
```

Goal:

Architecturally accurate models of:

- discovery
- service enumeration
- mailbox transport
- Boot Information Block creation

---

# First Boot Milestone

Success criteria:

```text
Power On
 ↓
Supervisor
 ↓
Fabric
 ↓
CPU ROM
 ↓
Identity EEPROM
 ↓
Discovery ROM
 ↓
Service Directory
 ↓
Boot Information Block
 ↓
Kernel Stub
```

Expected monitor output:

```text
ATARIX ROM MONITOR v0.1

Board: Rev A CPU Card
Vendor: ATARIX
Serial: 000001

>
```

When this milestone is reached, ATARIX transitions from architecture project to functioning platform.
