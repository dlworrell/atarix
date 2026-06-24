# Chapter 13: Directory and Discovery Services

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/bios-api-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/bios-api-v1.md -->
### Integrated source: `docs/bios-api-v1.md`

# BIOS API v1

Hybrid firmware model. Firmware performs discovery, builds the Service Directory and Boot Information Block, then transfers control to the kernel. A small BIOS remains available for console, diagnostics, recovery, supervisor access, and firmware updates.

Service classes:
- Console
- Block I/O
- Network bootstrap
- Supervisor
- Firmware update
- Diagnostic monitor

BIOS calls return carry clear on success and carry set on failure.

<!-- AEMS-MIGRATED-SOURCE: docs/boot-firmware-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/boot-firmware-architecture-v1.md -->
### Integrated source: `docs/boot-firmware-architecture-v1.md`

# Boot Firmware and BIOS Architecture v1

## Purpose

This document defines the firmware, BIOS, and boot architecture for ATARIX systems.

The goals are:

- Deterministic system bring-up
- Hardware discovery
- Service discovery
- Diagnostic visibility
- Reliable kernel loading
- Independence from filesystems during early development

The firmware architecture is designed around the ATARIX Fabric Northbridge model.

## Architectural Overview

```text
W65C816 CPU Card
        ↓
Boot ROM
        ↓
Firmware / BIOS
        ↓
Fabric Northbridge Discovery
        ↓
Service Directory Construction
        ↓
Boot Service Selection
        ↓
Bootloader
        ↓
Operating System
```

The firmware is responsible for discovering hardware and constructing the initial service directory used by the operating system.

## Boot Sequence

```text
RESET
  -> ROM reset vector
  -> CPU initialization
  -> Native-mode transition
  -> Local SRAM initialization
  -> Fabric Northbridge initialization
  -> Discovery ROM enumeration
  -> Service directory construction
  -> BIOS service initialization
  -> ROM monitor availability
  -> Boot service selection
  -> Kernel loading
  -> Kernel handoff
```

## CPU Initialization

The W65C816 starts in emulation mode after reset.

Firmware shall:

1. Disable interrupts.
2. Clear decimal mode.
3. Enter native mode.
4. Configure stack pointer.
5. Configure direct page.
6. Initialize interrupt vectors.
7. Initialize console services.
8. Initialize Fabric Northbridge access.

## BIOS Responsibilities

- Hardware initialization
- CPU-local SRAM initialization
- Fabric Northbridge initialization
- Discovery ROM enumeration
- Service directory construction
- Console services
- Block device services
- Interrupt management
- Diagnostic monitor support
- Kernel loading support

## Fabric Northbridge Integration

The Fabric Northbridge is the architectural center of the system.

Firmware communicates with discovery services, service directory services, memory services, mailbox services, DMA services, and boot services.

All hardware is discovered dynamically.

## Discovery Phase

Each expansion card publishes a Discovery ROM.

```text
Enumerate Card
    -> Validate Discovery ROM
    -> Enumerate Services
    -> Record Resources
    -> Register Service Endpoints
```

The result is a runtime Service Directory.

## Service Directory

Example service classes:

- Console Service
- DMA Service
- Storage Service
- Network Service
- Framebuffer Service
- Memory Service
- Supervisor Service

Operating systems bind to services rather than physical slots.

## ROM Monitor

Recommended commands:

```text
M addr len       memory dump
E addr value     memory edit
S                list services
D                list devices
B                boot
R                reset
```

## Boot Services

Examples:

- Local Storage Boot Service
- Network Boot Service
- Recovery Boot Service

## Kernel Image Header

```c
struct atarix_kernel_header {
    char magic[8];
    uint16_t header_size;
    uint16_t flags;
    uint32_t load_addr;
    uint32_t entry_addr;
    uint32_t image_size;
    uint32_t checksum;
};
```

## Kernel Handoff

Before transferring control:

- Stack shall be valid.
- Direct page shall be valid.
- Service Directory shall be available.
- Fabric Northbridge shall be initialized.
- Boot Information Block shall be available.

## Boot Information Block

Provides:

- Firmware Version
- Service Directory Pointer
- Discovery Information
- Memory Service Information
- Boot Device Information
- Diagnostic Information

