---
document: ATX-100-CH01
title: Purpose and Scope
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-DESIGN-001
  - ATX-DESIGN-002
---

# Chapter 1: Purpose and Scope

## Purpose

Atarix is an architecture-first computing system that combines retro-compatible processor experimentation with modern object, capability, mailbox, fabric, audit, and engineering-management concepts.

The purpose of ATX-100 is to provide the canonical architectural overview of Atarix.

This chapter defines the scope of the architecture manual and explains how it relates to working specifications, reviews, implementation files, tests, and engineering gates.

## System Scope

Atarix includes:

- A native object model.
- Explicit capability-based authority.
- Mailbox-mediated communication.
- Directory and discovery services.
- Fabric-mediated hardware services.
- Compatibility personalities.
- Audit, recovery, lifecycle, and versioning models.
- RTL and software implementation evidence.
- AEMS-managed documentation and traceability.

## Non-Goals

ATX-100 does not replace detailed specifications.

Detailed register layouts, binary packet formats, RTL timing, C API specifics, and test vectors remain in the ATX-SPEC series or implementation-specific documentation.

## Canonical Architecture

ATX-100 owns stable architectural concepts.

A working specification may introduce a concept. After review and verification, stable explanatory material may be integrated into ATX-100. The originating specification remains as implementation detail or is archived for historical traceability.

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/abi-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/abi-v1.md -->
### Integrated source: `docs/abi-v1.md`

# ATARIX ABI v1

## Purpose

This document defines the first draft of the ATARIX application binary interface for firmware, monitor, loader, and early kernel work.

The ABI is intentionally small. It exists to make ROM code, BIOS calls, kernel handoff, and early freestanding C code interoperable before the operating system is complete.

## Scope

This ABI covers:

- W65C816 execution mode
- register assumptions
- stack and direct-page assumptions
- firmware call conventions
- kernel-entry assumptions
- simple error/result conventions

It does not yet define a full userspace ABI.

## Execution Mode

Firmware shall enter the operating-system loader or kernel with the W65C816 in native mode.

Required state at handoff:

- emulation mode cleared
- decimal mode cleared
- interrupts masked unless explicitly documented otherwise
- stack valid
- direct page valid
- data bank register documented
- program bank register naturally set by long jump or long call

## Register Width

Unless a specific entry point says otherwise:

- accumulator: 16-bit
- X/Y index registers: 16-bit
- stack pointer: 16-bit native-mode stack

Code that changes register width must restore the documented ABI width before returning across an ABI boundary.

## Endianness

ATARIX follows W65C816 little-endian memory layout.

Multibyte fields in firmware-visible structures are little-endian unless explicitly marked otherwise.

## Address Model

The ABI distinguishes:

- CPU-local 24-bit addresses visible to the W65C816
- fabric addresses managed by the Fabric Northbridge
- service handles used to avoid hard-coding physical addresses

Early firmware calls should prefer handles and descriptors over raw fabric addresses.

## Stack

The caller owns temporary stack contents.

The callee must preserve stack balance.

Firmware and BIOS calls may use the caller stack for return addresses and small temporaries, but should not assume deep stack availability during early boot.

## Direct Page

Direct page is reserved for the active execution context.

Firmware shall publish the direct-page location in the Boot Information Block at kernel handoff.

Kernel code may relocate direct page after handoff, but must then own all consequences.

## Calling Convention Draft

For early firmware calls:

- A: primary argument or service number
- X: pointer low word or secondary argument
- Y: pointer bank/selector or tertiary argument
- carry clear: success
- carry set: failure
- A on return: result code or error code
- X/Y on return: optional result pointer or value

This convention is deliberately close to monitor/BIOS style firmware calls and easy to inspect with a debugger.

## Register Preservation

Unless otherwise documented:

- callee may clobber A, X, Y, and status flags
- callee must preserve DBR if it changes it
- callee must preserve stack balance
- long-lived service calls must document any additional clobbers

## Error Codes

Initial error-code space:

```text
0x0000  OK
0x0001  ERR_UNSUPPORTED
0x0002  ERR_INVALID_ARGUMENT
0x0003  ERR_NOT_FOUND
0x0004  ERR_BUSY
0x0005  ERR_TIMEOUT
0x0006  ERR_CHECKSUM
0x0007  ERR_PERMISSION
0x0008  ERR_IO
0x0009  ERR_NO_MEMORY
0x000A  ERR_BAD_STATE
```

## Kernel Entry

At kernel entry:

- A should contain the ABI version.
- X/Y should identify the Boot Information Block pointer according to the boot-firmware document.
- carry should be clear.
- stack and direct page must already be valid.

The kernel must validate the Boot Information Block before trusting service pointers.

## Freestanding C Notes

The early C environment should assume:

- no hosted libc
- no operating-system process model
- no implicit heap
- no file descriptors
- no startup runtime beyond explicitly linked reset/startup code

The ABI should be implemented first in assembly stubs, then wrapped by freestanding C headers.

## Versioning

The ABI version is a 16-bit value.

Recommended first value:

```text
0x0001
```

Breaking changes require a new ABI version.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-002-Authority-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-002-Authority-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-002-Authority-Model.md`

# ATX-SPEC-002 Authority Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines authority within Atarix.

Authority is the right to perform an operation on or through an object, resource, service, namespace binding, supervisor function, or fabric facility.

Authority is explicit. Authority is scoped. Authority is auditable. Authority is revocable where possible.

## Core Rule

```text
Identity is not authority.
```

Authority must never be inferred from user identity, object identity, physical location, execution state, network reachability, directory visibility, or implementation detail.

## Authority Sources

Recognized authority sources include:

```text
Object ownership policy
Explicit capabilities
Delegated capabilities
Ring policy
Resource policy
Lifecycle policy
Supervisor-mediated authority
Bootstrap authority
Administrative policy
Recovery policy
```

No source is ambient. Each source must be visible to audit and policy review.

## Authority Domains

### Runtime Authority

Normal authority used by runtime objects, services, applications, drivers, and fabric participants.

Runtime authority is mediated by ring policy, capabilities, lifecycle state, resource policy, and object policy.

### Bootstrap Authority

Narrow authority available before the runtime authority model exists.

Bootstrap authority may initialize minimal hardware, validate firmware, fetch boot data, record boot events, and enter recovery.

Bootstrap authority is not runtime authority and must not silently persist into runtime.

### Supervisor Authority

Authority belonging to the Supervisor Management Fabric.

Supervisor authority controls reset, watchdog, RTC, power, firmware validation, recovery, and fault reporting.

The Operational Fabric may request supervisor action through mediated interfaces but may not directly become supervisor.

### Recovery Authority

Authority used to restore, quarantine, reconcile, or repair system state.

Recovery authority must be explicit, auditable, and constrained by policy.

Recovery must not silently regrant authority.

### Administrative Authority

Authority granted to administrative users, services, or policy domains.

Administrative identity is not itself authority; administrative authority must still be represented by policy and capability state.

## Ownership

Ownership answers:

```text
Who is responsible for this object or resource?
```

Ownership does not automatically answer:

```text
What operations may be performed?
```

Ownership may influence authority but does not replace capabilities, policy, or ring checks.

## Delegation

Delegation transfers or derives authority from an existing authority.

Delegation must:

- Never increase authority beyond the delegator's authority.
- Preserve auditability.
- Preserve scope.
- Preserve lifecycle constraints.
- Preserve resource constraints.
- Preserve ring constraints.
- Be revocable where supported.

Delegation across ring boundaries requires explicit policy.

## Revocation

Revocation removes or invalidates authority.

Revocation may be immediate, deferred, graceful, forced, quarantined, unsupported, or policy-denied depending on the object and resource type.

Revocation must be auditable.

Revocation of authority may require resource cleanup, mailbox closure, directory update, lifecycle transition, or quarantine.

## Authority And Directory

Directory lookup never grants authority.

Directory binding ownership controls name lifecycle.

Object ownership controls object responsibility.

Capability or policy state controls operation authority.

These must not be conflated.

## Authority And Resources

Resource allocation is authority-bearing.

Resource ownership is accountability, not unrestricted control.

Allocation authority, observation authority, accounting authority, revocation authority, and control authority are separate.

## Authority And Audit

Every authority-bearing operation should identify:

```text
Actor identity label
Authority used
Operation
Target object or resource
Policy result
Result
Reason
```

Audit visibility does not imply authority over the audited object.

## Authority And Errors

If authority cannot be verified, the operation must fail closed.

If authority state is unknown after crash, partition, version mismatch, or recovery, affected objects or resources must be restricted or quarantined until reconciled.

## Compatibility Authority

Future POSIX or virtual hardware compatibility environments must receive explicit bounded authority.

Compatibility environments may translate legacy interfaces, but they may not import ambient authority into native Atarix.

## Required Future Work

- Define authority object or authority-record wire format.
- Define ownership transfer semantics.
- Define shared administrative sponsorship.
- Define bootstrap authority in ATX-SPEC-014.
- Define supervisor authority in ATX-SPEC-016.
- Define policy evaluation in ATX-SPEC-013.

## Summary

Authority is not identity, not lookup, not observation, not execution, and not connectivity.

Authority is explicit, bounded, auditable, and policy-mediated.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-007-Namespace-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-007-Namespace-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-007-Namespace-Model.md`

# ATX-SPEC-007 Namespace Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This specification formalizes the Atarix Namespace Model.

The Namespace Model defines human-readable names, paths, bindings, aliases, and namespace structure.

The Namespace Model exists to make the system comprehensible without turning names into authority.

## Core Rule

```text
Lookup is not access.
```

Names help humans and services find objects.

Names do not grant authority.

## Name, Binding, Identity

The core relationship is:

```text
Name
  -> Binding
  -> Object identity
  -> Object
```

A name is not an object.

A binding is not a capability.

An object identity is not authority.

## Namespace Axioms

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Physical location is not identity.
```

## Namespace Layout

Recommended top-level namespaces:

```text
/system
/service
/device
/fabric
/user
/session
/temporary
/audit
```

These are architectural names. Their implementation may vary by platform maturity.

## `/system`

System-owned objects required for node or fabric operation.

System objects are not exempt from ownership, lifecycle, audit, cleanup, scope, or persistence rules.

## `/service`

Service objects and service interfaces.

Examples:

```text
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/directory
```

Service location is not service identity.

## `/device`

Device objects and device-facing abstractions.

## `/fabric`

Fabric-visible nodes, links, memory pools, and fabric services.

Rev A assumes one operational fabric.

Future federated fabrics require explicit specification.

## `/user`

User-associated namespace roots.

User names are labels and administration handles, not authority.

## `/session`

Session-scoped objects and temporary runtime bindings.

Session entries are lease-oriented by default.

## `/temporary`

Temporary objects and bindings.

Temporary entries require explicit cleanup policy.

## `/audit`

Audit-facing objects and audit service interfaces.

Audit visibility is policy-controlled.

Audit visibility does not imply control authority.

## Binding Generation

Bindings should carry generation metadata.

Generation identifies binding evolution, not schema compatibility.

Versioning identifies schema or protocol compatibility.

These concepts must not be conflated.

## Aliases

Aliases provide alternate names.

Aliases must not create authority.

Aliases must not bypass ring, capability, lifecycle, resource, or policy checks.

Alias loops must be detected.

## Rebinding

A name may be rebound only by an authority permitted to modify that binding.

Rebinding must update generation metadata and create an audit event.

Clients requiring stability must pin object identity or binding generation rather than assuming name stability.

## Unnamed Objects

Objects may be unnamed.

Unnamed objects remain subject to authority, lifecycle, resource, audit, and cleanup rules.

## Namespace And Directory Service

The Namespace Model defines naming semantics.

The Directory Service implements name-to-object resolution.

Directory lookup provides discoverability, not authority.

## Namespace And Resources

Resources may be named, but resource visibility does not grant allocation or control authority.

## Namespace And Errors

Malformed paths, stale bindings, alias loops, unknown namespace authority, and unsupported versions are explicit errors.

Authority-bearing operations must fail closed when namespace state cannot be verified.

## Required Future Work

- Define exact path syntax.
- Define allowed character set.
- Define path and component length limits.
- Define canonical path rules.
- Define namespace policy interaction.
- Define federated namespace rules.
- Define version negotiation for namespace records in ATX-SPEC-012.

## Summary

The Namespace Model gives Atarix human-readable structure.

It does not grant authority.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/namespace-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/namespace-model.md -->
### Integrated source: `docs/architecture/namespace-model.md`

# Atarix Namespace Model

## Purpose

The Namespace Model defines how human-readable names resolve to Atarix objects.

Users and services interact with names. The kernel and fabric operate on objects. Hardware operates on physical locations. These concepts must remain separate.

## Core Rule

Names are not objects.

Objects are not names.

Correct model:

```text
name
  -> binding
  -> object identity
  -> object
```

## Identity Hierarchy

Atarix distinguishes:

1. Physical location
2. Object identity
3. Name
4. Path

Physical location is not stable and is not user visible.

Object identity is opaque, immutable, globally unique within the fabric, and never reused.

A name is human-readable and mutable.

## Lookup Rule

Lookup success does not imply access.

Knowing that an object exists does not grant authority to use it.

Access still requires ring validation, capability validation, and object-state validation.

## Namespace Layout

Recommended top-level namespaces:

```text
/
  services/
  nodes/
  users/
  devices/
  system/
  fabric/
```

## Invariants

1. Names are bindings.
2. Names are not identities.
3. Objects may be unnamed.
4. Multiple names may bind to one object.
5. A name may be rebound to another object.
6. Lookup does not imply access.
7. Physical location is never exposed as public namespace identity.

<!-- AEMS-MIGRATED-SOURCE: docs/cpu-card-spec.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/cpu-card-spec.md -->
### Integrated source: `docs/cpu-card-spec.md`

# ATARIX CPU Card Specification

## Purpose

The CPU card architecture separates processor implementation from platform infrastructure.

This permits experimentation with multiple CPU designs while preserving a stable backplane.

## Initial CPU Card

Processor:

- W65C816S

Responsibilities:

- Execute operating-system code
- Manage local execution state
- Interface with system fabric

## CPU Card Interfaces

### Power

Candidate rails:

- +5V
- +3.3V
- Ground

### Clock

Signals:

- System clock
- Clock enable
- Optional debug clock

### Reset

Signals:

- Global reset
- Local reset
- Recovery reset

### Interrupts

Signals:

- IRQ
- NMI
- Fabric-generated events

### Mailboxes

Mailbox channels support:

- Supervisor communication
- Interprocessor communication
- Service requests

## Local Resources

Potential resources:

- Local SRAM
- Debug registers
- Board identification
- Configuration EEPROM

## Multiprocessor Expansion

Future CPU cards may support:

- Dual-CPU configurations
- Shared-memory experiments
- Message-passing experiments

## Design Objectives

- Simplicity
- Serviceability
- Testability
- Stable platform interface
- Forward compatibility

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-001-Architectural-Philosophy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-001-Architectural-Philosophy.md -->
### Integrated source: `docs/design/ATX-DESIGN-001-Architectural-Philosophy.md`

# ATX-DESIGN-001: Architectural Philosophy

## Purpose

This document explains the foundational philosophy behind Atarix.

Atarix is architecture-first. Specifications define behavior, implementations satisfy specifications, and engineering gates verify evidence.

## Core Themes

- Architecture precedes implementation.
- Code is not the architecture.
- Identity is not authority.
- Fabric services are central.
- Human readability is an engineering requirement.
- Engineering evidence is part of the system.

## Relationship To ATX-100

Stable material from this document should be integrated into ATX-100 Part II.

<!-- AEMS-MIGRATED-SOURCE: docs/diagnostic-access-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/diagnostic-access-v1.md -->
### Integrated source: `docs/diagnostic-access-v1.md`

# ATARIX Diagnostic Access v1

## Status

Draft hardware engineering requirement.

This document defines test-point, oscilloscope, logic-analyzer, jumper, LED, and diagnostic-access requirements for ATARIX cards and backplanes.

## Purpose

ATARIX is a staged hardware project. Early revisions must be easy to debug with ordinary bench tools.

Every major board should expose enough diagnostic access to allow bring-up, timing verification, signal-integrity checks, fault isolation, and firmware debugging without destructive probing.

## Design Goals

1. Make clock, reset, bus, interrupt, mailbox, DMA, and fabric activity observable.
2. Support oscilloscope probing without shorting adjacent pins.
3. Support logic-analyzer attachment.
4. Provide ground references close to critical test points.
5. Support single-step or slow-clock operation.
6. Preserve recovery access even when the CPU or fabric is not booting.

## Required Diagnostic Features

Each major board should include:

- Clearly labeled test points.
- Nearby ground test points.
- At least one logic-analyzer header where practical.
- Power-rail measurement points.
- Reset-state visibility.
- Clock visibility.
- Debug LEDs.
- Recovery or mode-selection jumpers/DIP switches where appropriate.

## Backplane Test Points

The backplane should expose test points for:

```text
+12V
+5V
+3.3V
+3.3V Standby
Ground
System Clock
Debug Clock
Global Reset
Slot Reset Lines
Fabric Ready
Supervisor Ready
Recovery Mode
```

The backplane should also expose diagnostic access to:

```text
Interrupt/Event Lines
Mailbox Control Lines
Discovery Bus
Slot Identification Lines
Fabric Transaction Signals
```

## CPU Card Test Points

CPU cards should expose:

```text
CPU Clock
CPU Reset
RDY
IRQ
NMI
ABORT
R/W
VDA
VPA
ML
VPB
A0-A23 sample header or buffered subset
D0-D7 / D0-D15 sample header or buffered subset
ROM Chip Select
RAM Chip Select
I/O Chip Select
Bank Zero Select
Vector Pull Rewrite Active
```

If the full address and data buses cannot be exposed, provide a buffered logic-analyzer header with at least the most useful debug signals.

## Fabric Controller Test Points

The ULX3S/ECP5 fabric-controller connection should expose:

```text
FPGA Clock
FPGA Reset
Configuration Done
Fabric Ready
Fabric Fault
Mailbox Pending
DMA Active
DMA Fault
Interrupt Pending
Discovery Active
Slot Scan Active
```

The fabric controller should also have:

- DIP-switch inputs.
- LED outputs.
- JTAG access.
- UART or console access where practical.

## Supervisor Card Test Points

The supervisor controller should expose:

```text
Supervisor Clock
Supervisor Reset
Power Good Inputs
CPU Reset Output
FPGA Reset Output
Slot Reset Outputs
Watchdog Output
Recovery Input
Fault Log Trigger
UART TX/RX
I2C/SPI Management Bus
```

The supervisor must remain debuggable even when CPU and FPGA cards are held in reset.

## Service and Accelerator Card Test Points

Service and accelerator cards should expose:

```text
Card Power Rails
Local Reset
Fabric Interface Clock
Fabric Interface Reset
Interrupt Output
Mailbox Activity
DMA Request
DMA Grant
Discovery ROM / EEPROM Access
Fault Output
```

Accelerator cards should additionally expose job/queue activity indicators where practical.

## Oscilloscope Probing Requirements

Critical analog and timing points should use test pads or loops large enough for oscilloscope probes.

Recommended points:

```text
Clock Outputs
Reset Edges
Power Rails
FPGA PLL Outputs if accessible
CPU Clock Input
RDY Timing
Interrupt Assertion
Chip Select Timing
```

Provide ground loops or ground pads near high-speed or timing-sensitive points.

## Logic Analyzer Headers

Where practical, use keyed 0.1 inch or 2.54 mm headers for low-speed logic analyzer access.

Recommended CPU-card logic-analyzer groups:

```text
Address Low
Address High
Data Bus
Control Signals
Interrupt Signals
Chip Selects
```

Recommended fabric-controller groups:

```text
Mailbox Signals
Interrupt/Event Signals
DMA Signals
Discovery Signals
Slot Control Signals
```

## LEDs

LEDs should be used for coarse state visibility.

Recommended backplane or fabric LEDs:

```text
Power Good
Supervisor Ready
FPGA Config Done
Fabric Ready
Recovery Mode
CPU Reset Released
DMA Active
Fault
```

Recommended CPU-card LEDs:

```text
CPU Running
ROM Access
RAM Access
I/O Access
IRQ Active
NMI Active
Wait State / RDY Held
```

## Jumpers and DIP Switches

Manual configuration should be available for early bring-up.

Candidate controls:

```text
Recovery Mode
Golden ROM Select
Golden FPGA Bitstream Select
Slow Clock Enable
Single Step Enable
DMA Disable
Fabric Disable
Supervisor Override
Slot Test Mode
```

## Documentation Requirement

Every test point, jumper, DIP switch, LED, and debug header must be documented in the board schematic and in the relevant board-specific document.

Silkscreen labels should match documentation labels.

## Design Rule

If a signal is essential to boot, reset, clocking, interrupt delivery, DMA, mailbox operation, or recovery, it should either be directly testable or observable through a documented diagnostic mechanism.

Debuggability is a first-class hardware requirement, not a cleanup task after layout.

## Open Questions

- Final logic-analyzer header standard.
- Whether to use 0.1 inch headers, Tag-Connect, or both.
- How many LEDs belong on the backplane versus on the ULX3S adapter.
- Whether address/data buses are fully exposed or sampled through FPGA debug registers.
- Probe-loop footprint standard.

<!-- AEMS-MIGRATED-SOURCE: docs/discovery-rom-format-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/discovery-rom-format-v1.md -->
### Integrated source: `docs/discovery-rom-format-v1.md`

# Discovery ROM Format v1

Status: Draft v1

## Purpose

The Discovery ROM provides a platform-independent description of resources, services, instrumentation endpoints, providers, and dependencies offered by a card or subsystem.

The Discovery ROM answers:

```text
What do you provide?
How do clients bind to it?
What does it depend upon?
```

The Discovery ROM does not establish trust, grant authority, or assign permissions.

## Architectural Principles

```text
Discovery describes.
Discovery does not authorize.
Discovery does not establish trust.
Discovery does not grant capability.
```

```text
Address != Handle.
CPU Width != Fabric Width.
```

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Transport operates.
```