## Design Principles

1. Keep firmware simple.
2. Enumerate hardware before boot.
3. Discover services before loading the kernel.
4. Avoid filesystem dependencies during early bring-up.
5. Prefer raw block loading initially.
6. Keep policy in the operating system.
7. Keep mechanism in firmware.
8. Treat the Fabric Northbridge as the system integration point.
9. Make every stage observable and debuggable.
10. Support future heterogeneous processor cards.

<!-- AEMS-MIGRATED-SOURCE: docs/cpu-test-suite-integration-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/cpu-test-suite-integration-v1.md -->
### Integrated source: `docs/cpu-test-suite-integration-v1.md`

# CPU Test Suite Integration v1

## Purpose

This document defines how ATARIX should reuse external 65x02-family CPU test material for emulator validation, firmware validation, and eventual hardware bring-up.

The immediate source candidate is:

```text
dlworrell/65x02
```

That repository describes itself as a language-agnostic JSON-encoded instruction-by-instruction test suite for the 65[c]02 family that includes bus activity. Its README states that each test requires execution of only a single instruction and provides full processor and memory state before and after execution.

## Licensing

The `dlworrell/65x02` repository carries an MIT License.

Any copied or vendored test material must preserve the upstream copyright and license notice.

## Integration Policy

Do not blindly copy the entire upstream repository into ATARIX until the needed subset is identified.

Preferred approach:

1. Add a documented upstream reference.
2. Identify the exact CPU families and test sets needed.
3. Vendor only the test data required for ATARIX validation, or add a scripted fetch mechanism.
4. Preserve upstream license text.
5. Keep ATARIX-specific expected behavior and harness code separate from upstream data.

## Intended Uses

### Emulator Validation

The first use should be validating the ATARIX emulator CPU core or any CPU-core integration layer.

Use cases:

- single-instruction execution tests
- register before/after validation
- status flag validation
- memory before/after validation
- bus-cycle trace comparison where available

### Firmware Validation

The second use should be generating firmware-level validation ROMs or monitor commands that can run selected test vectors on ATARIX hardware.

These tests should not assume a full operating system.

### Hardware Bring-Up

The third use should be hardware-facing CPU validation.

These tests should help validate:

- reset behavior
- native-mode transition
- emulation-mode behavior
- address decoding
- interrupt routing
- bus timing assumptions
- watchdog interaction
- supervisor fault capture

## Required CPU Coverage

ATARIX requires validation coverage for:

```text
6502
65C02
W65C816
```

The `65x02` repository appears immediately useful for 6502 and 65C02-style instruction validation.

W65C816-specific behavior must be audited separately.

## Required ATARIX-Specific Coverage

The upstream test suite is instruction-level CPU validation. ATARIX also needs platform-level validation.

Missing ATARIX-specific coverage includes:

- W65C816 native mode
- W65C816 emulation mode transition
- 24-bit addressing
- bank register behavior
- direct page relocation
- stack relocation
- MVN and MVP block moves
- COP handling
- ABORT handling
- WAI behavior
- STP behavior
- IRQ routing through the Fabric Northbridge
- NMI routing
- reset vector behavior
- CPU-local SRAM decode
- CPU-local DMA buffer behavior
- mailbox access from CPU firmware
- Identity EEPROM read path
- supervisor watchdog interaction
- validation boot mode
- Backplane BIOS handoff
- network-first boot prerequisites

## Test Tree Plan

Create:

```text
tests/
├── cpu/
│   ├── upstream/
│   ├── harness/
│   └── atarix-specific/
├── eeprom/
├── discovery/
├── mailbox/
└── service_directory/
```

## Upstream Import TODO

- [ ] Inventory `dlworrell/65x02` repository tree.
- [ ] Identify available 6502 test sets.
- [ ] Identify available 65C02 test sets.
- [ ] Determine whether any W65C816 tests exist upstream.
- [ ] Preserve upstream MIT license in `tests/cpu/upstream/`.
- [ ] Decide between vendoring JSON test data or scripted download.
- [ ] Write ATARIX test manifest.
- [ ] Write emulator-side JSON runner.
- [ ] Write hardware validation ROM generator.
- [ ] Add CI test target for emulator CPU tests.

## Validation Strategy TODO