## Relationship to Identity EEPROM

Identity EEPROM provides hardware identity.

Example:

```text
Vendor: ATARIX
Board: CPU Card Rev A
Serial: 00000123
```

Discovery ROM provides firmware, resource, and service description.

Example:

```text
Provider: W65C816 Runtime
Version: 1.0
Services:
    CPU Diagnostics
    Mailbox
    Instrumentation
```

Firmware updates may change Discovery ROM contents without changing hardware identity.

## Design Goals

The Discovery ROM shall be:

```text
Architecture-neutral
Versioned
Self-describing
Checksummed
Forward-compatible
CPU-independent
```

## Endianness and Alignment

All numeric fields are little-endian.

All records shall begin on an 8-byte boundary.

Record lengths include the common record header and any padding required to align the next record.

## Discovery Header

All Discovery ROM images begin with a 32-byte Discovery Header.

### Header Layout

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Magic
0x04    2     Version Major
0x06    2     Version Minor
0x08    4     Total Length
0x0C    4     Record Count
0x10    4     Provider ID
0x14    4     Flags
0x18    4     Header CRC32
0x1C    4     Image CRC32
```

### Magic Value

The magic field shall contain:

```text
ATDX
```

ASCII bytes:

```text
0x41 0x54 0x44 0x58
```

Meaning:

```text
ATARIX Discovery
```

### Versioning

```text
Version Major
    Breaking format changes

Version Minor
    Backward-compatible additions
```

Initial version:

```text
Major = 1
Minor = 0
```

### Total Length

Total Length is the byte length of the entire Discovery ROM image, including the Discovery Header, all records, padding, and the END record.

### Record Count

Record Count is the number of records following the Discovery Header, including the END record.

### Provider ID

Provider ID is a 32-bit registry-assigned identifier for the firmware, runtime, or service provider that generated the Discovery ROM.

Provider ID is not derived from the Identity EEPROM.

Identity EEPROM identifies hardware.

Provider ID identifies software, firmware, or runtime providers.

Initial examples:

```text
0x00000001  Supervisor Runtime
0x00000002  Fabric Runtime
0x00000003  W65C816 Runtime
0x00000004  Validation Runtime
0x00000005  Instrumentation Runtime
```

A formal provider registry may be defined later.

### Flags

Initial Discovery Header flags:

```text
Bit 0   Boot required
Bit 1   Runtime provider
Bit 2   Instrumented
Bit 3   CPU resource provider
Bit 4   Hotplug capable
Bit 5   Experimental
Bits 6-31 reserved
```

### Header CRC32

Header CRC32 covers bytes 0x00 through 0x1F of the Discovery Header with the Header CRC32 field treated as zero during calculation.

### Image CRC32

Image CRC32 covers the entire Discovery ROM image with the Image CRC32 field treated as zero during calculation.

CRC algorithm:

```text
CRC-32/ISO-HDLC
```

unless superseded by a later revision.

## Handle Format

Discovery records shall use 64-bit handles.

```text
typedef uint64_t atarix_discovery_handle_t;
```

Handle Encoding v1:

```text
63........48  Provider ID low 16 bits
47........32  Resource or Service Type
31.........0  Instance ID
```

A handle is not an address.

A handle must not be interpreted as a CPU-local address, Fabric address, physical address, IRQ number, or DMA channel.

## Common Record Header

All records begin with an 8-byte common header.

```c
struct discovery_record_header {
    uint16_t type;
    uint16_t version;
    uint32_t length;
};
```

Unknown record types shall be skipped using `length`.

## Record Type Registry

```text
0x0000  RESERVED_RECORD
0x0001  RESOURCE_RECORD
0x0002  SERVICE_RECORD
0x0003  PROVIDER_RECORD
0x0004  DEPENDENCY_RECORD
0x0005  INSTRUMENTATION_RECORD
0xFFFF  END_RECORD
```

Experimental and vendor-specific record range:

```text
0x8000 - 0xFFFE
```

## Resource Records

Resource Records describe platform resources.

Examples:

```text
RESOURCE_MAILBOX
RESOURCE_DMA
RESOURCE_TIMER
RESOURCE_GPIO
RESOURCE_UART
RESOURCE_INTERRUPT_CONTROLLER
RESOURCE_NETWORK
RESOURCE_STORAGE
RESOURCE_FABRIC_MEMORY
RESOURCE_LOGIC_ANALYZER
RESOURCE_OSCILLOSCOPE
RESOURCE_DSP
RESOURCE_PATTERN_GENERATOR
RESOURCE_PROTOCOL_DECODER
RESOURCE_COUNTER_BANK
RESOURCE_TRIGGER_ROUTER
```

### Resource Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    2     Resource Type ID
0x02    2     Resource Version
0x04    4     Resource Flags
0x08    8     Resource Handle
0x10    4     Provider ID
0x14    4     Instance ID
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## Service Records

Service Records describe consumable platform services.

Examples:

```text
SERVICE_DIRECTORY
SERVICE_DISCOVERY
SERVICE_PROVISIONING
SERVICE_VALIDATION
SERVICE_MONITOR
SERVICE_NETWORK_BOOT
SERVICE_INSTRUMENTATION
SERVICE_CAPTURE
SERVICE_ANALYSIS
SERVICE_CPU_DIAGNOSTICS
```

### Service Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    2     Service ID
0x02    2     Service Version
0x04    4     Service Flags
0x08    8     Service Handle
0x10    4     Provider ID
0x14    4     Instance ID
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## Provider Records

Provider Records describe software, firmware, or runtime providers.

Examples:

```text
Supervisor Runtime
Fabric Runtime
W65C816 Runtime
Validation Runtime
Instrumentation Runtime
```

### Provider Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Provider ID
0x04    4     Provider Version
0x08    4     Provider Flags
0x0C    4     Name Length
0x10    N     Provider Name, UTF-8, padded to 8-byte boundary
```

## Dependency Records

Dependency Records describe relationships between resources and services.

Example:

```text
CPU Diagnostics
    depends on
Mailbox
```

### Dependency Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    8     Consumer Handle
0x08    8     Provider Handle
0x10    2     Dependency Type
0x12    2     Dependency Version
0x14    4     Dependency Flags
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## Instrumentation Records

Instrumentation Records advertise observability and testbench services.

Examples:

```text
Logic Analyzer
Protocol Decoder
Pattern Generator
Counter Bank
DSP Analysis
Trigger Router
```

### Instrumentation Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    2     Instrumentation Type
0x02    2     Instrumentation Version
0x04    4     Instrumentation Flags
0x08    8     Instrumentation Handle
0x10    8     Observed Handle
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## END Record

The END record terminates the record stream.

It uses the common record header with:

```text
Type    = 0xFFFF
Version = 0x0001
Length  = 8
```

## Forbidden Contents

Discovery records shall not expose:

```text
Physical addresses
CPU-local addresses
IRQ numbers
DMA channels
Board-specific implementation details
```

The platform binding layer resolves handles into implementation-specific resources.

## Enumeration Flow

Recommended enumeration sequence:

```text
Power On
    ↓
Supervisor Validation
    ↓
Identity Validation
    ↓
Discovery Header Validation
    ↓
Record Enumeration
    ↓
Dependency Validation
    ↓
Capability Evaluation
    ↓
Service Binding
    ↓
Transport Activation
```

## Error Handling

The enumerator shall:

```text
Reject invalid CRC
Reject invalid length
Reject invalid version
Skip unknown record types where possible
Continue enumeration where practical
Avoid trusting discovery data until identity validation succeeds
```

## Future Extensions

Future versions may add:

```text
Capability references
Health model references
Security metadata
Network service metadata
Hotplug metadata
Signed discovery images
```

## Related Documents

- identity-eeprom-v1.md
- cpu-observability-contract-v1.md
- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- ADR-THREE-FABRIC-ARCHITECTURE.md
- ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md

<!-- AEMS-MIGRATED-SOURCE: docs/fpga-instrumentation-testbench-services-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/fpga-instrumentation-testbench-services-v1.md -->
### Integrated source: `docs/fpga-instrumentation-testbench-services-v1.md`

# FPGA Instrumentation and Testbench Services v1

## Purpose

This document defines the role of FPGA-assisted instrumentation and electronic testbench services in ATARIX.

The Fabric FPGA is not only a transport device. It should also provide active measurement, stimulus, capture, and protocol-analysis services that are accessible through the Supervisor / Management Fabric and exposed to engineering tools.

## Core Principle

```text
Measurement must not depend on the thing being measured.
```

The CPU being tested must not be required to prove that the CPU is working.

The Fabric FPGA and supervisor together form an independent electronic testbench.

## Relationship to the Three Fabrics

ATARIX uses three architectural fabrics:

```text
Management Fabric
Enumeration Fabric
Transport Fabric
```

Instrumentation services attach primarily to the Management Fabric, with FPGA capture and stimulus engines implemented near the Transport and Enumeration interfaces.

Conceptually:

```text
RP2350 Supervisor
    ↓
Management Fabric
    ↓
FPGA Instrumentation Services
    ↓
Logic analyzer / trigger / pattern / counter / DSP blocks
    ↓
Observed buses, clocks, rails, cards, and fabric links
```

## Service Classes

### Logic Analyzer Service

Captures digital signals, bus transactions, mailbox traffic, fabric events, and selected card-side signals.

Expected features:

- configurable probe groups
- trigger conditions
- pre-trigger capture
- post-trigger capture
- timestamped samples
- exportable traces

### Oscilloscope / Sampler Service

Where analog front-end hardware exists, the system should support oscilloscope-like capture.

Expected uses:

- rail ripple observation
- clock quality observation
- signal-integrity bring-up
- slow analog sensor capture

This service may require external analog front-end circuitry and is not assumed to be purely FPGA-internal.

### DSP Analysis Service

The FPGA may provide DSP-assisted analysis for captured data.

Examples:

- frequency measurement
- jitter estimation
- pulse-width measurement
- edge timing
- simple spectral analysis
- threshold crossing analysis

### Pattern Generator Service

Generates controlled digital stimulus for bring-up and validation.

Examples:

- bus stimulus
- mailbox stimulus
- card-presence simulation
- trigger output patterns
- deterministic test vectors

### Protocol Decoder Service

Decodes platform protocols for engineering visibility.

Initial targets:

- Identity EEPROM sideband traffic
- Discovery ROM access
- mailbox protocol
- supervisor sideband protocol
- fabric transaction summaries

### Counter and Profiler Service

Provides hardware counters for:

- mailbox traffic
- DMA requests
- interrupt events
- timeout events
- fabric stalls
- discovery failures
- identity failures
- watchdog events

### Trigger Router Service

Routes trigger events between:

- engineering panel trigger input
- engineering panel trigger output
- logic analyzer
- pattern generator
- supervisor event log
- fabric counters

## Supervisor Integration

The supervisor should expose instrumentation commands through the engineering console.

Recommended commands:

```text
show instruments
show probes
show counters
capture start <profile>
capture stop
capture status
capture export
trigger arm <source>
trigger fire
pattern load <profile>
pattern start
pattern stop
```

The exact command syntax is not frozen by this document.

## Observability Without the CPU

The instrumentation system must be usable when:

- CPU is held in reset
- CPU has crashed
- Transport Fabric is partially degraded
- operating system is unavailable
- bootloader is unavailable

This preserves the principle that the measurement system does not depend on the thing being measured.

## Engineering Panel Integration

The engineering panel should provide physical access to instrumentation signals.

Recommended interfaces:

```text
BNC clock output
BNC trigger input
BNC trigger output
logic analyzer header
power test jacks
USB-C supervisor console
status LEDs
```

## Discovery and Enumeration

Instrumentation blocks should be discoverable through the Enumeration Fabric as services, but their safety authority remains under the Management Fabric.

A CPU or OS may consume instrumentation data, but the supervisor must retain independent access.

## Safety and Policy

Pattern generation and stimulus services can affect hardware state and must be policy-controlled.

Default policy:

```text
Factory / engineering mode:
    full instrumentation access permitted by supervisor policy

Normal mode:
    passive capture and counters preferred

Homebrew / restricted cards:
    no privileged stimulus unless explicitly authorized
```

## Implementation Targets

Future implementation locations:

```text
src/emulator/fabric/instrumentation.*
src/emulator/fabric/trigger.*
src/emulator/fabric/counters.*
include/atarix/instrumentation.h
```

## Relationship to Other Documents

Related documents:

- supervisor-observability-contract-v1.md
- timing-observability-v1.md
- cpu-test-suite-integration-v1.md
- future instrumentation-service-v1.md
- future fabric-csr-map-v1.md

## Non-Goals

This document does not define:

- final FPGA HDL module layout
- ADC selection
- oscilloscope analog front-end design
- final engineering panel mechanical design
- final CSR addresses

Those belong in lower-level hardware and Fabric CSR specifications.

<!-- AEMS-MIGRATED-SOURCE: docs/gnu-dev-tools-setup.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/gnu-dev-tools-setup.md -->
### Integrated source: `docs/gnu-dev-tools-setup.md`

# GNU Development Tools Setup

Status: Draft v1

## Purpose

This document defines the baseline GNU-style development environment for ATARIX.

ATARIX should build cleanly on a conventional Unix-like system using standard GNU development tools.

## Baseline Host Tools

Required for normal C development:

```text
GNU Make
GCC or Clang with C11 support
binutils
pkg-config
Git
```

Recommended for debugging and inspection:

```text
GDB
objdump
readelf
nm
addr2line
hexdump or xxd
```

Recommended for generated build systems:

```text
Autoconf
Automake
Libtool
m4
```

Recommended for static checks:

```text
cppcheck
clang-format
clang-tidy
shellcheck
```

Optional but useful:

```text
bison
flex
gperf
doxygen
graphviz
lcov
gcov
```

## Debian / Ubuntu Package Set

```sh
sudo apt update
sudo apt install \
    build-essential \
    gcc \
    g++ \
    make \
    binutils \
    gdb \
    git \
    pkg-config \
    autoconf \
    automake \
    libtool \
    m4 \
    bison \
    flex \
    cppcheck \
    clang-format \
    clang-tidy \
    shellcheck \
    doxygen \
    graphviz \
    lcov
```

## Fedora Package Set

```sh
sudo dnf install \
    gcc \
    gcc-c++ \
    make \
    binutils \
    gdb \
    git \
    pkgconf-pkg-config \
    autoconf \
    automake \
    libtool \
    m4 \
    bison \
    flex \
    cppcheck \
    clang-tools-extra \
    ShellCheck \
    doxygen \
    graphviz \
    lcov
```

## macOS Package Set

Using Homebrew:

```sh
brew install \
    make \
    gcc \
    binutils \
    gdb \
    autoconf \
    automake \
    libtool \
    m4 \
    bison \
    flex \
    cppcheck \
    clang-format \
    shellcheck \
    doxygen \
    graphviz \
    lcov
```

macOS may use Apple Clang for host tools and Homebrew GCC for stricter GNU compatibility testing.

## Expected Repository Workflow

The repository should support this development flow:

```sh
./bootstrap
./configure
make
make check
make distcheck
```

Early development may temporarily use direct compiler commands, but the long-term goal is a GNU-style build system.

## Current Direct Build Examples

Build the Discovery fixture generator:

```sh
cc -std=c11 -Wall -Wextra -Werror -Iinclude \
    tools/mkdiscovery/mkdiscovery.c \
    -o mkdiscovery
```

Generate valid Discovery fixtures:

```sh
mkdir -p tests/reference
./mkdiscovery --profile minimal --output tests/reference/minimal.discovery
./mkdiscovery --profile cpu-card --output tests/reference/cpu_card.discovery
./mkdiscovery --profile supervisor --output tests/reference/supervisor.discovery
./mkdiscovery --profile instrumentation --output tests/reference/instrumentation.discovery
```

Build a Discovery test manually:

```sh
cc -std=c11 -Wall -Wextra -Werror -Iinclude \
    tests/discovery/test_discovery_header.c \
    src/discovery/discovery_database.c \
    src/discovery/discovery_iterator.c \
    src/discovery/discovery_validate.c \
    src/discovery/discovery_parser.c \
    -o test_discovery_header
```

Run it:

```sh
./test_discovery_header
```

## Planned GNU Build Files

Planned files:

```text
bootstrap
configure.ac
Makefile.am
src/Makefile.am
tests/Makefile.am
tools/Makefile.am
m4/
```

## Compiler Policy

Default language standard:

```text
C11
```

Default warning policy:

```text
-Wall -Wextra -Werror
```

Recommended development flags:

```text
-g -O0 -fsanitize=address,undefined
```

Recommended release flags:

```text
-O2
```

## Cross-Compilation Direction

ATARIX host tools should build on the development host.

Firmware and target-specific code should eventually support cross-compilation using explicit toolchain prefixes:

```sh
./configure --host=<target-triplet>
```

Potential future targets:

```text
65C816 toolchain
RP2350 firmware toolchain
ECP5 FPGA toolchain
RISC-V cross toolchain
m68k cross toolchain
PowerPC cross toolchain
```

## Policy

The GNU toolchain setup exists to make ATARIX reproducible, testable, and portable.

New C modules should avoid hidden IDE assumptions.

Every runtime subsystem should eventually be reachable through:

```sh
make check
```

## Related Documents

- discovery-rom-format-v1.md
- resource-type-registry-v1.md
- service-id-registry-v1.md
- operation-id-registry-v1.md
- capability-record-format-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/identity-eeprom-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/identity-eeprom-v1.md -->
### Integrated source: `docs/identity-eeprom-v1.md`

# Identity EEPROM v1

## Purpose

The ATARIX Identity EEPROM is the root of hardware identity for every card in the system.

It answers:

```text
Who am I?
What am I?
Who built me?
How was I provisioned?
Can my identity record be validated?
How much trust should the platform assign me?
```

The Identity EEPROM is mandatory on every ATARIX card.

A board shall not be considered operational until its identity source has been validated by the supervisor.

## Core Principle

```text
Identity proves existence.
Trust determines authority.
Capabilities determine access.
```

Identity, trust, and capabilities are separate architectural concepts.

A valid EEPROM proves that a card can identify itself. It does not automatically grant dangerous capabilities such as DMA, boot authority, recovery authority, supervisor authority, or memory-controller authority.

## Scope

This specification defines:

- required physical identity storage
- required binary identity block
- validation rules
- manufacturing and provisioning fields
- trust-class model
- homebrew card support
- required observability signals
- required engineering test points
- recovery behavior

Detailed service capabilities are defined by the future Capability Record Format specification.

## Physical Device

Rev A should use a simple nonvolatile serial EEPROM.

Recommended class:

```text
I2C EEPROM
24AA256 / 24LC256 / AT24C256 or equivalent
Minimum capacity: 32 KiB
```

Rationale:

- inexpensive
- widely available
- simple to read from supervisor firmware
- large enough for future metadata
- adequate for provisioning records, signatures, and service history extensions

## Endianness and Data Model

All numeric fields are little-endian and follow `docs/data-model-and-endianness-v1.md`.

Unqualified addresses are forbidden in this specification.

Identity EEPROM records may contain handles or identifiers, but the mandatory identity block does not contain CPU-local addresses or Fabric addresses.

## Mandatory Identity Block

The first 512 bytes of the EEPROM are reserved for the mandatory identity block.

Future extension blocks may follow after the first 512 bytes.

## Binary Layout

```text
Offset  Size  Field
------  ----  --------------------------------
0x0000  4     Magic
0x0004  2     EEPROM format version
0x0006  2     Header length
0x0008  4     Total record length
0x000C  4     Vendor ID
0x0010  4     Device ID
0x0014  8     Serial number
0x001C  2     Board revision
0x001E  2     Board class
0x0020  2     Trust class
0x0022  2     Reserved / alignment
0x0024  8     Manufacture timestamp
0x002C  4     Boot firmware revision
0x0030  4     Provisioning tool revision
0x0034  8     Capability flags
0x003C  32    Board name
0x005C  4     Provisioning revision
0x0060  8     Provisioning timestamp
0x0068  8     Factory test signature
0x0070  4     CRC32
0x0074  396   Reserved for v1-compatible expansion
```

Total mandatory block size:

```text
512 bytes
```

## Magic

The magic field shall contain:

```text
ATIX
```

ASCII bytes:

```text
0x41 0x54 0x49 0x58
```

## Format Version

Initial version:

```text
0x0001
```

Breaking changes require a new format version.

Compatible extensions should use reserved space or extension blocks after the mandatory 512-byte identity block.

## Header Length

For v1:

```text
0x0200
```

This is the length of the mandatory identity block in bytes.

## Total Record Length

The total record length describes the total identity record area used in the EEPROM.

For a v1 record with no extensions:

```text
0x00000200
```