- [ ] Define pass/fail record format.
- [ ] Define failure reporting format for monitor output.
- [ ] Define supervisor event codes for CPU validation failure.
- [ ] Define validation boot-mode behavior.
- [ ] Define how test failures are exposed through the engineering console.
- [ ] Define reduced smoke-test set for hardware.
- [ ] Define full exhaustive emulator test set.

## Principle

Instruction-level CPU tests prove the CPU core.

ATARIX validation must also prove that the CPU is correctly integrated into the system.

<!-- AEMS-MIGRATED-SOURCE: docs/github-project-organization-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/github-project-organization-v1.md -->
### Integrated source: `docs/github-project-organization-v1.md`

# GitHub Project Organization v1

Status: Accepted

## Purpose

Define the organizational structure used to manage ATARIX development within GitHub.

Project management exists to support engineering work. The repository favors simplicity, traceability, and low administrative overhead.

## Workflow

All work follows:

```text
Discussion
    -> ADR
    -> Issue
    -> Implementation
    -> Test
    -> CI Green
    -> Close Issue
```

Design discussions belong in GitHub Discussions.

Authoritative architectural decisions belong in ADR documents.

Executable work belongs in GitHub Issues.

## Discussions

Discussions are divided into announcements and design discussions.

### Announcements

Announcements contain authoritative project information.

Current announcement numbering:

```text
001 - Project Roadmap
002 - Architecture Overview
003 - Development Workflow
```

Future announcements should reserve numbering gaps to allow insertion without renumbering.

Recommended numbering:

```text
001-099 Project governance and project status
```

### Design Discussions

Discussions are used for design exploration and architectural debate.

Recommended numbering:

```text
100-199 Architecture
200-299 Hardware
300-399 Toolchain
400-499 Kernel and operating system
```

Examples:

```text
110 - Capability Delegation Semantics
120 - Service Discovery Evolution
200 - CPU Card Mechanical Envelope
300 - 65C816 Toolchain Strategy
400 - Kernel ABI Model
```

## ADR Relationship

Discussions may lead to ADRs.

Example:

```text
Discussion 110
    -> ADR-002
    -> Issue
    -> Implementation
    -> Test
    -> CI Green
```

ADRs remain the authoritative record of architectural decisions.

## Milestones

Milestones represent sprint-level deliverables.

Current milestones:

```text
Discovery Sprint 1
Capability Sprint 1
Directory Sprint 1
Supervisor Runtime Sprint 1
Toolchain Sprint 1
65C816 Bring-up Sprint 1
```

Guideline:

```text
Milestone = weeks of work
Issue     = days of work
Task      = hours of work
```

## Issues

Issues represent actionable work.

Issues should normally have:

```text
Priority label
Type label
Area label
Milestone
```

Issue titles should describe the concrete work to be completed.

Examples:

```text
Implement capability_policy.c
Add capability evaluator tests
Implement directory_builder.c
Add supervisor health-state tests
```

## Labels

### Priority Labels

```text
P0-Critical
P1-High
P2-Normal
P3-Low
```

Priority meaning:

```text
P0 = Fix immediately
P1 = Current sprint
P2 = Future sprint
P3 = Backlog or cleanup
```

### Type Labels

```text
type:architecture
type:implementation
type:test
type:documentation
type:hardware
type:toolchain
type:ci
```

### Area Labels

```text
area:discovery
area:capability
area:directory
area:supervisor
area:health
area:kernel816
area:tooling
area:documentation
area:ci
```

## Project Board

Project:

```text
ATARIX Development
```

Status values:

```text
Backlog
Ready
In Progress
Review
Blocked
Done
```

The project board tracks workflow state. Milestones track sprint membership. Labels track priority, type, and subsystem.

## Wiki

The wiki is a navigation layer, not the source of truth.

The repository remains authoritative for:

```text
Specifications
ADRs
Headers
Source code
Tests
```

The wiki may summarize project status, link to ADRs, and provide navigation for contributors.

## Repository Governance

All code merged into main should leave CI passing.

Discovery Sprint 1 established the baseline CI process:

```text
bootstrap
configure
build
test
```

Future sprints should maintain a green build and test state.

## Revision History

```text
v1 - Initial project organization structure.
```