## Vendor ID

Vendor IDs are 32-bit identifiers.

Reserved values:

```text
0x00000000  Invalid / unprogrammed
0x00000001  ATARIX reserved
0xFFF00000-0xFFFFFFFF  Homebrew / experimental range
```

The homebrew range allows user-built cards without requiring official vendor allocation.

## Device ID

Device IDs are 32-bit vendor-assigned identifiers.

A Device ID is meaningful only within the Vendor ID namespace.

## Serial Number

Serial numbers are 64-bit vendor-assigned values.

Homebrew cards may use:

- user-assigned serial numbers
- generated serial numbers
- locally administered serial numbers

Serial numbers should be unique within a Vendor ID namespace.

## Board Revision

Board revision is a 16-bit value.

Recommended convention:

```text
major << 8 | minor
```

Example:

```text
0x0100  Rev 1.0
0x0101  Rev 1.1
```

## Board Class

Initial board classes:

```text
0x0000  Invalid / unknown
0x0001  CPU card
0x0002  Memory card
0x0003  Network card
0x0004  Storage card
0x0005  Supervisor card
0x0006  Backplane controller
0x0007  Development card
0x0008  Homebrew card
0x0009  Prototype card
0x000A  Unprovisioned card
0xFFFF  Vendor-specific
```

Board class describes what the board is.

Board class does not define what the board is authorized to do.

## Trust Class

Trust class describes the initial trust level assigned by provisioning policy.

Initial trust classes:

```text
0x0000  Invalid
0x0001  Unknown
0x0002  Homebrew
0x0003  Development
0x0004  Factory
0x0005  Platform critical
0xFFFF  Vendor-specific
```

Trust class is not a capability grant.

The platform may reduce effective trust based on policy, signature failure, missing provisioning records, development-mode switches, or supervisor configuration.

## Manufacture Timestamp

Manufacture timestamp is a 64-bit unsigned integer.

Recommended representation:

```text
microseconds since Unix epoch
```

If unavailable, set to zero.

## Boot Firmware Revision

Boot firmware revision identifies firmware associated with the card at provisioning time.

Recommended convention:

```text
major << 24 | minor << 16 | patch
```

## Provisioning Tool Revision

Provisioning tool revision identifies the tool version that wrote or last validated the identity record.

This is intentionally separate from boot firmware revision.

## Capability Flags

Capability flags are a compact summary of broad capability classes.

Detailed capabilities are described by Capability Records.

Initial bit assignments:

```text
Bit 0   Mailbox endpoint present
Bit 1   Discovery ROM present
Bit 2   DMA-capable hardware present
Bit 3   Interrupt-capable hardware present
Bit 4   Boot-capable hardware present
Bit 5   Recovery-capable hardware present
Bit 6   Supervisor-facing hardware present
Bit 7   Fabric memory participant
Bit 8   Network-capable hardware present
Bit 9   Storage-capable hardware present
Bit 10  Instrumentation-capable hardware present
Bit 11  Development / experimental hardware
Bits 12-63 reserved
```

A set bit advertises that hardware exists. It does not grant authority.

Authority is granted by policy and capability evaluation.

## Board Name

Board name is a fixed 32-byte UTF-8 field.

Rules:

- NUL-terminated if shorter than 32 bytes.
- Not required to be unique.
- Intended for engineering consoles and human-readable inventory.

Example:

```text
ATARIX Rev A CPU Card
```

## Provisioning Revision

Provisioning revision identifies the provisioning schema or process revision.

## Provisioning Timestamp

Provisioning timestamp is a 64-bit unsigned integer.

Recommended representation:

```text
microseconds since Unix epoch
```

If unavailable, set to zero.

## Factory Test Signature

Factory test signature is a 64-bit value written after manufacturing validation.

It may be a simple test signature in Rev A and may later become a handle or digest associated with a richer factory-test record.

Recommended initial states:

```text
0x0000000000000000  Not tested / unknown
0x4154495854455354  ATIXTEST / factory test passed marker
```

## CRC32

CRC32 validates the mandatory identity block.

Rules:

- CRC field is treated as zero during calculation.
- CRC covers bytes `0x0000` through `0x01FF` of the mandatory identity block.
- Algorithm: CRC-32/ISO-HDLC unless superseded by a later revision.

## Validation Chain

A card is considered operational only if all of the following succeed:

```text
Physical presence
Communication
Header validation
CRC validation
Identity validation
Policy evaluation
```

Supervisor validation states:

```text
ID_PRESENT
ID_READABLE
ID_HEADER_VALID
ID_CRC_VALID
ID_IDENTITY_VALID
ID_POLICY_ACCEPTED
ID_VALID
ID_ERROR
```

`ID_VALID` shall be true only when the supervisor has validated the identity source and policy accepts the card for the current operating mode.

## Supervisor Responsibilities

The supervisor shall:

1. Detect EEPROM physical presence where hardware permits.
2. Read the mandatory identity block.
3. Validate magic and format version.
4. Validate header length and total record length.
5. Validate CRC32.
6. Validate Vendor ID, Device ID, Board Class, and Trust Class against policy.
7. Record validation state.
8. Expose validation state through the engineering console.
9. Expose validation state to the Fabric / Backplane BIOS as appropriate.
10. Drive identity status signals and visual indication.

The CPU shall not be the only entity capable of validating identity.

## CPU Reset Policy

For platform-critical boards, the supervisor may hold CPU reset if identity validation fails.

Recommended default:

```text
Factory / Platform Critical failure:
    hold CPU reset or enter safe mode

Development / Homebrew failure:
    enter engineering or restricted mode

Unprovisioned card:
    enter provisioning mode if allowed
```

## Required Signals

Every ATARIX card shall expose or report the following logical identity states:

```text
ID_PRESENT
ID_VALID
ID_ERROR
```

These may be physical sideband signals, supervisor GPIOs, Fabric-readable status bits, or a combination of these mechanisms.

The signal source must not depend solely on the main CPU.

## Required Test Points

Rev A hardware should expose:

```text
TP_ID_SCL
TP_ID_SDA
TP_ID_PWR
TP_ID_GND
TP_ID_VALID
```

These test points allow validation with:

- DMM
- oscilloscope
- logic analyzer
- Bus Pirate-class tool
- RP2350 diagnostics

The engineering goal is that identity communication and validation can be observed without disassembling the system beyond normal service access.

## Visual Status Indicator

Identity status shall be visible to an engineer.

Recommended supervisor-controlled indication:

```text
OFF     No EEPROM detected
GREEN   Valid factory / trusted identity
BLUE    Valid homebrew identity
AMBER   Valid but restricted or warning state
RED     Invalid identity or communication failure
```

The CPU shall not be the sole controller of this indicator.

## Homebrew Card Support

Homebrew cards are supported as first-class citizens.

Homebrew support is not implicit trust.

A homebrew card may participate in the system if:

```text
EEPROM present
EEPROM readable
CRC valid
Board class is homebrew, prototype, or development
Capability flags are explicit
Policy allows the card in the current mode
```

Default restrictions for homebrew cards:

```text
DMA disabled
Boot authority disabled
Recovery authority disabled
Supervisor authority disabled
Memory-controller authority disabled
Interrupt-controller authority disabled unless explicitly granted
```

Homebrew cards may expose safer services such as:

- GPIO
- serial
- LEDs
- sensors
- simple mailbox endpoints
- instrumentation outputs

Privileged access requires explicit provisioning and policy approval.

## Development Mode

ATARIX hardware may provide a development-mode jumper or switch.

Example names:

```text
JP_DEV_MODE
SW_DEV_ENABLE
```

When asserted, policy may allow:

- homebrew cards
- experimental EEPROMs
- verbose diagnostics
- relaxed provisioning requirements

Development mode must not silently grant unrestricted DMA, supervisor authority, boot authority, or recovery authority.

## Engineering Console Requirements

The engineering console should expose identity status.

Recommended command:

```text
show identity
```

Example output:

```text
Slot: 3
Board: Homebrew Sensor Card
Vendor ID: 0xFFF01234
Device ID: 0x00000001
Serial: 0x0000000000000001
Board Class: Homebrew
Trust Class: Homebrew
Identity: VALID
Policy: RESTRICTED
Restrictions:
    DMA disabled
    Boot disabled
    Supervisor access disabled
```

## Recovery Behavior

Recovery must not depend on Directory Services.

If identity validation fails, the supervisor and Backplane BIOS must still provide enough diagnostic access to determine:

- whether the EEPROM is missing
- whether communication failed
- whether CRC failed
- whether identity was rejected by policy
- whether provisioning is required

## Relationship to Discovery ROM

Identity EEPROM and Discovery ROM are related but distinct.

Identity EEPROM answers:

```text
Who are you?
What board are you?
How were you provisioned?
How much should I initially trust you?
```

Discovery ROM answers:

```text
What resources and services do you expose?
How should software bind to them?
```

A valid Identity EEPROM is required before Discovery ROM information is trusted.

## Relationship to Capability Records

Capability flags in the EEPROM are only a compact summary.

Detailed capabilities belong in Capability Records.

A hardware capability does not imply authorization to use that capability.

## Future Expansion

The mandatory 512-byte block reserves space for v1-compatible extension pointers or future fields.

Future extension blocks may include:

- certificate block
- cryptographic signature
- public key
- asset tags
- calibration data
- manufacturing notes
- service history
- repair history
- signed capability grants

Rev A does not require cryptographic trust.

The format reserves room for it.

## Implementation Targets

This specification should produce:

```text
include/atarix/identity_eeprom.h
tests/eeprom/
src/emulator/discovery/identity_eeprom.*
src/firmware/monitor/eeprom.*
```

## Non-Goals

This specification does not define:

- Discovery ROM binary format
- Capability Record binary format
- Service Directory format
- cryptographic signing policy
- official vendor allocation process

Those are separate specifications.

<!-- AEMS-MIGRATED-SOURCE: docs/management-anomaly-detection-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/management-anomaly-detection-v1.md -->
### Integrated source: `docs/management-anomaly-detection-v1.md`

# ATARIX Management Anomaly Detection v1

## Status

Draft management-plane architecture specification.

This document defines an advisory anomaly-detection layer for the ATARIX supervisor and management fabric. The intent is to help distinguish normal hardware behavior from abnormal behavior so watchdog and recovery actions are more appropriate.

## Purpose

A simple watchdog can detect that something stopped responding.

A management anomaly detector can help answer a better question:

```text
Is this behavior normal for this card, state, workload, and boot phase?
```

The system should use this information to improve fault classification, reset policy, and diagnostic logging.

## Design Rule

Anomaly detection is advisory.

It may influence watchdog policy, reset escalation, and fault classification, but it must not be the only mechanism that decides to reset hardware.

Hard safety limits remain deterministic.

## Design Goals

1. Learn normal heartbeat behavior.
2. Detect abnormal timing, voltage, temperature, reset, DMA, and fabric activity.
3. Improve watchdog reset decisions.
4. Reduce unnecessary resets during slow but valid operations.
5. Escalate faster when behavior clearly indicates a hang or unsafe condition.
6. Keep the algorithm simple enough for supervisor MCU implementation.
7. Preserve deterministic recovery paths.

## Inputs

Candidate management-plane inputs:

```text
CPU heartbeat interval
FPGA heartbeat interval
Slot heartbeat interval
Boot phase
Reset reason
Power rail state
Temperature readings
DMA active duration
Mailbox queue depth
Interrupt rate
Fabric fault status
Network boot status
RTC/NTP sync state
Slot identity
Card revision
Firmware version
```

## Bayesian Filtering Concept

A Bayesian or Bayesian-inspired filter may maintain probability estimates for states such as:

```text
Normal
Booting
Slow But Valid
Hung
Power Fault
Thermal Fault
DMA Fault
Fabric Fault
Communication Fault
Unknown
```

The model estimates the most likely state based on observed evidence.

Example:

```text
Long CPU heartbeat gap
+ DMA inactive
+ no mailbox progress
+ power good
+ same boot phase for too long
= likely CPU hang
```

Another example:

```text
Long CPU heartbeat gap
+ netboot active
+ network receive activity
+ DMA active within limit
= likely slow but valid boot activity
```

## Deterministic Limits

Some conditions should bypass probabilistic reasoning.

Examples:

```text
Power rail out of tolerance
Over-temperature limit exceeded
DMA active beyond hard timeout
Fabric fault asserted
Supervisor watchdog failure
Manual recovery request
```

These conditions should trigger predefined recovery actions.

## State Confidence

The anomaly detector should maintain a confidence value.

Candidate confidence bands:

```text
0-25%   Unknown / insufficient evidence
26-60%  Weak indication
61-85%  Probable
86-100% Strong indication
```

Recovery actions should require both:

```text
Observed fault condition
Policy threshold met
```

## Watchdog Policy Integration

The watchdog should support escalation levels:

```text
Level 0 Observe only
Level 1 Log anomaly
Level 2 Warn / assert soft fault
Level 3 Reset affected service
Level 4 Reset affected slot
Level 5 Reset fabric or CPU
Level 6 Enter recovery mode
```

The anomaly detector may modify escalation timing.

Examples:

```text
High confidence CPU hang -> escalate faster
Low confidence slow boot -> delay reset
Repeated same-slot fault -> escalate to slot quarantine
Power instability -> avoid repeated reboot loop
```

## Normal Behavior Profiles

The supervisor may maintain per-card profiles.

Profile fields:

```text
Card type
Card revision
Firmware version
Expected heartbeat interval
Expected boot duration
Expected idle temperature range
Expected active temperature range
Expected DMA maximum duration
Expected mailbox response range
Fault history
```

Profiles should be updateable but not trusted blindly.

## Learning Mode

A supervised learning mode may record normal behavior during known-good operation.

Learning mode should record:

```text
Boot duration
Heartbeat intervals
Normal interrupt rates
Normal DMA durations
Normal temperature range
Normal voltage range
Mailbox response times
```

Learning mode must be explicitly enabled.

The system should not silently relearn during fault conditions.

## Fault Classification

Fault classes:

```text
CPU Hang
Fabric Hang
DMA Timeout
Mailbox Timeout
Thermal Fault
Power Fault
Network Boot Fault
Clock Fault
Reset Loop
Slot Fault
Unknown Fault
```

Fault logs should include:

```text
Timestamp
Card identity
Boot phase
Observed values
Estimated state
Confidence
Action taken
Previous related faults
```

## Reset Correctness

The purpose of the filter is not just to reset hardware, but to reset the correct hardware.

Examples:

```text
Slot heartbeat missing but fabric healthy -> reset slot
Fabric heartbeat missing and multiple slots silent -> reset fabric
CPU heartbeat missing but supervisor and fabric healthy -> reset CPU card
Power rail unstable -> hold affected slots in reset
Repeated accelerator faults -> quarantine accelerator slot
```

## Quarantine Mode

Repeated faults may place a card or slot into quarantine.

Quarantine behavior:

```text
Hold slot in reset
Disable DMA grants
Disable accelerator execution
Mark slot unsafe in discovery table
Require manual or supervisor-authorized re-enable
```

## Implementation Notes

Rev A implementation may begin with a simple scoring system rather than a full Bayesian model.

Example scoring model:

```text
+3 missed heartbeat
+2 stuck boot phase
+2 DMA active too long
+1 mailbox queue not draining
-2 network boot active
-1 recent valid interrupt
-1 temperature normal
```

Later revisions may replace this with a formal Bayesian filter.

## Supervisor Resource Limits

The algorithm must be small enough for the supervisor MCU.

Avoid:

```text
Large dynamic memory allocation
Floating-point dependency
Unbounded history buffers
Opaque model behavior
```

Prefer:

```text
Integer counters
Fixed-size history windows
Explicit thresholds
Small probability tables
Explainable fault decisions
```

## Operator Visibility

The supervisor diagnostic console should expose:

```text
SHOW HEALTH
SHOW ANOMALIES
SHOW PROFILE <slot>
SHOW WATCHDOG
SHOW FAULTS
CLEAR PROFILE <slot>
LEARN PROFILE <slot>
QUARANTINE SLOT <slot>
RELEASE SLOT <slot>
```

## Design Principle

The management fabric should learn what normal looks like, but recovery must remain explainable and deterministic.

The system should never hide behind a mysterious model when resetting hardware.

## Open Questions

- Whether Rev A uses scoring only or a small Bayesian table.
- Where profiles are stored.
- Whether learned profiles are signed or checksummed.
- Whether anomaly state is exposed through the discovery system.
- Whether the FPGA fabric contributes hardware counters directly.
- Whether quarantine state persists across reboot.
- How manual override interacts with quarantine and recovery mode.

<!-- AEMS-MIGRATED-SOURCE: docs/supervisor-observability-contract-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/supervisor-observability-contract-v1.md -->
### Integrated source: `docs/supervisor-observability-contract-v1.md`

# Supervisor Observability Contract v1

## Purpose

This document defines the minimum observability, health, and safety responsibilities of the ATARIX supervisor for cards, backplane services, and early platform bring-up.

The supervisor is an independent witness. It must be able to observe and validate the platform even if the CPU is held in reset, stalled, or unavailable.

## Core Principles

```text
Measurement must not depend on the thing being measured.
Observability Is A Feature.
Recovery Is Mandatory.
Engineering Access Is A First-Class Feature.
Scientific Method Over Assumption.
```

## Supervisor Role

The supervisor should be able to answer:

```text
Is the card present?
Is the card powered correctly?
Is the card electrically healthy?
Is the card thermally safe?
Is the card clocked correctly?
Is the identity source valid?
Is the discovery source valid?
Is the card ready for normal participation?
If not, why?
```

## Minimum Card Health Checks

Each supervised card should expose enough telemetry for the supervisor to validate the following categories.

### Identity Health

Required checks:

- Identity EEPROM present
- Identity EEPROM readable
- Identity EEPROM CRC valid
- Identity policy accepted
- ID_PRESENT / ID_VALID / ID_ERROR state available

### Discovery Health

Required checks:

- Discovery ROM present, if advertised
- Discovery ROM readable
- Discovery header valid
- Discovery CRC valid
- Discovery record count sane
- Discovery policy accepted

Recommended logical states:

```text
DISCOVERY_PRESENT
DISCOVERY_VALID
DISCOVERY_ERROR
```

### Electrical Rail Health

Required checks where hardware supports measurement:

- card input voltage present
- local regulator output valid
- over-voltage not detected
- under-voltage not detected
- brownout not detected
- over-current not detected
- rail sequencing completed
- power-good asserted

Recommended monitored rails:

```text
VIN_BACKPLANE
V5_CARD
V3P3_CARD
V2P5_CARD
V1P8_CARD
V1P2_CARD
VCORE_CARD
VBAT_OR_RTC
```

Actual rail set depends on card class.

### Current and Power Health

Recommended checks:

- input current
- per-rail current where practical
- inrush event count
- over-current latch state
- fuse or eFuse state
- power budget class
- card power enable state

### Thermal Health

Required checks where practical:

- board temperature
- regulator temperature
- FPGA temperature, if present
- CPU temperature, if measurable
- memory temperature, if present
- thermal warning threshold
- thermal critical threshold

Recommended states:

```text
THERMAL_OK
THERMAL_WARN
THERMAL_CRITICAL
```

### Clock and Timing Health

Required checks where practical:

- reference clock present
- fabric clock present
- CPU clock present
- clock frequency within tolerance
- missing-clock fault state
- PLL lock state
- reset release timing
- monotonic timer alive

### Reset and Watchdog Health

Required checks:

- reset asserted / deasserted state
- reset reason
- watchdog enabled
- watchdog timeout count
- last watchdog event
- CPU reset hold reason
- supervisor safe-mode reason

### Communication Health

Required checks:

- supervisor link to card alive
- mailbox link alive, if advertised
- fabric link alive, if present
- I2C / SPI sideband bus health
- timeout count
- bus fault detection where practical
- transaction error count

## Health State Model

Each card should have a supervisor-visible health state.

Recommended states:

```text
ABSENT
PRESENT_UNVALIDATED
IDENTITY_VALID
DISCOVERY_VALID
RESTRICTED
READY
WARNING
FAULTED
SAFE_MODE
RECOVERY_MODE
```

A card may be identified and still restricted.

A card may be electrically present and still unsafe.

A card may be valid but not authorized for the current operating mode.

## Supervisor Validation Sequence

Recommended sequence:

```text
Power applied
    ↓
Card presence detect
    ↓
Rail sanity check
    ↓
Clock sanity check
    ↓
Identity EEPROM validation
    ↓
Discovery ROM validation
    ↓
Thermal sanity check
    ↓
Policy evaluation
    ↓
Normal participation allowed or restricted
```

Failure at any stage should produce:

- supervisor event log entry
- health state transition
- visible indication
- engineering console report
- recovery or restricted-mode state where appropriate

## Event Log Requirements

The supervisor should maintain an event log with at least:

```text
timestamp_us
slot_or_card_id
event_class
stage
result
error_code
raw_status
```

Example:

```text
00001234 us  SLOT 3  POWER       V3P3        PASS
00001241 us  SLOT 3  IDENTITY    CRC32       PASS
00001251 us  SLOT 3  DISCOVERY   HEADER      PASS
00001280 us  SLOT 3  POLICY      HOMEBREW    RESTRICTED
```

The timestamp should use the Fabric Monotonic Timer model where available.

## Engineering Console Commands

Recommended supervisor console commands:

```text
show card <slot>
show health <slot>
show identity <slot>
show discovery <slot>
show rails <slot>
show thermals <slot>
show clocks <slot>
show faults <slot>
test identity <slot>
test discovery <slot>
test mailbox <slot>
test rails <slot>
test watchdog <slot>
acknowledge faults <slot>
```

## Physical Observability

Cards should expose test points sufficient for independent validation.

Identity test points:

```text
TP_ID_SCL
TP_ID_SDA
TP_ID_PWR
TP_ID_GND
TP_ID_VALID
```

Power test points:

```text
TP_VIN
TP_5V
TP_3V3
TP_2V5
TP_1V8
TP_1V2
TP_VCORE
TP_GND
```

Clock / timing test points, where applicable:

```text
TP_REFCLK
TP_CPUCLK
TP_FABCLK
TP_TRIGGER_IN
TP_TRIGGER_OUT
```

## Visual Status Indication

Supervisor-controlled visual indication should exist for:

- identity state
- card health state
- fault state
- activity state

Recommended identity colors:

```text
OFF     not present
GREEN   valid and accepted
BLUE    valid homebrew
AMBER   valid but restricted / warning
RED     invalid or failed
```

Recommended health colors:

```text
GREEN   ready
AMBER   warning or restricted
RED     faulted
PURPLE  recovery / provisioning mode
```

The CPU shall not be the sole controller of safety or identity indicators.

## Safety Policy

The supervisor should prevent normal participation when card health checks fail.

Recommended failure handling:

```text
Rail fault:
    enter faulted or restricted state

Thermal critical:
    enter faulted or safe state

Clock fault:
    enter reset-hold or faulted state

Identity fault:
    platform-critical cards enter safe state; development cards enter engineering/provisioning state

Discovery fault:
    restrict service publication; do not grant capabilities

Mailbox fault:
    isolate endpoint in policy; log timeout
```

## Relationship to Identity EEPROM

The Identity EEPROM specification defines the identity record.

This document defines how the supervisor observes, validates, reports, and acts on identity state.

## Relationship to Discovery ROM

Discovery ROMs describe resources and services.

The supervisor should validate discovery data before the card participates in normal service enumeration.

Discovery failure must not prevent recovery diagnostics.

## Relationship to Instrumentation Service

The Instrumentation Service should expose supervisor-observed health, counters, and fault records to higher-level software.

Higher-level software may consume supervisor telemetry, but it must not be required for basic measurement or recovery.

## Non-Goals

This document does not define:

- exact ADC part numbers
- exact voltage thresholds
- exact thermal thresholds
- full engineering panel mechanical layout
- final Fabric CSR mapping

Those belong in card-specific hardware specifications and Fabric CSR specifications.

<!-- AEMS-MIGRATED-SOURCE: docs/timing-observability-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/timing-observability-v1.md -->
### Integrated source: `docs/timing-observability-v1.md`

# ATARIX Timing and Observability Architecture v1

## Status

Draft architecture specification.

This document defines ATARIX timing domains, monotonic counters, RTC usage, watchdog timing, event counters, measurement timestamps, and correlation rules for diagnostics and performance analysis.

## Purpose

ATARIX must be diagnosable from the first hardware revision.

Timing and observability are not optional conveniences. They are part of the platform architecture.

The system must be able to answer:

```text
When did it happen?
How long did it take?
Why was it slow?
Did forward progress stop?
What subsystem failed first?
```

## Design Rule

RTC answers:

```text
When did it happen?
```

Monotonic timers answer:

```text
How long did it take?
```

Counters answer:

```text
Why was it slow?
```

No subsystem shall use RTC time for scheduling, timeout, watchdog, or performance calculations.

## Timing Domains

ATARIX defines four timing domains:

```text
RTC Time
Fabric Monotonic Time
CPU Cycle or Instruction Counters
Subsystem Event Counters
```

Each domain has a distinct purpose.

## RTC Time

RTC time is human-readable wall-clock time.

Uses:

```text
Fault log wall-clock timestamps
Maintenance records
Audit records
User-visible date and time
NTP synchronization state
```

RTC time may be corrected by:

```text
NTP
Manual setting
RTC battery replacement
Supervisor policy
```

Because RTC time can move forward or backward, it must not be used for watchdogs, deadlines, scheduling, latency measurement, or performance profiling.

## Fabric Monotonic Timer

The ECP5 fabric controller should expose a global monotonic timer.

Recommended Rev A format:

```text
u64 monotonic counter
1 MHz frequency
1 tick = 1 microsecond
```

Properties:

```text
Never runs backward while powered
Wrap time effectively irrelevant for practical operation
Readable by CPU firmware
Readable by supervisor where practical
Usable by fabric services
Usable by diagnostics
```

At 1 MHz, a 64-bit counter wraps after roughly 584,000 years.

## Relationship to Linux Jiffies

The ATARIX monotonic timer is conceptually similar to Linux jiffies because it provides an elapsed-time counter.

However, it should not be a scheduler-tick counter.

Instead, it should be a fixed-rate microsecond-scale monotonic counter.

This avoids ambiguity caused by variable tick rates.

## Fabric Timer CSR Model

The fabric CSR map should expose timer registers similar to:

```text
TIMER_LOW       Lower 32 bits of monotonic counter
TIMER_HIGH      Upper 32 bits of monotonic counter
TIMER_FREQ      Ticks per second, recommended 1000000
TIMER_SNAPSHOT  Optional latch / snapshot control
```

Multi-byte and multi-register access rules must follow:

```text
docs/data-model-and-endianness.md
docs/register-map-v1.md
```

64-bit reads should support snapshot semantics so software can read a stable value on an 8/16-bit CPU.

## CPU Counters

CPU cards may expose local counters.

Examples:

```text
CPU cycle counter
Instruction retire counter, if available
Interrupt entry counter
Wait-state counter
RDY stall counter
Fabric-access stall counter
```

For W65C816 Rev A, some counters may be implemented by CPU-card bridge logic rather than the CPU itself.

CPU counters are local performance counters, not the global system time base.

## Event Counters

Every major subsystem should expose event counters where practical.

Examples:

```text
Mailbox messages sent
Mailbox messages received
Mailbox errors
DMA descriptors submitted
DMA descriptors completed
DMA bytes moved
DMA faults
Interrupts delivered
Interrupts dropped
Discovery reads
Capability lookups
Capability failures
Network packets
Netboot retries
Sensor samples
Watchdog warnings
Slot resets
```

Counters should be wide enough to avoid frequent wrap.

Recommended width:

```text
u64 for long-running counters
u32 for small local counters where wrap is acceptable and documented
```

## Measurement Timestamping

All measurements, diagnostics, health records, traces, and fault logs shall support monotonic timestamps.

A measurement record should include:

```text
Source
Measurement type
Value
Unit
Monotonic timestamp
Optional RTC timestamp
Slot or resource ID
Context, if available
```

Example:

```text
Source:       Fluke 289
Measurement:  +5V rail
Value:        5.013 V
Tick:         123456789
Slot:         CPU-0
Context:      Netboot stage 2
```

The monotonic timestamp is the primary correlation key.

The RTC timestamp is for human interpretation.

## Watchdog Timing

Watchdogs should use monotonic time.

Each monitored component should expose:

```text
Heartbeat counter
Last progress tick
Current state
Last fault tick
Reset count
Fault count
```

Supervisor logic should evaluate:

```text
current_monotonic_tick - last_progress_tick
```

rather than comparing RTC time.

A component should be considered suspicious when heartbeat continues but forward progress stops.

## Forward Progress

Heartbeat is not enough.

A card or service can be alive but wedged.

Therefore, every significant subsystem should distinguish:

```text
Alive
Making progress
Idle by design
Busy
Degraded
Faulted
Timed out
Quarantined
```

Forward progress indicators may include:

```text
Completed transactions
Advanced queue pointer
Updated state machine
Processed interrupt
Completed DMA descriptor
Responded to mailbox command
```

## Instrumentation Timing

The engineering instrumentation path should timestamp samples with monotonic time where practical.

This applies to:

```text
Voltage samples
Current samples
Temperature samples
Fan tachometer samples
Bluetooth instrument samples
Logic analyzer trigger events
Scope trigger events
Pattern generator events
Fault captures
```

Measurements collected through RP2350 supervisor paths and ECP5 fabric paths should be correlated to a shared monotonic timeline where practical.

## RTC and NTP Interaction

NTP may update RTC time after network synchronization.

NTP must not alter monotonic time.

NTP must not reset watchdog deadlines.

NTP must not affect elapsed-time measurements.

The supervisor should record time state:

```text
Unknown
RTC only
NTP synchronized
Manually set
Recovered from fault log
```

## Precision Timing Expansion

Future revisions should reserve architectural room for:

```text
PPS input
10 MHz reference input
10 MHz reference output
GPSDO integration
IEEE-1588 PTP
External trigger input
External trigger output
```

These features are not required for Rev A, but the architecture should not preclude them.

## Engineering Panel Integration

Engineering-panel signals may include:

```text
Clock output
Trigger input
Trigger output
Logic analyzer trigger
Supervisor console timestamp command
Measurement capture control
```

All engineering-panel capture events should be timestamped where practical.

## Monitor Commands

Suggested monitor commands:

```text
TIME SHOW
TIMER SHOW
TIMER READ
COUNTERS SHOW
HEALTH SHOW
WATCHDOG SHOW
TRACE SHOW
MEASURE SHOW
LATENCY SHOW
```

## Testing Requirements

Testing should verify:

```text
Monotonic timer increments
Monotonic timer does not move backward
Stable 64-bit timer reads on W65C816
RTC adjustment does not affect monotonic timer
Watchdog uses monotonic time
Event counters increment correctly
Counter wrap behavior is documented
Measurement records include timestamps
Trace correlation works across subsystems
```

See:

```text
docs/testing-strategy.md
```

## Design Principle

A timestamped failure is easier to debug than an unobserved failure.

A subsystem that cannot report health, progress, latency, faults, and recovery status is incomplete.

Correlating events across subsystems is a primary design goal.

<!-- AEMS-MIGRATED-SOURCE: docs/toolchain-and-build-strategy-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/toolchain-and-build-strategy-v1.md -->
### Integrated source: `docs/toolchain-and-build-strategy-v1.md`

# Toolchain and Build Strategy v1

## Purpose

This document records the current ATARIX development-toolchain direction for the W65C816 operating-system, firmware, monitor, BIOS, and bring-up environment.

The guiding constraint is that ATARIX should not require running Linux on the target W65C816 system. Linux, BSD, macOS, or other modern hosts may be used as cross-development machines, but the target should remain an ATARIX workstation architecture with its own firmware, loader, service model, and operating-system path.

## Development Model

ATARIX uses a cross-development model:

```text
Host workstation
    -> editor / build tools / emulator / FPGA tools
    -> ROM, monitor, firmware, kernel, test images
    -> programmer, serial link, netboot, storage image, or FPGA bridge
    -> ATARIX target hardware
```

The target is not expected to self-host at Rev A. Self-hosting is a long-term goal, not a prerequisite for hardware bring-up.

## No-Linux-on-Target Policy

The W65C816 target should not depend on Linux as its operating environment.

This means:

- Do not require a Linux kernel on the W65C816 side.
- Do not design the firmware around Linux device-tree boot assumptions.
- Do not require POSIX semantics during early bring-up.
- Do not require ELF loading at the first ROM-monitor stage.
- Keep target firmware small, observable, and deterministic.

Linux remains useful as a host-side build and test environment.

## Host Build Environment

Recommended host-side tools:

- Vim or another plain-text editor
- Make
- Git
- Python for build glue, image construction, and test harnesses
- 64TASS for practical W65C816 assembly
- WDC tools as a vendor-reference option
- Verilator or other HDL simulation tooling where practical
- Yosys / nextpnr / ECP5 FPGA tooling for ULX3S bring-up
- Serial-console tools
- ROM-image and disk-image construction utilities

Autotools-style components such as m4, autoconf, automake, bison, flex, and binutils may be useful for building host-side tools, but they should not be a prerequisite for the target firmware itself.

## Assembler Direction

### 64TASS

64TASS is the practical near-term assembler choice for W65C816 firmware and monitor development.

Use it for:

- Reset vectors
- CPU initialization
- ROM monitor
- BIOS entry stubs
- Interrupt/vector tables
- Early kernel stubs
- Hardware diagnostics

### WDC Toolchain

The WDC toolchain should be kept as a compatibility and reference path.

Use it to compare:

- Instruction encoding
- Calling conventions
- Object layout
- Vendor examples
- Datasheet-adjacent code expectations

It should not be the only path unless the project deliberately chooses a vendor-locked workflow.

## C and libc Direction

ATARIX should treat C support as staged.

### Stage 0: No libc

Early ROM and monitor code should not depend on libc.

Required primitives can be implemented directly:

- byte copy
- byte fill
- string length
- simple compare
- checksum
- hexadecimal formatting
- serial output
- panic / halt

### Stage 1: freestanding C subset

Once the monitor and BIOS interfaces stabilize, introduce a freestanding C subset.

Possible conventions:

- `-ffreestanding` style assumptions for host-side compilers where applicable
- no malloc requirement
- no file I/O requirement
- no process model
- no POSIX dependency
- explicit memory-map and service-directory access

### Stage 2: ATARIX libc

A native ATARIX libc can be introduced after the kernel has stable services.

Initial libc scope:

- fixed-width integer types
- memory and string functions
- formatted output subset
- error codes
- service-call wrappers
- block I/O wrappers
- console I/O wrappers

Deferred libc scope:

- full POSIX compatibility
- dynamic linking
- fork/exec model
- Unix filesystem assumptions
- threads

## Object and Image Formats

Early bring-up should prefer simple binary formats over complex loaders.

Recommended progression:

1. Raw ROM image
2. ROM monitor command images
3. Flat kernel image with ATARIX kernel header
4. Relocatable module format
5. Optional ELF tooling bridge later

The firmware should remain capable of loading a simple flat image even after richer formats exist.

## Emulator Strategy

An emulator should be built early enough to test firmware, monitor commands, service-directory construction, and kernel handoff before hardware is complete.

Minimum useful emulator features:

- W65C816 reset and native-mode transition
- ROM image loading
- local SRAM model
- serial console model
- simple Fabric Northbridge mailbox model
- service-directory fixture injection
- deterministic trace output

The emulator does not need to model the full ECP5 fabric at first. It should model the architectural contract seen by firmware and kernel code.

## FPGA / ULX3S Integration

The ULX3S / ECP5 path should be treated as a hardware-development fixture for the Fabric Northbridge.

Host-side FPGA work should produce:

- simulated mailbox endpoints
- fabric-register definitions
- discovery ROM test fixtures
- service-directory test fixtures
- logic-analyzer-friendly debug points
- firmware-visible revision and capability registers

The software build should be able to generate matching firmware headers and FPGA register manifests from a single source of truth where practical.

## Build Order

The project should avoid waiting for perfect hardware before writing software, and avoid writing an entire OS before hardware reality exists.

Recommended order:

1. Define the firmware-visible register and mailbox contracts.
2. Write minimal ROM reset and serial monitor code.
3. Build an emulator harness for the monitor and BIOS contracts.
4. Implement a Fabric Northbridge stub in simulation.
5. Bring up the ULX3S / ECP5 fabric service skeleton.
6. Add discovery ROM parsing and service-directory construction.
7. Add raw block or network boot loading.
8. Add a freestanding kernel stub.
9. Expand into driver framework, capability broker, and DMA manager.

This keeps the chicken-and-egg problem under control: write enough software to test the board, and build enough board/fabric behavior to constrain the software.

## Working Rule

For Rev A, success is not a complete operating system. Success is a reproducible path from reset to monitor, from monitor to discovered services, and from discovered services to a loaded kernel image.

<!-- AEMS-MIGRATED-SOURCE: docs/vega816-analysis.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/vega816-analysis.md -->
### Integrated source: `docs/vega816-analysis.md`

# Vega816 Hardware Migration Analysis

## Source

The Vega816 KiCad archive was reviewed as a reference design for ATARIX hardware planning.

The archive contains:

```text
Vega816.kicad_pro
Vega816.kicad_sch
Vega816.kicad_pcb
Vega816.kicad_sym
CPU Shim/
CPU Buffer/
DMA Controller/
Sysctrl Registers/
VectorPullRewrite/
Footprints.pretty/
```

## Purpose of This Analysis

Vega816 should be treated as a Rev 0 reference design for several low-level W65C816 hardware concepts.

It should not be copied wholesale into ATARIX because ATARIX is moving toward a modular workstation architecture with a structured fabric interface rather than a raw shared CPU bus.

## Vega816 Design Summary

The Vega816 design explores symmetric multiprocessing concepts for W65C816 systems, including:

- CPU shims.
- CPU buffering.
- DMA arbitration.
- Per-CPU bank-zero isolation.
- Multi-channel DMA.
- Vector Pull Rewrite.
- IRQ vector expansion.
- BB816-compatible connectors.

The design assumes a hardware environment closer to a shared-bus multiprocessor experiment than a managed workstation fabric.

## Subproject Review

### CPU Shim

Useful ideas:

- CPU-local adaptation layer.
- DMA request hooks.
- A22/A23-based DMA channel selection.
- Optional address rewrite.
- RDY-based CPU pausing.
- Vector Pull Rewrite integration.
- IRQ0B through IRQ7B interrupt concept.

ATARIX migration:

The CPU Shim concept maps well onto an ATARIX CPU-card bridge.

The exact BB816 connector dependency should not be retained as the long-term ATARIX interface.

### CPU Buffer

Useful ideas:

- Explicit CPU bus buffering.
- Direction-controlled data transceiver.
- DMAB-style bus isolation.
- Separation between CPU-side and DMA-side connectors.
- Use of standard logic for debug visibility.

ATARIX migration:

The buffering strategy is valuable for the Rev A CPU card and for debugging CPU-local timing.

The long-term backplane should not expose the entire raw CPU bus.

### DMA Controller

Useful ideas:

- Multiple DMA channels.
- Multiple CPU requestors.
- Per-CPU home DMA channel.
- Bank-zero isolation.
- Arbitration based on valid-address behavior.
- Cross-mapping of DMA channels between CPUs.

ATARIX migration:

The concept of per-CPU home memory and bank-zero isolation is important and should be preserved.

The direct shared-DMA wiring model should be replaced by fabric-mediated DMA and explicit capability grants.

See:

```text
docs/dma-engine-v1.md
docs/capability-model.md
```

### Vector Pull Rewrite

Useful ideas:

- Hardware recognition of W65C816 vector pulls.
- Expansion of IRQ vector handling.
- Multiple prioritized IRQ inputs.
- Native-mode vector rewrite behavior.

ATARIX migration:

This is one of the strongest Vega816 ideas and should be retained as an optional CPU-card feature.

It should be integrated with the ATARIX interrupt architecture rather than treated as a separate one-off circuit.

See:

```text
docs/interrupt-architecture-v1.md
```

### Sysctrl Registers

Useful ideas:

- Register-oriented control architecture.
- System-control register blocks.
- Hardware/software boundary definition.

ATARIX migration:

The concept is compatible with the ATARIX register map, but the register definitions should be normalized into the ATARIX fabric register model.

See:

```text
docs/register-map-v1.md
```

## Keep

The following Vega816 concepts should be kept:

1. CPU-local buffering.
2. CPU shim / bridge layer.
3. Vector Pull Rewrite.
4. Multiple prioritized IRQ inputs.
5. RDY-based wait-state control.
6. Per-CPU bank-zero isolation.
7. Explicit DMA request and grant behavior.
8. Observable discrete-logic bring-up path.

## Modify

The following concepts should be modified for ATARIX:

1. BB816-specific connector model.
2. Direct raw-bus expansion.
3. DMA channel mapping.
4. Multi-CPU arbitration.
5. System-control register layout.
6. Interrupt ownership model.

## Discard or Avoid

The following should not be carried directly into ATARIX:

1. Exposing the full raw W65C816 bus across the system backplane.
2. Treating DMA as an implicit bus right.
3. Treating multi-CPU sharing as primarily a wiring problem.
4. Assuming every participant sees every low-level CPU signal.

## ATARIX Migration Path

Recommended migration:

```text
Vega816 CPU Shim
    -> ATARIX CPU-card local bridge

Vega816 CPU Buffer
    -> ATARIX Rev A CPU-local buffering

Vega816 DMA Controller
    -> ATARIX FPGA-mediated DMA service

Vega816 Vector Pull Rewrite
    -> ATARIX CPU-card interrupt assist

Vega816 Sysctrl Registers
    -> ATARIX fabric register map
```

## Proposed ATARIX CPU Card Model

```text
W65C816
    |
Local SRAM / ROM
    |
CPU-local buffer and shim logic
    |
CPU-card FPGA or CPLD bridge
    |
ATARIX fabric interface
    |
DIN41612 backplane
```

## Architectural Conclusion

Vega816 is highly valuable as a source of low-level W65C816 hardware techniques.

Its strongest contribution to ATARIX is not the entire shared-bus architecture, but the recognition that a W65C816 multiprocessor or modular system needs:

- CPU-local mediation.
- Bus isolation.
- Wait-state control.
- Interrupt assist logic.
- Bank-zero awareness.
- Explicit DMA arbitration.

ATARIX should preserve those lessons while moving the system bus toward a Sun/NuBus-inspired fabric architecture.
