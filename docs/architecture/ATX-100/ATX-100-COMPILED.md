# ATX-100: Compiled Atarix Architecture Book

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


---

# Chapter 2: Historical Context

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md -->
### Integrated source: `docs/ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md`

# ADR: Discoverable, Observable, Testable, Instrumentable, Recoverable

Status: Accepted

## Context

Complex systems become difficult to maintain when subsystems cannot be located, measured, validated, diagnosed, or recovered independently.

Historically, many computer systems assumed that successful operation was sufficient evidence of correctness.

ATARIX is intended to support:

- long service life
- multiple processor architectures
- homebrew hardware
- experimental hardware
- integrated instrumentation
- self-validation
- recovery without a functioning operating system

A formal architectural doctrine is required.

## Decision

Every ATARIX subsystem shall be:

```text
Discoverable
Observable
Testable
Instrumentable
Recoverable
```

This doctrine applies to hardware, firmware, fabric services, supervisor services, and operating-system-visible services.

## Discoverable

A subsystem should be identifiable and enumerable.

Questions answered:

```text
What is it?
Where is it?
What version is it?
What services does it provide?
```

Examples:

- Identity EEPROM
- Discovery records
- Resource records
- Service records

A subsystem that cannot be discovered is difficult to manage and support.

## Observable

A subsystem should expose measurable state.

Questions answered:

```text
Is it healthy?
What is it doing?
What faults have occurred?
```

Examples:

- health state
- counters
- fault records
- telemetry
- event logs

Observation should not depend solely on the subsystem itself.

## Testable

A subsystem should support validation.

Questions answered:

```text
Does it function correctly?
Does it meet specification?
Can faults be reproduced?
```

Examples:

- validation profiles
- self-test mechanisms
- supervisor-assisted testing
- manufacturing validation

## Instrumentable

A subsystem should support measurement and analysis.

Questions answered:

```text
Can behavior be measured?
Can traffic be captured?
Can timing be characterized?
```

Examples:

- logic analyzer services
- protocol decoders
- counters
- trigger systems
- DSP analysis
- engineering test points

Instrumentation should be available independently of the operating system where practical.

## Recoverable

A subsystem should support diagnosis and restoration after faults.

Questions answered:

```text
Can it be repaired?
Can it be reprovisioned?
Can it be isolated?
Can diagnostics still operate after failure?
```

Examples:

- provisioning mode
- recovery mode
- supervisor diagnostics
- fault logging
- restricted mode operation

Recovery should not depend on successful operation of the failed subsystem.

## Relationship to Supervisor

The supervisor is the primary enforcement mechanism for this doctrine.

The supervisor should maintain visibility into platform state even when:

- CPUs are held in reset
- operating systems are unavailable
- services are degraded
- hardware faults are present

## Relationship to Instrumentation

Instrumentation is considered a first-class platform feature.

Examples:

```text
Logic Analyzer
Protocol Decoder
Pattern Generator
Counter Bank
DSP Analysis
Trigger Router
```

Instrumentation exists to support observability, testing, and recovery.

## Relationship to Enumeration

The Enumeration Fabric should make discoverable:

- resources
- services
- instrumentation endpoints
- capability descriptions
- version information

Enumeration is the primary mechanism for discoverability.

## Review Checklist

Every new specification should answer:

```text
How is it discovered?
How is it observed?
How is it tested?
How is it instrumented?
How is it recovered?
```

If one of these questions cannot be answered, the specification should justify why.

## Architectural Benefits

This doctrine:

- improves maintainability
- improves diagnosability
- improves validation quality
- improves field supportability
- supports long-term evolution
- reduces hidden assumptions
- encourages measurable engineering decisions

## Related ADRs

- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- ADR-THREE-FABRIC-ARCHITECTURE.md

## Guiding Principle

```text
Measurement must not depend on the thing being measured.
```

## Summary

```text
Every subsystem shall be:

Discoverable
Observable
Testable
Instrumentable
Recoverable
```

This doctrine is a core architectural requirement of ATARIX.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0001-ulx3s-selection.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0001-ulx3s-selection.md -->
### Integrated source: `docs/adr/ADR-0001-ulx3s-selection.md`

# ADR-0001: ULX3S LFE5U-85F Selection

Status: Accepted

## Context
Need a fabric controller platform.

## Decision
Use ULX3S with LFE5U-85F.

## Evidence
- Open toolchain support
- Large FPGA resource budget
- Existing community support
- Suitable for rapid prototyping

## Alternatives Considered
- Custom FPGA board
- Smaller ECP5 variants

## Consequences
Faster development and lower initial risk.

## Future Revisions
May migrate to custom ATARIX fabric controller card.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0003-local-cpu-bus.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0003-local-cpu-bus.md -->
### Integrated source: `docs/adr/ADR-0003-local-cpu-bus.md`

# ADR-0003: Local CPU Bus

Status: Accepted

## Context
Should the W65C816 bus be exposed across the backplane?

## Decision
No. The CPU bus remains local to the CPU card.

## Evidence
- Improved signal integrity
- Better scalability
- Easier SMP evolution
- Cleaner DMA architecture

## Alternatives Considered
- Shared raw bus
- VME-style architecture
- Direct backplane extension

## Consequences
Requires bridge logic and fabric interfaces.

## Future Revisions
Revisit only if evidence supports a shared-bus design.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0009-observability-first.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0009-observability-first.md -->
### Integrated source: `docs/adr/ADR-0009-observability-first.md`

# ADR-0009: Observability First

Status: Accepted

Date: 2026-06-10

## Context

Complex systems fail in unexpected ways. Debugging a system after failure is dramatically easier when visibility is designed into the architecture.

## Decision

Observability is a first-class architectural requirement.

Every major subsystem should expose status, counters, diagnostics, health information, fault history, and recovery information whenever practical.

## Evidence

- Faster hardware bring-up.
- Reduced debugging time.
- Better fault isolation.
- Easier performance characterization.
- Supports evidence-based engineering.

## Alternatives Considered

- Minimal diagnostics.
- Debugging only through external instruments.
- Diagnostics added later.

## Consequences

Pros:
- Easier troubleshooting.
- Better operational insight.
- Better long-term maintainability.

Cons:
- Additional design effort.
- Additional registers and counters.

## Future Revisions

May expand to include trace buffers, fabric analyzers, and automated health reporting.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0010-evidence-based-engineering.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0010-evidence-based-engineering.md -->
### Integrated source: `docs/adr/ADR-0010-evidence-based-engineering.md`

# ADR-0010: Evidence-Based Engineering

Status: Accepted

Date: 2026-06-10

## Context

Engineering decisions are often made from intuition, habit, or preference. ATARIX aims to favor measurable and reproducible evidence whenever practical.

## Decision

Adopt evidence-based engineering as a project-wide principle.

Measure first.
Decide second.
Document always.

## Evidence

Scientific and engineering disciplines consistently produce better outcomes when hypotheses, experiments, measurements, and conclusions are documented and reproducible.

## Alternatives Considered

- Opinion-driven design.
- Authority-driven design.
- Trend-driven design.

## Consequences

Pros:
- Better decisions.
- Reduced redesign risk.
- Improved reproducibility.
- Better ADR quality.

Cons:
- Additional documentation effort.
- Additional testing effort.

## Future Revisions

Experiment records and benchmark repositories should eventually become part of the project workflow.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0011-little-endian-data-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0011-little-endian-data-model.md -->
### Integrated source: `docs/adr/ADR-0011-little-endian-data-model.md`

# ADR-0011: Little-Endian Data Model

Status: Accepted

Date: 2026-06-10

## Context

ATARIX begins with a W65C816 CPU but is expected to grow into a larger fabric-based workstation architecture with memory services, accelerators, large identifiers, and long-lived persistent resources.

The project requires a consistent binary representation.

## Decision

All architecture-defined binary structures use little-endian encoding.

The architecture may use 8-bit, 16-bit, 24-bit, 32-bit, and 64-bit fields as appropriate.

CPU width is not system width.

## Evidence

- Natural fit with W65C816 memory representation.
- Simplifies firmware implementation.
- Simplifies protocol definition.
- Avoids mixed-endian ambiguity.
- Supports future 64-bit identifiers and timestamps.

## Alternatives Considered

- Big-endian architecture.
- Mixed-endian structures.
- 16-bit-only architecture.
- 32-bit-only architecture.
- 64-bit-only architecture.

## Consequences

Pros:
- Consistent binary representation.
- Future-proof identifiers and timestamps.
- Easier protocol evolution.

Cons:
- Requires explicit rules for large values on an 8/16-bit CPU.
- Requires documentation of multi-byte register access semantics.

## Future Revisions

Future formats may define 128-bit identifiers if required, but the little-endian rule should remain stable.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/README.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/README.md -->
### Integrated source: `docs/adr/README.md`

# Architecture Decision Records

ADR documents capture significant architectural decisions.

## Required Sections

- Title
- Status
- Date
- Context
- Decision
- Evidence
- Alternatives Considered
- Consequences
- Future Revisions

## Engineering Rule

Major architectural decisions should be supported by evidence whenever practical.

Measure first. Decide second. Document always.

## Numbering

ADR-0001
ADR-0002
ADR-0003
...


---

# Chapter 3: Documentation Taxonomy

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/engineering/Documentation-Taxonomy-and-Style-Guide.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/engineering/Documentation-Taxonomy-and-Style-Guide.md -->
### Integrated source: `docs/engineering/Documentation-Taxonomy-and-Style-Guide.md`

# Documentation Taxonomy and Style Guide

## Purpose

This guide defines the initial documentation taxonomy, naming conventions, and lifecycle rules for Atarix documentation.

## Document Classes

Atarix uses these document classes:

- ATX-100: Master architecture reference.
- ATX-DESIGN: Design philosophy and guiding principles.
- ATX-SPEC: Normative architecture and implementation requirements.
- ATX-REVIEW: Architecture review records and reconciliation findings.
- ATX-ENG: Engineering process, gate, tooling, and release documentation.
- Archive: Historical frozen documents retained for traceability.

## Lifecycle States

Documents may use these lifecycle states:

```text
Concept
Draft
Reviewed
Verified
Integrated
Archived
```

## Metadata

Every major document SHOULD include a YAML front matter block with:

- document
- title
- type
- status
- lifecycle
- version
- canonical_owner

## Generated Material

Generated material belongs under docs/generated or an AEMS output directory.

Generated material must identify its source command and input files.

## Archival Rule

Superseded documents are not deleted. They are archived and linked to their canonical successor.

<!-- AEMS-MIGRATED-SOURCE: docs/roadmap.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/roadmap.md -->
### Integrated source: `docs/roadmap.md`

# ATARIX Roadmap

## Phase 1 - Rev A

Objectives:

- Build first hardware
- Validate W65C816 operation
- Establish monitor environment
- Verify memory subsystem

## Phase 2 - Rev B

Objectives:

- Modularization
- Recovery systems
- Networking
- Backplane planning

## Phase 3 - Rev C

Objectives:

- FPGA chipset services
- Interrupt routing
- Mailboxes
- DMA foundations

## Phase 4 - Rev D

Objectives:

- Multiprocessor experimentation
- Resource mediation
- Security architecture

## Phase 5 - Rev E

Objectives:

- Accelerator framework
- Capability broker
- Heterogeneous compute support

## Documentation Goals

- Hardware specifications
- Register maps
- Firmware interfaces
- Security model
- Software architecture

## Long-Term Vision

Create a durable experimental workstation platform capable of supporting operating-system research, FPGA development, secure computing experimentation, and heterogeneous compute research.


---

# Chapter 4: Architecture Before Implementation

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/architecture-review-checklist.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/architecture-review-checklist.md -->
### Integrated source: `docs/architecture/architecture-review-checklist.md`

# Atarix Architecture Review Checklist

## Purpose

This checklist is used before implementing or merging any Atarix subsystem change.

It exists to prevent implementation details from accidentally defining architecture.

## Core Checks

- ARC-001: Physical location exposure
- ARC-002: Internal implementation exposure
- ARC-003: Name and identity separation
- ARC-004: Ring boundary enforcement
- ARC-005: Lookup is not access
- ARC-006: Capability and ring interaction
- ARC-007: Single-node viability
- ARC-008: Many-node viability
- ARC-009: Object mobility
- ARC-010: Unnamed object support
- ARC-011: Name rebinding
- ARC-012: Human-readable interface
- ARC-013: Scriptability without hardware knowledge
- ARC-014: CPU assumption leakage
- ARC-015: Future compatibility
- ARC-016: Contract vs implementation
- ARC-017: CPU-card replaceability
- ARC-018: Fabric first
- ARC-019: No undocumented special cases
- ARC-020: Replaceability
- ARC-021: Cleanup and resource hygiene

## Required Review Questions

Every subsystem proposal must answer:

```text
Affected Pillars:
Modified Pillars:
Relevant ARC Checks:
Known Exceptions:
Required Tests:
```

If a foundational pillar is modified, architecture review is required before implementation.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/architecture-review-template.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/architecture-review-template.md -->
### Integrated source: `docs/reviews/architecture-review-template.md`

# Atarix Architecture Review Template

## Review Target

Name:

Type:

Status:

Related Specification:

Related Pull Request:

## Affected Pillars

```text
Object Model:
Namespace Model:
Security Model:
Authority Model:
Capability Model:
Lifecycle Model:
Mailbox Model:
Resource Model:
Error Model:
Audit Model:
Time Model:
Versioning Model:
Portability Model:
Test Model:
```

## Security Doctrine Compliance

Mark each item PASS, FAIL, N/A, or NEEDS REVIEW.

```text
ARC-SEC-001 Authority not inferred from execution:
ARC-SEC-002 Connectivity does not imply trust:
ARC-SEC-003 Compromise remains contained:
ARC-SEC-004 No ambient authority:
ARC-SEC-005 Explicit lifecycle:
ARC-SEC-006 No orphaned objects:
ARC-SEC-007 Lease behavior defined:
ARC-SEC-008 Crash recovery defined:
ARC-SEC-009 Persistence justified:
ARC-SEC-010 Authority cleanup defined:
ARC-SEC-011 Garbage observable:
ARC-SEC-012 System objects comply:
ARC-SEC-013 Attack surface minimized:
ARC-SEC-014 No identity-based escalation:
ARC-SEC-015 Identity theft is not authority theft:
ARC-SEC-016 Secure defaults:
ARC-SEC-017 Fail closed:
ARC-SEC-018 State auditable:
ARC-SEC-019 Recovery reproducible:
ARC-SEC-020 Least persistence:
```

## Authority Review

How is authority granted?

How is authority revoked?

Can authority be inferred?

Can authority be escalated?

## Lifecycle Review

Who owns each created object?

What is each object's lifetime?

What cleans it up?

What happens after crash, uninstall, removal, or revocation?

## Required Tests

```text
Unit tests:
Integration tests:
Security tests:
Regression tests:
Failure tests:
Lifecycle cleanup tests:
```

## Review Decision

```text
Decision: ACCEPTED / ACCEPTED WITH TESTS / RETURNED / REJECTED
Reviewer:
Date:
Notes:
```


---

# Chapter 5: Security Before Convenience

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md -->
### Integrated source: `docs/ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md`

# ADR: Identity, Trust, and Capability Separation

Status: Accepted

## Context

Many systems implicitly treat identification as authorization.

Example:

```text
Identity
    => Authority
```

This creates tight coupling between discovery, provisioning, security, and resource access.

ATARIX is intended to support:

- factory hardware
- development hardware
- prototype hardware
- homebrew hardware
- future third-party hardware

without requiring changes to core platform architecture.

## Decision

ATARIX separates three concepts:

```text
Identity proves existence.
Trust determines authority.
Capabilities determine access.
```

### Identity

Identity answers:

```text
Who are you?
What are you?
Who built you?
```

Identity is provided by the Identity EEPROM.

Identity validation proves only that a board can identify itself and that the identity record is valid.

Identity alone grants no authority.

### Trust

Trust answers:

```text
How much confidence should the platform place in this device?
```

Trust is evaluated by supervisor policy.

Trust may be influenced by:

- provisioning state
- board class
- trust class
- development mode
- manufacturing records
- future cryptographic validation

Trust does not automatically grant access to resources.

### Capabilities

Capabilities answers:

```text
What may this device do?
```

Capabilities describe permissions and allowed operations.

Examples:

- DMA access
- boot authority
- recovery authority
- mailbox participation
- interrupt generation
- service publication

Capabilities are evaluated independently of identity.

## Consequences

The following states are legal:

```text
Identified but Untrusted
Trusted but Restricted
Homebrew but Allowed
Factory but Limited
```

This supports safe extensibility while preserving platform control.

## Architectural Benefits

- Supports homebrew cards without implicit trust.
- Supports future security enhancements.
- Reduces coupling between discovery and authorization.
- Allows policy evolution without changing hardware identity formats.
- Keeps provisioning, discovery, and capability systems independent.

## Related Specifications

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- capability-record-format-v1.md
- service-directory-format-v1.md
- factory-provisioning-v1.md

## Related Principles

```text
Identity Is A First-Class Service
Observability Is A Feature
Provisionability Is A Feature
Recovery Is Mandatory
```

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-RING-SECURITY-MODEL.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-RING-SECURITY-MODEL.md -->
### Integrated source: `docs/ADR-RING-SECURITY-MODEL.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-THREE-FABRIC-ARCHITECTURE.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-THREE-FABRIC-ARCHITECTURE.md -->
### Integrated source: `docs/ADR-THREE-FABRIC-ARCHITECTURE.md`

# ADR: Three Fabric Architecture

Status: Accepted

## Context

Traditional systems often combine management, enumeration, and data transport into a single bus architecture.

This creates coupling between:

- platform health
- device discovery
- resource authorization
- bulk data movement
- recovery operations

ATARIX is intended to support multiple processor architectures, long service life, integrated instrumentation, self-validation, and recovery independent of the operating system.

A more explicit separation of concerns is required.

## Decision

ATARIX adopts a Three Fabric Architecture.

```text
Management Fabric
Enumeration Fabric
Transport Fabric
```

Each fabric has distinct responsibilities and authority.

## Management Fabric

The Management Fabric is governed by the supervisor.

Primary authority:

```text
RP2350 Supervisor
```

Responsibilities include:

```text
Identity validation
Health monitoring
Telemetry
Provisioning
Recovery
Reset control
Fault logging
Policy enforcement
Instrumentation management
```

Primary question answered:

```text
Is the platform healthy and allowed to operate?
```

The Management Fabric remains available as much as practical during failure and recovery scenarios.

## Enumeration Fabric

The Enumeration Fabric describes the platform.

Primary authority:

```text
Fabric Controller
```

Responsibilities include:

```text
Identity records
Discovery records
Resource records
Service records
Capability descriptions
Provider descriptions
Version descriptions
Binding information
```

Primary questions answered:

```text
What exists?
What services exist?
What versions exist?
How do I bind to them?
```

The Enumeration Fabric describes resources and services.

It does not grant authority.

## Transport Fabric

The Transport Fabric moves information.

Responsibilities include:

```text
Mailbox traffic
DMA traffic
Interrupt signaling
Shared memory transport
Network transport
Bulk data movement
```

Primary question answered:

```text
How do I use the platform?
```

The Transport Fabric is optimized for operation rather than description or governance.

## Relationship Between Fabrics

The fabrics operate together but remain logically distinct.

```text
Management Fabric
    validates
    monitors
    governs

Enumeration Fabric
    identifies
    describes
    binds

Transport Fabric
    moves
    signals
    exchanges
```

## Boot Sequence Model

A typical startup flow is:

```text
Power On
    ↓
Supervisor Initialization
    ↓
Identity Validation
    ↓
Enumeration
    ↓
Capability Evaluation
    ↓
Service Binding
    ↓
Transport Activation
    ↓
Monitor
    ↓
Operating System
```

## Relationship to Identity, Trust, and Capability

The Three Fabric Architecture works together with the Identity/Trust/Capability model.

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Transport operates.
```

Trust evaluation belongs to Management Fabric policy.

Resource and service description belong to the Enumeration Fabric.

Operational communication belongs to the Transport Fabric.

## Instrumentation Integration

Instrumentation services span all three fabrics.

Examples:

```text
Logic Analyzer
Protocol Decoder
Pattern Generator
Counter Bank
DSP Analysis
Trigger Router
```

Instrumentation remains under Management Fabric authority even when exposed through Enumeration and Transport mechanisms.

## Architectural Benefits

This separation:

- reduces coupling
- improves recovery behavior
- improves observability
- supports multiple CPU architectures
- simplifies future expansion
- supports homebrew and experimental hardware
- supports integrated testbench functionality

## Consequences

A CPU card does not require direct knowledge of:

- slot numbering
- physical wiring
- transport implementation details
- fabric topology details

The platform exposes services and resources through enumeration and transport abstractions.

## Related ADRs

- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md

## Related Specifications

- identity-eeprom-v1.md
- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- discovery-rom-format-v1.md
- capability-record-format-v1.md
- service-directory-format-v1.md

## Guiding Principle

```text
Management Fabric
    validates and governs

Enumeration Fabric
    describes and binds

Transport Fabric
    transfers and signals
```

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0002-rp2350-selection.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0002-rp2350-selection.md -->
### Integrated source: `docs/adr/ADR-0002-rp2350-selection.md`

# ADR-0002: RP2350 Supervisor Selection

Status: Accepted

Date: 2026-06-10

## Context

ATARIX needs an always-available supervisor controller for reset sequencing, watchdogs, RTC coordination, fault logging, recovery-mode control, power monitoring, and management-plane communication.

The supervisor must remain operational when the W65C816 CPU card or the FPGA fabric controller is not booting.

## Decision

Use the RP2350 as the Rev A supervisor MCU target.

## Evidence

- Strong GPIO, timer, and peripheral support.
- More headroom than RP2040 for watchdog policy, anomaly scoring, management commands, and future cryptographic checks.
- Good toolchain availability.
- Good community and documentation ecosystem.
- Suitable for deterministic low-speed management functions.

## Alternatives Considered

- RP2040.
- STM32 family.
- ESP32-S3.
- Small CPLD-only supervisor.
- Supervisor implemented inside the FPGA fabric.

## Consequences

Pros:

- Keeps management independent of the main CPU and FPGA fabric.
- Gives the supervisor enough compute headroom for health scoring and fault handling.
- Keeps Rev A implementation accessible and inexpensive.

Cons:

- Adds another firmware target.
- Requires explicit management-plane protocol design.
- Requires careful authority separation between supervisor and fabric controller.

## Future Revisions

Future boards may replace the RP2350, but the logical supervisor role should remain stable.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0014-testing-and-verification-strategy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0014-testing-and-verification-strategy.md -->
### Integrated source: `docs/adr/ADR-0014-testing-and-verification-strategy.md`

# ADR-0014: Testing and Verification Strategy

Status: Accepted

Date: 2026-06-10

## Context

ATARIX combines hardware, firmware, FPGA logic, protocols, security mechanisms, and operating-system research.

Verification must be treated as a required engineering activity.

## Decision

Testing and verification are first-class architectural requirements.

All major architectural interfaces should have documented tests, validation procedures, or simulation coverage.

The project testing authority is:

```text
docs/testing-strategy.md
```

## Principles

- Test behavior rather than implementation details.
- Preserve reproducibility.
- Version test formats.
- Prefer small targeted tests.
- Support randomized testing where practical.
- Record sufficient state to diagnose failures.
- Keep architectural tests portable across implementations.

## Scope

Testing applies to:

- CPU behavior.
- Firmware.
- FPGA fabric.
- Discovery records.
- Mailbox protocols.
- DMA descriptors.
- Capability records.
- Supervisor services.
- Recovery mechanisms.
- Hardware bring-up.
- Integration workflows.

## Consequences

Pros:

- Earlier defect discovery.
- Better documentation.
- Easier hardware bring-up.
- Safer architectural evolution.

Cons:

- Additional engineering effort.
- Additional tooling.
- Additional CI infrastructure.

## Design Rule

If a subsystem cannot be verified, it is not complete.

Observability and testability are architectural features.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-00X-capability-evaluation-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-00X-capability-evaluation-model.md -->
### Integrated source: `docs/adr/ADR-00X-capability-evaluation-model.md`

# ADR-00X: Capability Evaluation Model

Status: Accepted

## Context

ATARIX uses a layered security model built around identity, trust, rings, capabilities, resources, and operations.

The Capability Engine must not be an independent permission system. It must implement the ATARIX trust and ring model.

The security chain is:

```text
Identity
    -> Trust
    -> Revocation
    -> Ring
    -> Ownership
    -> Capability
    -> Resource
    -> Operation
    -> ALLOW / DENY
```

## Decision

ATARIX capability evaluation shall be based on the following authority model:

```text
Supervisor (-2)
    Root of Trust

Fabric (-1)
    Enforcement Authority

Kernel (0)
    System Authority

Drivers (1)
    Device Authority

Services (2)
    Service Authority

Applications (3)
    User Authority
```

The Supervisor is the root of trust.

The Fabric is the enforcement authority.

Capabilities are bound to principals.

Capabilities are not transferable.

Delegation is permitted only when authority is reduced.

Revocation is hybrid: capabilities may expire naturally, and the Supervisor may revoke them immediately.

The default decision is DENY.

## Root of Trust

The Supervisor operates at Ring -2 and is the root of trust.

Responsibilities include:

```text
Identity management
Trust establishment
Trust revocation
Health monitoring
Recovery coordination
```

No entity below Ring -2 may establish system trust.

## Enforcement Authority

The Fabric operates at Ring -1 and enforces access decisions.

Responsibilities include:

```text
Capability enforcement
Resource mediation
Transport authorization
Ring boundary enforcement
```

A CPU, service, driver, or application may request access, but the Fabric is authoritative for fabric-mediated access.

## Principal-Bound Capabilities

Capabilities are bound to a principal.

Possession does not imply ownership.

A capability issued to one principal may not be used by another principal.

A capability request must therefore identify:

```text
Caller principal
Capability owner principal
Issuer principal
Target resource
Requested operation
```

The evaluator must verify that the caller is allowed to use the capability before evaluating the operation.

## Non-Transferability

Capabilities are non-transferable by default and by design.

A principal may not hand an existing capability to another principal as a way to grant authority.

If authority is to be delegated, the system must create a separate delegated capability with reduced authority and its own identity.

## Delegation

Delegation may reduce authority only.

Delegation may never:

```text
Increase ring privilege
Add operations
Expand resource scope
Bypass trust requirements
Bypass revocation
Create Supervisor authority
```

A delegated capability must remain traceable to its parent capability.

## Revocation

ATARIX uses a hybrid revocation model.

Capabilities may have expiration times.

Capabilities may also be revoked immediately by the Supervisor.

A capability is usable only if:

```text
Capability is structurally valid
Capability is not expired
Capability is not revoked
Issuer remains trusted
Owner remains trusted
```

Revocation is authoritative and immediate for future access attempts once the Fabric has received the revocation state.

## Evaluation Order

Capability evaluation shall proceed in the following order:

```text
1. Validate request structure
2. Validate caller identity
3. Validate issuer trust
4. Validate owner trust
5. Check revocation state
6. Check expiration
7. Check caller ring
8. Check capability ownership
9. Check delegation chain
10. Check resource scope
11. Check requested operation
12. Return ALLOW or DENY
```

The evaluator shall stop at the first failing condition and return DENY or a more specific restricted decision where applicable.

## Security Invariants

The following invariants define the model:

```text
I1  Trust precedes capability evaluation.
I2  Capabilities never increase authority.
I3  Delegation may only reduce authority.
I4  Ring boundaries are absolute.
I5  Supervisor authority is non-delegable.
I6  Fabric enforcement is authoritative.
I7  Deny-by-default.
I8  Capabilities are principal-bound.
I9  Revocation is authoritative and immediate.
```

## Ring Enforcement

A capability may not allow a caller to act above its authorized ring.

Ring boundaries are absolute.

A lower-authority principal cannot use a capability to become a higher-authority principal.

Examples:

```text
Application -> Kernel authority: DENY
Service -> Supervisor authority: DENY
Driver -> Fabric authority: DENY
```

## Supervisor Authority

Supervisor authority is non-delegable.

Operations such as trust establishment, trust revocation, power-cycle authority, and recovery override are Supervisor functions.

A capability may describe access to Supervisor-mediated services, but it may not transfer Supervisor authority to another ring.

## Fabric Enforcement

The Fabric is responsible for enforcing policy for fabric-mediated resources and transports.

The Fabric evaluates:

```text
Identity
Trust
Revocation
Ring
Ownership
Capability
Resource
Operation
```

before allowing access.

## Deny-by-Default

If any required field, trust state, revocation state, ownership relation, ring relation, delegation relation, resource scope, or operation permission is missing or invalid, the result is DENY.

ALLOW requires explicit success through the complete evaluation path.

## Consequences

Benefits:

```text
Clear trust root
Fabric-mediated enforcement
Principal-bound authority
Reduced confused-deputy risk
Immediate revocation support
Deterministic evaluation order
Testable security invariants
```

Costs:

```text
More metadata required per request
Delegation requires explicit derived capabilities
Fabric must receive revocation state
Policy evaluation is stricter than simple ACL checks
```

## Implementation Direction

Capability Sprint 1 shall implement this model through:

```text
include/atarix/capability_policy.h
src/capability/capability_policy.c
tests/capability/
```

Initial tests should cover:

```text
Capability validation
Ownership enforcement
Ring boundary enforcement
Delegation reduction
Revocation denial
Expiration denial
Operation allow/deny
Deny-by-default behavior
```

## Related Documents

```text
docs/trust-model-v1.md
docs/capability-record-format-v1.md
docs/operation-id-registry-v1.md
docs/service-directory-format-v1.md
include/atarix/rings.h
include/atarix/capability.h
```

## Revision History

```text
v1 - Initial accepted capability evaluation model.
```

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-00Y-service-oriented-resource-access.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-00Y-service-oriented-resource-access.md -->
### Integrated source: `docs/adr/ADR-00Y-service-oriented-resource-access.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-00Z-core-architectural-doctrines.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-00Z-core-architectural-doctrines.md -->
### Integrated source: `docs/adr/ADR-00Z-core-architectural-doctrines.md`

# ADR-00Z: Core Architectural Doctrines

Status: Accepted

## Context

ATARIX now has a tested Discovery baseline, a tested Capability Policy baseline, and an emerging hardware-validation path.

The project needs a compact set of architectural doctrines that future ADRs, APIs, tests, code reviews, and bench procedures can be checked against.

These doctrines are not low-level implementation rules. They are the constitutional rules of the system.

## Decision

ATARIX adopts the following core architectural doctrines.

## Doctrines

### 1. CPU Width Is Not System Width

The W65C816 is a local processor, not the definition of the whole system bus.

ATARIX may use local 65C816 buses, Fabric services, DMA engines, wider memory systems, and future heterogeneous processors without exposing all resources as direct 65C816-addressable hardware.

### 2. Supervisor Owns Trust

The Supervisor is the root of trust and recovery authority.

It owns reset, watchdog, recovery, health monitoring, trust establishment, and fault response.

### 3. Fabric Enforces Trust

The Fabric is the enforcement point for protected operations.

Trust decisions and capability policy must be enforced at the Fabric boundary before resource access is granted.

### 4. Everything Is a Service

Hardware and system resources are exposed as services.

A UART, timer, storage device, framebuffer, network interface, accelerator, memory window, or diagnostic endpoint is accessed through a service contract, not by assuming a raw address.

### 5. Discovery Advertises

Discovery tells the system what exists.

Discovery does not grant authority.

### 6. Directory Describes

Directory names, resolves, and describes services.

Directory does not grant authority.

### 7. Capabilities Authorize

Capability Policy is the authority mechanism for protected resource access.

A request must be validated through capability policy before protected work is performed.

### 8. No Direct Hardware Access

Software components shall not bypass services to access protected hardware directly.

Direct register or physical-address access is not a public software contract.

### 9. Addresses Are Implementation Details

Physical addresses, register offsets, transport details, and device placement are implementation details hidden behind service contracts.

The same service may later move between FPGA logic, a CPU card, a peripheral card, or a virtual implementation without changing callers.

### 10. Deny by Default

The default authorization result is DENY.

A request is allowed only after satisfying all applicable identity, trust, ring, ownership, resource, operation, revocation, expiration, and delegation checks.

### 11. Hardware Implementation Is Not Software Contract

Rev A hardware choices must not become unnecessary permanent software assumptions.

Software should depend on service contracts and public headers, not board-specific placement.

### 12. Knowledge Is Not Authority

Knowing an address is not authority.

Knowing a handle is not authority.

Knowing a service name is not authority.

Discovery access is not authority.

Directory access is not authority.

Only successful capability evaluation grants authority.

## Architectural Review Rule

Every significant subsystem, public API, and bench test should be reviewable against these questions:

```text
Which doctrine does this implement?
Which ADR justifies this behavior?
Which public API expresses the contract?
Which test or bench procedure enforces it?
```

If the answer is unclear, the design is not yet ready.

## Enforcement

These doctrines are enforced through:

```text
Architecture Decision Records
Public headers
Unit tests
CI
Code review
Bench validation
Fault injection
```

## Relationship to Current Work

Discovery Sprint 1 supports:

```text
Discovery advertises.
```

Capability Sprint 1 supports:

```text
Capabilities authorize.
Deny by default.
Knowledge is not authority.
```

Hardware Validation Sprint 1 will begin proving:

```text
Supervisor owns trust.
Fabric enforces trust.
Everything is a service.
No direct hardware access.
```

## Consequences

Future Directory, Fabric Service, Memory Services, DMA, Storage, Networking, GPU, and SMP work must preserve these doctrines.

Designs that expose raw hardware access as the ordinary programming model violate this ADR.

Designs that allow Discovery or Directory to grant access without capability policy violate this ADR.

Designs that bypass Fabric enforcement for protected operations violate this ADR.

## Result

ATARIX has a defined architectural constitution.

Future work should extend the system without weakening these doctrines.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-001-Security-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-001-Security-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-001-Security-Model.md`

# ATX-SPEC-001 Security Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the mandatory security architecture of Atarix.

The Security Model exists to make compromise expensive, contained, observable, recoverable, and difficult to expand. It does not claim that exploitation can be made impossible.

## Core Doctrine

Atarix assumes:

- Software defects exist.
- Remote code execution may occur.
- Services may be compromised.
- User identities may be stolen.
- Internal nodes are not inherently trusted.
- Connectivity is not trust.
- Cleanup failures become security failures.
- Unknown state is expected.

Security is therefore centered on explicit authority, containment, lifecycle control, and auditability.

## Architectural Axioms

The Security Model depends on these axioms:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Execution is not authority.
Ownership is not authority.
Allocation is not ownership.
Persistence is not ownership.
Connectivity is not trust.
Bootstrap authority is not runtime authority.
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```

## Authority Rule

Code execution never creates authority.

A process, object, service, driver, mailbox endpoint, CPU card, fabric node, or compatibility environment may only perform operations for which it has explicit authority.

Authority may be derived from:

```text
Ownership policy
Capabilities
Delegation
Ring policy
Supervisor-mediated recovery authority
Bootstrap authority during boot only
Administrative policy
```

Authority may not be derived from:

```text
Identity alone
User name alone
Object name alone
Directory lookup
Physical location
Network reachability
Execution context
System component status
```

## Security Evaluation

A normal runtime access decision is constrained by:

```text
Object identity
  -> Ring policy
  -> Ownership / authority policy
  -> Capability validation
  -> Revocation state
  -> Expiration state
  -> Lifecycle state
  -> Resource policy
  -> Operation
  -> ALLOW / DENY
```

If any required security state cannot be verified, the operation must fail closed.

## Ring And Capability Separation

Rings provide coarse-grained boundaries.

Capabilities provide fine-grained authority.

A capability does not automatically bypass ring policy.

A ring position does not automatically grant capability authority.

Cross-ring operations must be explicit, mediated, and auditable.

## Directory And Security

The Directory Service provides discoverability, not authority.

The following are explicitly false:

```text
Knowing a name grants authority.
Resolving a name grants authority.
Enumerating a directory grants authority.
Owning a name grants authority over the target object.
```

Directory state may influence policy decisions, but directory lookup itself is not access.

## Resource Security

Resource use is authority-bearing.

Resource visibility does not grant control.

Resource allocation must be explicit, owned, accounted, auditable, and reclaimable.

Resource exhaustion must not produce broader authority as a fallback.

Reserved recovery, supervisor, and audit resources must be protected from normal workload exhaustion.

## Audit Security

Audit is evidence.

Audit authority and operational authority are separate.

The ability to read audit data does not imply the ability to control the audited component.

The ability to modify system state does not imply the ability to modify audit history.

Audit suppression, loss, exhaustion, and degradation must be observable.

## Error Security

Unknown state must be explicit.

Unknown state may require degraded operation, quarantine, recovery, or manual review.

Unknown state must not be silently treated as valid.

Authority-bearing operations must fail closed when security state is unknown.

## Supervisor Security

The Supervisor Management Fabric is control-isolated from the Operational Fabric.

The Operational Fabric may observe supervisor state through authorized audit or observation bridges.

The Operational Fabric may not directly control supervisor resources.

Supervisor resources include reset, watchdog, RTC, power, recovery, secure boot state, firmware validation state, and fault logs.

Observation is not control.

## Bootstrap Security

Bootstrap authority is not runtime authority.

Before the runtime security model exists, only narrowly scoped bootstrap authority may exist.

Bootstrap authority may validate firmware, initialize minimal hardware, fetch boot data, record boot audit, start supervisor services, or enter recovery mode.

Bootstrap authority may not become normal runtime administrative authority.

## Compatibility Security

Compatibility environments, including future POSIX or virtual hardware personalities, are objects.

They must have explicit capabilities, bounded resources, lifecycle policy, audit visibility, and cleanup.

Compatibility must not weaken native Atarix security semantics.

## Required Security Events

The following must be auditable:

```text
Capability grant
Capability revoke
Delegation
Authorization denial
Policy override
Ring violation
Resource allocation
Resource exhaustion
Directory mutation
Supervisor action
Bootstrap security failure
Recovery mode entry
Quarantine transition
Cleanup failure
```

## Failure Rule

Atarix prefers denial, quarantine, and explicit degraded operation over assumed safety.

The controlling rule is:

```text
When authority cannot be verified, fail closed.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-003-Capability-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-003-Capability-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-003-Capability-Model.md`

# ATX-SPEC-003 Capability Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines capabilities in Atarix.

A capability is an explicit authority-bearing object or record that permits specific operations against a specific object, service, resource, namespace binding, mailbox endpoint, or controlled interface.

Capabilities are not identities.

Capabilities are not names.

Capabilities are not directory entries.

## Core Rule

```text
Possession of identity is not possession of authority.
```

A caller may know an object exists and still lack a capability to operate on it.

## Capability Properties

A capability must be:

- Explicit
- Scoped
- Auditable
- Revocable where possible
- Bound to intended authority
- Constrained by ring policy
- Constrained by lifecycle state
- Constrained by resource policy
- Constrained by expiration or lease where applicable

## Capability Scope

A capability may be scoped by:

```text
Object identity
Resource identity or pool
Directory binding
Mailbox endpoint
Service interface
Operation set
Ring
Owner
Session
Time
Node
Fabric
Policy domain
```

## Capability Operations

Common operations include:

```text
READ
WRITE
CONTROL
ALLOCATE
RESERVE
OBSERVE
ACCOUNT
DELEGATE
REVOKE
CREATE
DESTROY
EXECUTE
RECOVER
```

Operation names are architectural names. Their wire-format encodings are implementation details.

## Capability And Ring Interaction

Rings provide coarse-grained security boundaries.

Capabilities provide fine-grained authority within or across those boundaries.

A capability does not automatically bypass ring policy.

A ring level does not automatically grant a capability.

Cross-ring use must be explicit and auditable.

## Capability And Directory Interaction

Directory lookup does not return authority.

A directory entry may help locate an object, service, or resource, but the caller must still present or derive a valid capability before performing authority-bearing operations.

## Capability And Resource Interaction

Resource use is authority-bearing.

Capabilities may authorize resource allocation, reservation, observation, accounting, revocation, or control.

Resource visibility does not grant resource control.

## Delegation

Delegation creates a derived authority from an existing authority.

Delegated capabilities must not exceed the authority of the delegator.

Delegation must preserve:

```text
Scope
Operation limits
Ring limits
Resource limits
Lifecycle limits
Expiration
Audit chain
Revocation relationship
```

## Revocation

Revocation invalidates capability authority.

Revocation may require:

```text
Mailbox closure
Resource reclamation
Directory update
Lifecycle transition
Audit event
Quarantine
```

A revoked capability must remain denied after ring checks, resource checks, or directory resolution.

## Expiration And Leases

Capabilities may be time-bounded.

Expiration must not silently convert into persistence.

Expired capabilities must fail closed for authority-bearing operations.

## Capability And Audit

Capability events must be auditable, including:

```text
Capability created
Capability granted
Capability delegated
Capability used
Capability denied
Capability expired
Capability revoked
Capability destroyed
```

Audit records should identify the authority chain used for the decision.

## Capability And Error Handling

If capability validity cannot be verified, the operation must fail closed.

If capability state is unknown after crash or recovery, affected authority should be quarantined or denied until reconciled.

## Compatibility Environments

Compatibility VMs, POSIX personalities, virtual hardware, auxiliary compute providers, and service shims are objects and require explicit capabilities.

Compatibility must not become an ambient authority bridge.

## Required Future Work

- Define concrete capability wire format.
- Define capability identity versus capability record identity.
- Define revocation graph semantics.
- Define capability serialization rules.
- Define compatibility with future Policy Model.
- Define version negotiation for capability records in ATX-SPEC-012.

## Summary

Capabilities are explicit authority.

Knowing, naming, observing, or executing is not enough.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-004-Lifecycle-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-004-Lifecycle-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-004-Lifecycle-Model.md`

# ATX-SPEC-004 Lifecycle Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines lifecycle behavior for Atarix objects, resources, bindings, capabilities, mailboxes, services, and persistent state.

Lifecycle is a security concern.

An object that cannot be cleaned up, revoked, audited, or reconciled becomes permanent attack surface.

## Core Rule

```text
No object is immortal by default.
```

Every object and resource must have an owner, scope, lifetime, cleanup path, persistence policy, audit visibility, and failure behavior.

## Lifecycle Axioms

```text
Persistence is not ownership.
Allocation is not ownership.
Ownership is not authority.
Unknown state must be explicit.
Recovery must not silently regrant authority.
Cleanup failure is a security event.
```

## Suggested Lifecycle States

General object lifecycle states:

```text
CREATED
INITIALIZING
ACTIVE
SUSPENDED
QUIESCING
REVOKING
REVOKED
EXPIRED
DESTROYING
DESTROYED
QUARANTINED
RECOVERING
FAILED
UNKNOWN
```

Not every object supports every state.

State transitions must be explicit and auditable.

## Ownership And Lifetime

Every object has an owner or responsible authority.

The owner is accountable for the object's existence and cleanup.

Ownership does not automatically grant every operation on the object.

Owner disappearance must be handled explicitly by policy.

## Leases

Temporary objects and resources should use leases by default.

A lease must define:

```text
Owner
Start time
Expiration time
Renewal authority
Expiration behavior
Cleanup authority
Audit behavior
```

Lease expiration must not silently become persistence.

## Persistence

Persistent state must be explicitly requested and policy-approved.

Persistent objects must define:

```text
Owner
Reason
Scope
Recovery behavior
Removal path
Audit visibility
Version or schema requirements
```

Persistence is not ownership and does not imply permanent authority.

## Cleanup

Cleanup may include:

```text
Capability revocation
Mailbox closure
Resource release
Directory binding removal
Temporary storage removal
DMA shutdown
Audit record generation
Cache invalidation
Persistent state removal or tombstoning
```

Cleanup failure must be visible to Audit and Error models.

## Quarantine

Quarantine isolates uncertain or unsafe state.

Quarantine may apply to:

```text
Objects
Resources
Directory bindings
Persistent storage
Capabilities
Services
Audit streams
```

Quarantine preserves evidence while preventing unsafe operation.

## Recovery

Recovery transitions an object or resource from failed, unknown, or quarantined state toward known safe state.

Recovery must not silently regrant authority.

Recovery must be auditable.

Recovered state must be distinguishable from uninterrupted normal state.

## Directory Integration

When an object is destroyed, directory bindings must be removed, marked stale, or tombstoned.

Destroyed object identities must not be reused to satisfy stale bindings.

## Resource Integration

When an object exits, expires, is destroyed, or loses authority, its resource allocations must be released, revoked, reconciled, or quarantined.

Resource leaks are architectural defects.

## Audit Integration

The following lifecycle events must be auditable:

```text
Object created
Object activated
Object suspended
Object expired
Object revoked
Object destroyed
Object quarantined
Object recovered
Cleanup started
Cleanup failed
Persistent state created
Persistent state removed
```

## Error Integration

Unknown lifecycle state must be explicit.

If lifecycle state cannot be verified, authority-bearing operations must fail closed.

## Supervisor And Bootstrap Integration

Supervisor objects and bootstrap objects are not exempt from lifecycle rules.

Bootstrap authority must not silently become runtime authority.

Supervisor control state must remain isolated while supervisor audit state remains observable where policy permits.

## Required Future Work

- Define concrete lifecycle state machine.
- Define required transitions for object, resource, capability, mailbox, and binding types.
- Define owner disappearance semantics.
- Define uninstall semantics.
- Define persistent-state cleanup semantics.
- Define recovery and reconciliation in ATX-SPEC-018.

## Summary

Lifecycle makes existence accountable.

If Atarix creates something, it must know who owns it, why it exists, how long it lives, how it is cleaned up, and what happens when cleanup fails.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-005-Mailbox-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-005-Mailbox-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-005-Mailbox-Model.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-006-Object-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-006-Object-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-006-Object-Model.md`

# ATX-SPEC-006 Object Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This specification formalizes the Atarix Object Model.

Objects are the fundamental architectural entities of Atarix.

Services, devices, resources, mailboxes, directory bindings, compatibility environments, virtual hardware, supervisor interfaces, and auxiliary compute providers are represented as objects or object-owned resources.

## Core Rule

```text
Name is not identity.
Identity is not authority.
```

An object may be named, discovered, observed, moved, virtualized, or persisted without changing its identity or authority model.

## Object Identity

An object's identity is:

- Opaque
- Immutable
- Never reused
- Independent of name
- Independent of physical location
- Independent of current provider
- Independent of capability possession

Object identity must not be a memory address, table slot, mailbox number, CPU-local handle, physical bus address, or implementation detail.

## Required Object Metadata

Every object has:

```text
Object identity
Object type
Owner or responsible authority
Lifecycle state
Ring association or security domain
Resource accounting
Audit visibility
Version or schema information
```

## Object Names

Objects may have zero, one, or many names.

Names are namespace bindings.

Names are useful for humans, administration, discovery, and service lookup.

Names are not identity and do not grant authority.

## Object Authority

Possessing an object identity does not grant authority.

Operating on an object requires explicit authority through capability, ownership policy, ring policy, lifecycle state, resource policy, and system policy.

## Object Lifecycle

Objects participate in the Lifecycle Model.

Objects may be active, suspended, revoked, expired, quarantined, recovering, destroyed, failed, or unknown depending on type.

Destroyed object identities must not be reused.

## Object Resources

Objects may own, allocate, reserve, expose, or consume resources.

Resource allocation is authority-bearing and must be explicit.

Object ownership does not automatically imply full authority over all resources associated with the object.

## Object Audit

Object events should be auditable, including:

```text
Object created
Object named
Object rebound
Object activated
Object suspended
Object granted authority
Object denied authority
Object quarantined
Object destroyed
Object recovered
```

Audit visibility does not imply control authority over the object.

## Object Errors

Unknown object state must be explicit.

If object identity, lifecycle state, version, policy, or authority cannot be verified, authority-bearing operations must fail closed.

## Services As Objects

A service is an object that offers one or more interfaces.

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

The provider may be a CPU card, auxiliary compute card, storage processor, network card, or future fabric node.

## Virtual Hardware And Compatibility Objects

Virtual hardware, compatibility VMs, POSIX personalities, and legacy environments are objects.

They must have explicit capabilities, bounded resources, lifecycle policy, audit visibility, versioned interfaces, and cleanup.

Compatibility must not become ambient authority.

## Supervisor Objects

Supervisor-facing objects and supervisor audit exports are objects, but supervisor control remains isolated in the Supervisor Management Fabric.

Operational objects may observe supervisor state only through authorized observation or audit bridges.

Observation is not control.

## Local And Remote Objects

An object may be local, remote within the same operational fabric, or eventually remote through a federated fabric.

Rev A requires only one operational fabric.

Physical location must not be exposed as public object identity.

## Required Future Work

- Define object identity wire format.
- Define object type registry.
- Define object schema versioning in ATX-SPEC-012.
- Define service objects in ATX-SPEC-019.
- Define virtual hardware and POSIX compatibility in ATX-SPEC-015.
- Define storage-backed persistent objects in ATX-SPEC-017.

## Summary

Everything visible to Atarix is modeled as an object or object-owned resource.

Objects provide identity and lifecycle context, not automatic authority.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-008-Directory-Service-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-008-Directory-Service-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-008-Directory-Service-Model.md`

# ATX-SPEC-008 Directory Service Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Directory Service Model.

The Directory Service maps human-readable names to object identities. It provides discovery, binding, aliasing, enumeration, lookup, and stale-binding handling for Atarix objects.

The Directory Service does not grant authority.

Lookup is not access.

Knowing a name is not authority.

Resolving an object identity is not authority.

All operations on resolved objects remain subject to ring policy, ownership rules, capability validation, lifecycle state, resource policy, and system policy.

## Design Philosophy

Atarix separates:

```text
Name
Binding
Object identity
Object location
Authority
Capability
Lifecycle state
Resource state
```

The Directory Service answers:

```text
Given this name, what object identity does it currently bind to?
```

It does not answer:

```text
May this caller use the object?
```

That question belongs to the Security, Authority, Capability, Policy, Lifecycle, and Resource models.

## Core Invariants

1. Names are not objects.
2. Names are not capabilities.
3. Object identities are not names.
4. Object identities are not authority.
5. Lookup success does not imply access.
6. Enumeration success does not imply access.
7. Directory visibility does not imply control.
8. Bindings have owners.
9. Bindings have lifecycle state.
10. Bindings have generation metadata.
11. Bindings can become stale.
12. Stale bindings must be observable.
13. Stale bindings must be removable or tombstoned.
14. Destroyed object identities must never be reused to satisfy stale bindings.
15. Directory operations must be auditable.
16. Directory failure must fail closed for authority-bearing operations.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Terminology

### Name

A human-readable identifier within a namespace.

Example:

```text
/system/directory
/service/audit
/fabric/node/0
/device/network/eth0
/user/donovan/session/0
```

### Binding

A directory record that maps a name or path to an object identity.

### Object Identity

An opaque immutable identity defined by the Object Model.

### Alias

A binding that resolves to another binding or canonical object path.

### Directory Entry

A record containing binding metadata.

### Directory Object

An object that stores and manages directory entries for a namespace subtree.

### Stale Binding

A binding whose target object no longer exists, cannot be verified, has moved without update, or is otherwise invalid.

### Canonical Path

The preferred human-readable path for an object.

An object may have multiple names, but at most one canonical path within a given namespace authority.

## Operational Fabric Plane

The Directory Service operates on the Operational Fabric discovery and service planes.

It is not part of the bulk data plane.

It is not supervisor control authority.

The Operational Fabric may request information from directory services, but directory lookup remains separate from control authority and resource authority.

## Top-Level Namespace Layout

The recommended top-level namespace layout is:

```text
/
  system/
  service/
  device/
  fabric/
  user/
  session/
  temporary/
  audit/
```

## Binding Model

The core model is:

```text
Name
  -> Binding
  -> Object Identity
  -> Object
```

A binding must contain at least:

```text
Binding ID
Name / path component
Target object identity
Binding type
Owner
Creator
Lifecycle state
Generation
Creation time
Last update time
Persistence policy
Lease / expiration policy
Audit policy
Version / schema information
```

## Binding Types

Atarix recognizes the following conceptual binding types:

```text
DIRECT
ALIAS
MOUNT
REDIRECT
TOMBSTONE
STALE
```

Aliases, mounts, and redirects must not create authority or bypass capability checks.

## Lookup Semantics

A lookup operation resolves a name into metadata.

Conceptually:

```text
lookup("/system/directory")
```

returns:

```text
Object identity
Object type hint
Binding ID
Binding generation
Owner
Lifecycle state
Policy reference
Directory authority
Version information
```

Lookup does not return a capability.

Lookup does not grant object access.

Lookup does not imply that future lookup of the same name resolves to the same object unless a binding generation or object identity is pinned.

## Lookup Failure

Lookup may fail because:

```text
Name not found
Malformed path
Permission denied to enumerate or resolve
Directory unavailable
Binding stale
Binding expired
Alias loop detected
Redirect denied
Target object destroyed
Policy unavailable
Unsupported version
Unknown state
```

Authority-bearing operations must fail closed when lookup state cannot be verified.

## Lookup And Authority

The following are explicitly false:

```text
Knowing a path grants authority.
Resolving a path grants authority.
Enumerating a directory grants authority.
Owning a name grants authority over the target object.
Owning an object automatically grants authority over all names bound to it.
```

Authority must be checked separately.

The access path is:

```text
Lookup
  -> Object identity
  -> Ring policy
  -> Ownership / authority policy
  -> Capability validation
  -> Lifecycle validation
  -> Resource policy
  -> Operation
  -> ALLOW / DENY
```

## Directory Visibility

Atarix may allow directory entries to be visible without granting object access.

Visibility itself may still be policy-controlled.

Observation is not control.

## Generation And Version

Binding generation identifies binding evolution.

Version identifies schema or protocol compatibility.

These must not be conflated.

Rebinding must increment generation metadata and create an audit event.

Unsupported binding or reply versions must fail closed for authority-bearing operations unless explicit compatibility rules allow degraded interpretation.

## Lifecycle Integration

Directory entries participate in the Lifecycle Model.

Bindings may be:

```text
CREATED
ACTIVE
SUSPENDED
STALE
EXPIRED
REMOVED
TOMBSTONED
QUARANTINED
UNKNOWN
```

When an object is destroyed:

```text
Object destroyed
  -> capabilities revoked or invalidated
  -> mailboxes closed
  -> directory bindings marked stale, removed, or tombstoned
  -> resources reclaimed
  -> audit record generated
```

No binding may silently resolve to a replacement object after target destruction.

## Temporary And Persistent Bindings

Temporary bindings are lease-oriented by default.

Persistent bindings require explicit policy, recovery behavior, cleanup authority, version handling, and audit visibility.

Persistence must not be accidental.

## Distributed Directory Behavior

The Directory Service must support future distributed operation.

A lookup may eventually resolve to:

```text
Local object
Object on another node in the same fabric
Migrated object
Replicated service
Remote namespace authority
Federated fabric gateway
```

Rev A requires only one operational fabric.

Remote resolution does not imply remote access.

## Service Location Independence

Services are discovered by name, interface, version, and capability requirements, not by physical implementation.

A service may be provided by a CPU card, auxiliary compute card, storage processor, network card, or future fabric node.

Service location is not service identity.

## Bootstrap Considerations

Some early boot services may need name-like resolution before the full Directory Service exists.

That behavior belongs to the Bootstrap Security Model.

Bootstrap lookup must not be treated as normal runtime directory authority.

Bootstrap authority is not runtime authority.

## POSIX Compatibility Considerations

Future POSIX compatibility will map POSIX paths onto Atarix namespace and directory operations.

POSIX path lookup must not import POSIX ambient-authority assumptions into Atarix.

## Audit Requirements

The following operations must be auditable:

```text
Create binding
Remove binding
Rename binding
Rebind name
Create alias
Remove alias
Create mount
Remove mount
Directory lookup denied
Directory lookup failed due to stale target
Directory lookup redirected
Directory cache invalidated
Binding expired
Binding tombstoned
Binding quarantined
Unsupported version rejected
```

## Initial Directory Sprint Scope

Directory Sprint 1 should implement only:

```text
Direct bindings
Human-readable paths
Object identity targets
Lookup
Create binding
Remove binding
Generation counters
Stale binding behavior
Basic tests
```

Directory Sprint 1 should not implement distributed lookup, replication, caching, POSIX path emulation, complex mounts, or policy language.

## Required Tests

Initial tests should verify:

```text
Create binding
Lookup existing binding
Lookup missing binding
Remove binding
Removed binding does not resolve
Rebinding increments generation
Alias loop rejected
Lookup does not grant authority
Stale binding rejected
Destroyed object identity not reused
Malformed path rejected
Temporary binding expiration
Directory mutation requires authority
Unsupported version fails closed
```

## Open Questions

Q-001: What exact path syntax is allowed?

Q-002: What maximum path length should be supported?

Q-003: What maximum component length should be supported?

Q-004: Are paths case-sensitive?

Q-005: Are Unicode names allowed in kernel-visible paths, or only higher-level display names?

Q-006: How are binding IDs represented?

Q-007: What minimum metadata must fit in a wire-format directory reply?

Q-008: Which namespace roots are mandatory at boot?

Q-009: How does the Directory Service recover persistent bindings after crash?

Q-010: What exact audit event format is required?

## Summary

The Directory Service provides discoverability, not authority.

Its central rule is:

```text
Lookup is not access.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-009-Resource-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-009-Resource-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-009-Resource-Model.md`

# ATX-SPEC-009 Resource Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Resource Model.

A resource is anything finite that an object may consume, reserve, control, expose, delegate, or depend upon.

Resource use is authority-bearing and must be explicit.

Resource visibility is not authority.

Resource allocation is not ownership.

Resource cleanup is a security requirement.

## Design Philosophy

Atarix treats resources as first-class architectural entities rather than incidental implementation details.

A resource must be:

- Named or discoverable where appropriate.
- Owned or attributable.
- Accounted.
- Constrained.
- Auditable.
- Revocable where possible.
- Reclaimed or quarantined when no longer valid.

The Resource Model answers:

```text
Who owns this resource?
Who may allocate it?
Who is consuming it?
How long may it live?
What authority keeps it alive?
What happens when it is revoked?
What happens when the owner disappears?
```

## Core Invariants

1. Every resource has an owner or allocator authority.
2. Every resource has a lifecycle state.
3. Every resource has accounting metadata.
4. Every resource has a cleanup path.
5. Temporary resources use leases by default.
6. Persistent resource reservations require explicit policy.
7. Resource visibility does not imply control.
8. Resource allocation does not bypass ring policy.
9. Resource ownership does not automatically grant all operations on the resource.
10. Resource exhaustion must fail safely.
11. Resource cleanup must be auditable.
12. Resource leaks are architectural defects.
13. Resource identities must not expose implementation-private handles as public contracts.
14. Resource state must be inspectable by authorized observers.
15. System resources are not exempt from lifecycle, audit, or cleanup rules.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model
- Future scheduler, network, filesystem, process, and POSIX compatibility specifications

## Resource Classes

Atarix recognizes these conceptual resource classes:

```text
Compute resources
Memory resources
Communication resources
Storage resources
Device resources
Fabric resources
Supervisor resources
Environmental resources
Audit resources
Recovery resources
```

## Compute Resources

Examples:

```text
CPU core
CPU time slice
scheduler queue slot
accelerator context
GPU compute queue
FPGA region
auxiliary compute card service
```

A compute resource may be provided by a CPU card, auxiliary compute card, GPU card, FPGA region, Raspberry Pi-class service card, RISC-V service card, storage processor, or future provider.

CPU capability is not system capability.

A weak primary CPU may still access strong system services through explicit resource and service interfaces.

## Memory Resources

Examples:

```text
CPU-local SRAM
FPGA-local buffers
fabric memory
RAM-card memory
DMA buffer
shared memory region
page frame
kernel heap allocation
audit buffer pool
```

Memory location is not memory authority.

CPU-local SRAM, fabric RAM, RAM-card memory, and supervisor RAM have different authority and failure semantics.

Resources are not pooled merely because they exist.

Resources are pooled only when ownership, authority, failure domains, and recovery semantics are compatible.

## Supervisor Resources

Supervisor resources belong to the Supervisor Management Fabric, not the Operational Fabric.

Examples:

```text
reset line
watchdog
RTC
power rail control
firmware update slot
recovery mode control
health sensor
fault log
supervisor RAM
```

Operational objects may observe supervisor resource state through authorized observation or audit paths, but they must not directly control supervisor resources.

Observation is not control.

Supervisor memory is not operational scratch space.

## Audit Resources

Audit resources include:

```text
Audit ingress queues
High-speed audit buffers
RAM-backed audit journals
Audit persistence queues
Persistent audit storage
Supervisor audit buffers
```

Audit resources should be reserved so normal workload exhaustion does not silently disable audit.

Audit generation should not require synchronous physical storage writes.

RAM-backed audit buffering is a target architecture once fabric RAM cards or compatible memory services are available.

## Resource Identity And Naming

Resources may be named through the Directory Service.

Examples:

```text
/device/network/eth0
/fabric/node/0/cpu/0
/fabric/node/0/memory/local
/fabric/node/0/memory/card0
/system/resource-manager
/system/power/budget
```

Resource names are not resource identities.

Resource identities are not authority.

Resource directory lookup does not grant resource access.

## Resource Ownership

Ownership answers:

```text
Who is responsible for this resource?
```

Ownership does not automatically answer:

```text
What operations are allowed on this resource?
```

Authority is still capability and policy mediated.

## Resource Allocation

A resource allocation must include:

```text
Allocation ID
Resource identity or pool identity
Owner
Requester
Authority used
Scope
Lifecycle state
Creation time
Lease or persistence policy
Quota impact
Cleanup policy
Audit policy
Version or schema information
```

Allocation may be exclusive, shared, read-only, write-capable, control-capable, time-sliced, quota-limited, best-effort, reserved, or borrowed.

## Leases And Quotas

Temporary resource allocations use leases by default.

Quota enforcement must fail safely.

If quota state cannot be verified, new authority-bearing allocations should be denied.

Lease expiration must trigger reclamation, revocation, or quarantine.

## Revocation And Reclamation

Resource revocation may be:

```text
Immediate
Deferred
Graceful
Forced
Quarantined
Policy-denied
Unsupported
```

Resources must be reclaimed when the owner exits, owner is destroyed, lease expires, capability is revoked, session ends, node crashes, fabric partitions, policy changes, or quota enforcement requires it.

Reclamation may destroy, return, quarantine, scrub, or transfer the resource by explicit policy.

## Resource Exhaustion

Resource exhaustion is expected and must be safe.

When resources are exhausted, Atarix must:

- Fail closed for authority-bearing operations.
- Preserve system-critical recovery paths where possible.
- Preserve supervisor functions where possible.
- Preserve audit visibility where possible.
- Avoid granting broader authority as a fallback.
- Audit denied or degraded allocations.
- Prefer controlled degradation over undefined behavior.

## Resource Visibility

Authorized observers may inspect resource state.

Observation may include resource identity, type, owner, allocation state, quota use, lease expiration, lifecycle state, audit state, and health state.

Observation does not imply control.

## Resource And Capability Interaction

Capabilities grant explicit authority over resources.

A resource capability may authorize operations such as:

```text
READ
WRITE
CONTROL
ALLOCATE
RESERVE
DELEGATE
REVOKE
OBSERVE
ACCOUNT
```

## Auxiliary Compute And Buffer Cards

Auxiliary compute cards are resource providers, not primary CPU cards.

Examples:

```text
Raspberry Pi 5-class compute/buffer card
ARM SBC card
RISC-V service card
GPU service card
AI accelerator card
Storage processor card
```

These cards may provide services such as hashing, compression, encryption, audit staging, network offload, netboot caching, storage staging, or ZFS-style scrub assistance.

Their RAM is exposed as explicit fabric resources, not as transparent CPU-local RAM.

## Crash And Recovery

After crash or reboot, the Resource Model must reconcile:

```text
Persistent allocations
Leased allocations
Expired leases
Outstanding reservations
Shared memory state
DMA state
Mailbox state
Directory bindings
Audit state
Owner state
```

Unknown resource state must be quarantined or denied until reconciled.

Recovery must not silently regrant authority.

## Bootstrap Resource Considerations

Some resources exist before the full runtime Resource Manager exists.

Examples:

```text
Boot ROM storage
Supervisor firmware image
Fabric bitstream image
Boot network endpoint
NTP bootstrap time source
Recovery image slot
```

Bootstrap resources belong to the Bootstrap Security Model.

Bootstrap resource authority is not runtime resource authority.

## POSIX Compatibility Considerations

Future POSIX compatibility will map POSIX resources onto Atarix resources.

Examples:

```text
File descriptor -> capability-wrapped resource reference
Process -> application/session object
Socket -> network endpoint resource
Pipe -> mailbox or stream resource
Signal -> lifecycle/control event
```

POSIX resource assumptions must not import ambient authority into Atarix.

## Initial Resource Sprint Scope

Resource Sprint 1 should implement only:

```text
Resource type identifiers
Resource object metadata
Resource ownership metadata
Basic allocation records
Basic release records
Basic lease expiration semantics
Basic accounting counters
Basic tests
```

Resource Sprint 1 should not implement distributed resource management, complex quotas, persistent reservations, cross-fabric federation, POSIX emulation, scheduler integration, storage allocation, or network bandwidth management.

## Required Tests

Initial tests should verify:

```text
Create resource record
Allocate available resource
Deny allocation of unavailable resource
Release resource
Released resource returns to available state
Allocation records owner
Allocation records authority used
Lease expiration reclaims resource
Revoked capability prevents further allocation
Resource lookup does not grant allocation authority
Resource visibility does not grant control authority
Quota denial fails closed
Cleanup removes temporary allocation
Destroyed owner triggers resource reclamation
Malformed resource request is rejected
```

## Open Questions

Q-001: What is the wire format for resource identity?

Q-002: Are resource identities object identities, resource-local identities, or both?

Q-003: What minimum resource metadata must be present in C structures?

Q-004: What are the initial resource classes required for Rev A?

Q-005: Does CPU-local SRAM appear as one resource pool or multiple regions?

Q-006: How are DMA windows represented?

Q-007: How should mailbox capacity be accounted?

Q-008: What is the minimum quota model for early implementation?

Q-009: How should unreclaimable resources be represented?

Q-010: Which resources are reserved for supervisor recovery paths?

Q-011: How does audit survive resource exhaustion?

Q-012: Which resource operations require explicit capability bits?

## Summary

The Resource Model makes resource use explicit, owned, auditable, bounded, and recoverable.

Its central rule is:

```text
Resource use is authority-bearing and must be explicit.
```

Its companion rule is:

```text
Observation is not control.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-010-Audit-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-010-Audit-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-010-Audit-Model.md`

# ATX-SPEC-010 Audit Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Audit Model.

The Audit Model provides accountability, observability, provenance, incident reconstruction, lifecycle visibility, resource accountability, and security-event recording across the entire Atarix architecture.

Audit is a first-class architectural subsystem.

Audit is not debugging.

Audit is not logging.

Audit is evidence.

## Design Philosophy

Atarix assumes:

- Compromise may occur.
- Software may fail.
- Authority may be abused.
- Resources may leak.
- Cleanup may fail.
- Systems may crash.
- Administrators may need to reconstruct events long after they occurred.

The Audit Model exists to answer:

```text
What happened?
Who did it?
What authority was used?
What object was affected?
What resource was consumed?
When did it occur?
Why was it allowed?
What changed?
Can we prove it?
```

## Core Principles

1. Audit is architectural.
2. Audit is append-oriented.
3. Audit records are evidence.
4. Observation is not control.
5. Audit visibility does not imply authority.
6. Audit records must have provenance.
7. Audit records must identify authority used.
8. Audit records must survive routine failures where practical.
9. Audit suppression must be observable.
10. Audit exhaustion must not silently disable auditing.
11. System objects are not exempt.
12. Supervisor events are auditable.
13. Cleanup failures are auditable.
14. Resource exhaustion is auditable.
15. Security failures are auditable.
16. Audit generation should not depend on synchronous physical storage writes.
17. Audit storage state uncertainty must be explicit.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

## Core Rule

Audit authority and operational authority are separate.

The ability to read audit data does not imply the ability to modify system state.

The ability to modify system state does not imply the ability to modify audit history.

Observation is not control.

## Audit Architecture

Conceptually:

```text
Objects
Services
Resources
Directory
Lifecycle
Capabilities
Supervisor Fabric
        ↓
Audit Events
        ↓
Audit Ingress
        ↓
High-Speed Audit Buffer
        ↓
Audit Persistence Queue
        ↓
Audit Storage
        ↓
Audit Queries / Export
```

Audit should be treated as a service architecture rather than a collection of log files.

## Audit Domains

Atarix recognizes multiple audit domains:

```text
Security
Lifecycle
Resource
Directory
Supervisor
Fabric
Administrative
Bootstrap
Storage
Recovery
```

## Audit Record Requirements

Every audit record should contain:

```text
Audit record ID
Timestamp
Timestamp source
Event type
Actor identity label
Authority used
Object identity
Operation
Result
Reason
Record version
```

Additional metadata may include:

```text
Resource ID
Binding ID
Capability ID
Policy ID
Ring
Node
Fabric
Session
Version
Generation
Correlation ID
Supervisor provenance
Storage state
```

## Provenance

Every audit record must identify:

```text
Who generated the record
What authority generated it
What subsystem generated it
Where it was generated
What version generated it
```

Audit records without provenance are lower trust.

## Audit Integrity

Audit records should be:

```text
Append-oriented
Sequence numbered
Tamper evident
Ordered where practical
Correlatable
Versioned
Recoverable where practical
```

A compromised application should not be able to rewrite historical audit records.

## Audit Visibility

Audit data is observable according to policy.

Audit visibility does not imply control.

Audit consumers may include administrators, security services, supervisor services, monitoring systems, diagnostic tools, and restricted applications.

## Audit Buffering And Persistence

Audit generation must not require synchronous physical disk writes.

Audit events should first enter a high-speed append buffer.

Audit persistence should be asynchronous and journaled where practical.

Audit resources shall be reserved and isolated from normal workloads where practical.

Audit exhaustion shall be observable.

Supervisor audit buffering shall be independent of operational audit buffering.

Loss of persistent storage shall degrade audit persistence but must not silently disable audit generation.

## Audit Storage Hierarchy

Target architecture:

```text
L0  Supervisor audit buffer
L1  Operational audit ingress buffer
L2  RAM-backed audit journal
L3  Persistent audit journal
L4  Checksummed copy-on-write audit dataset
L5  Archive / replication target
```

Early Rev A implementations may support only supervisor-local fault logging and small operational ring buffers.

Fabric RAM cards or compatible memory services are expected before serious RAM-backed audit journals become practical.

## ZFS-Inspired Recovery Target

The long-term storage target should support ZFS-like properties:

```text
Checksums
Copy-on-write updates
Snapshots or immutable checkpoints
Replayable journal state
Scrub or verification
Send/receive or export equivalent
```

Direct ZFS use is not required for early Atarix, but the audit persistence model should not preclude ZFS or ZFS-like storage.

## Supervisor Audit Bridge

The Supervisor Management Fabric is control-isolated.

Supervisor events may be exported through a Supervisor Audit Bridge.

Conceptually:

```text
Supervisor Fabric
    ↓
Audit Bridge
    ↓
Operational Audit Service
```

The bridge is observational.

The bridge does not grant operational control over supervisor resources.

Examples of exported events:

```text
Watchdog timeout
Boot validation failure
Recovery entry
Power fault
Firmware update
RTC synchronization
```

## Audit And Resource Exhaustion

Audit must remain observable during resource pressure.

If audit buffers are exhausted:

```text
Audit loss must be recorded.
Audit suppression must be visible.
Audit degradation must be visible.
```

The system must not silently stop auditing.

## Audit And Cleanup

Cleanup operations are auditable.

Cleanup failures are security-relevant events.

Examples:

```text
Lease expiration
Object destruction
Mailbox cleanup
Resource reclamation
Persistent state removal
Directory binding removal
```

## Audit And Security Events

The following should always generate audit records:

```text
Capability grant
Capability revoke
Delegation
Authorization denial
Policy override
Ring violation
Supervisor action
Recovery mode entry
Quarantine transition
Bootstrap security failure
Unsupported version rejection
```

## Audit And Crashes

After crash recovery, audit systems should record:

```text
Crash detected
Recovery started
Recovery completed
Unreconciled state
Quarantined resources
Audit continuity state
```

Unknown state should be visible.

## Audit And Bootstrap

Some audit events occur before the runtime Audit Service exists.

Examples:

```text
Boot ROM validation
Supervisor startup
Fabric initialization
Netboot fetch
Firmware validation
```

These belong to the Bootstrap Security Model.

Bootstrap audit records should be imported into the runtime audit stream where practical.

## Audit And Distributed Systems

Future fabrics may contain multiple audit producers.

Audit records should support node identification, fabric identification, correlation IDs, independent provenance, and version information.

Rev A only requires one operational fabric.

## Audit Retention

Retention is policy controlled.

Deletion, truncation, export, or rollover of audit data should itself be auditable.

## Audit Authority

Audit authority should distinguish:

```text
Observe audit
Query audit
Export audit
Manage retention
Purge audit
Import supervisor audit
Verify audit integrity
```

These are separate authorities.

## Failure Rules

When audit certainty decreases, uncertainty must be visible.

Examples:

```text
Timestamp unavailable
Storage unavailable
Buffer exhausted
Producer unavailable
Supervisor disconnected
Unsupported record version
Integrity check failed
```

The system must not fabricate certainty.

## Initial Audit Sprint Scope

Audit Sprint 1 should implement:

```text
Audit record structure
Event categories
Basic audit service
Append-only event recording
Query by event type
Query by object
Basic tests
```

Audit Sprint 1 should not implement distributed correlation, remote audit federation, cryptographic signing, replication, advanced retention, RAM-drive audit persistence, or cross-fabric audit.

## Required Tests

Initial tests should verify:

```text
Record audit event
Query audit event
Append ordering
Record capability grant
Record capability revoke
Record resource allocation
Record object creation
Record object destruction
Record directory mutation
Record authorization denial
Record cleanup event
Record audit exhaustion event
Record unsupported audit version
Supervisor audit import path
```

## Open Questions

Q-001: What exact audit record wire format should be used?

Q-002: Are audit records objects?

Q-003: Should audit records have immutable object identities?

Q-004: What minimum timestamp precision is required?

Q-005: How should supervisor audit records be imported?

Q-006: What integrity guarantees are required in Rev A?

Q-007: What audit metadata is mandatory?

Q-008: What events are required during bootstrap?

Q-009: How should audit survive storage exhaustion?

Q-010: What audit query language is required?

Q-011: What minimum RAM-backed buffering level requires RAM cards?

## Summary

The Audit Model provides evidence, provenance, accountability, and observability across Atarix.

Its central rule is:

```text
Observation is not control.
```

Its companion rule is:

```text
Audit is evidence.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-011-Error-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-011-Error-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-011-Error-Model.md`

# ATX-SPEC-011 Error Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Error Model.

The Error Model provides a consistent architectural framework for handling failures, uncertainty, degradation, recovery, quarantine, reconciliation, and fault reporting throughout the system.

Errors are architectural events.

Errors are not merely return codes.

Failures are expected.

Unknown state is a first-class condition.

## Design Philosophy

Atarix assumes:

- Hardware fails.
- Software contains defects.
- Resources are exhausted.
- Networks partition.
- Storage becomes inconsistent.
- Audit systems degrade.
- Compromise may occur.
- Recovery is required.

The Error Model exists to answer:

```text
What failed?
How certain are we?
What is the blast radius?
Can operation continue?
Can the state be trusted?
Must the resource be quarantined?
Can recovery occur automatically?
Must authority be revoked?
What evidence exists?
```

## Core Principles

1. Failure is normal.
2. Unknown state is explicit.
3. Fail closed for authority-bearing operations.
4. Preserve evidence.
5. Prefer quarantine over assumption.
6. Degradation must be observable.
7. Recovery must be auditable.
8. Uncertainty must never be hidden.
9. Error handling must not create authority.
10. Recovery must not silently regrant authority.
11. Resource exhaustion is an error condition.
12. Cleanup failure is an error condition.
13. Supervisor failures are auditable.
14. Bootstrap failures are auditable.
15. Observation is not control.
16. Version mismatch is explicit.
17. Storage uncertainty is explicit.
18. Audit uncertainty is explicit.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

## Error Categories

### Validation Errors

```text
Malformed record
Invalid capability
Invalid path
Version mismatch
Schema violation
Unsupported feature
```

### Authority Errors

```text
Access denied
Capability missing
Capability expired
Delegation denied
Ring violation
Policy violation
Authority unknown
```

### Resource Errors

```text
Out of memory
Quota exceeded
Mailbox exhausted
Storage exhausted
DMA unavailable
Allocation denied
Reserved resource unavailable
```

### Lifecycle Errors

```text
Object destroyed
Object expired
Lease expired
Cleanup failure
Invalid state transition
Object quarantined
```

### Directory Errors

```text
Name not found
Binding stale
Alias loop
Redirect denied
Namespace unavailable
Unsupported binding version
```

### Communication Errors

```text
Mailbox timeout
Transport failure
Message corruption
Node unavailable
Partition detected
Unsupported message version
```

### Storage Errors

```text
Read failure
Write failure
Checksum mismatch
Journal replay required
Pool unavailable
Snapshot rollback required
Partial write suspected
```

### Audit Errors

```text
Audit storage unavailable
Audit buffer exhausted
Audit import failure
Audit integrity failure
Audit continuity unknown
Unsupported audit record version
```

### Supervisor Errors

```text
Watchdog event
Firmware validation failure
Recovery entry
Power fault
RTC fault
Supervisor unavailable
Supervisor audit bridge unavailable
```

### Bootstrap Errors

```text
Boot ROM validation failure
Supervisor startup failure
Fabric initialization failure
Netboot failure
Time source unavailable
Recovery image unavailable
```

## Error Severity

Suggested severity levels:

```text
INFO
NOTICE
WARNING
ERROR
CRITICAL
FATAL
UNKNOWN
```

Severity does not determine authority.

Severity helps determine response and audit handling.

## Unknown State

Unknown state is a distinct architectural condition.

Examples:

```text
Storage partially recovered
Node restarted unexpectedly
Resource ownership uncertain
Audit continuity uncertain
Directory reconciliation incomplete
Capability revocation state uncertain
Policy version unsupported
```

Unknown state must not be silently treated as valid.

Unknown state may require quarantine, restricted operation, manual review, or recovery workflow.

## Quarantine

Quarantine isolates potentially unsafe state.

Examples:

```text
Unverified storage
Unreconciled resource
Compromised service
Unknown capability state
Corrupted audit stream
Unsupported-version object
```

Quarantine preserves evidence while preventing unsafe operation.

Quarantine does not imply destruction.

## Fail-Closed Rules

Authority-bearing operations must fail closed when:

```text
Authority cannot be verified
Capability cannot be validated
Policy cannot be evaluated
Directory target cannot be verified
Resource ownership is uncertain
Object lifecycle is unknown
Version semantics are unsupported
Audit requirements cannot be satisfied where mandatory
```

Convenience must not override security.

## Degraded Operation

Some services may continue in degraded mode.

Examples:

```text
Read-only storage
Reduced networking
Limited directory service
Reduced audit retention
Supervisor-only recovery mode
```

Degraded operation must be explicit, auditable, observable, policy controlled, and reversible where practical.

## Recovery States

Suggested recovery states:

```text
NORMAL
DEGRADED
RECOVERING
RECONCILING
QUARANTINED
EMERGENCY
FAILED
UNKNOWN
```

State transitions must be auditable.

## Reconciliation

After crashes, partitions, unsupported versions, or partial recovery, the system may require reconciliation.

Examples:

```text
Resource ownership
Directory bindings
Audit continuity
Lease state
Mailbox state
Capability state
Persistent storage state
```

Reconciliation must not assume correctness.

Recovery must not silently regrant authority.

## Error Reporting Requirements

Every error should include:

```text
Error category
Error code
Subsystem
Object identity if applicable
Resource identity if applicable
Authority context
Timestamp
Severity
Recovery recommendation
Version context if applicable
Audit correlation ID if available
```

## Audit Integration

All significant errors must generate audit events.

Particularly:

```text
Authority failures
Recovery actions
Quarantine actions
Resource exhaustion
Supervisor failures
Audit failures
Bootstrap failures
Unsupported version rejections
Storage integrity failures
```

## Supervisor Integration

Supervisor failures should be exported through the Supervisor Audit Bridge.

Supervisor control isolation remains in effect.

Observation is not control.

## Resource Exhaustion

Resource exhaustion is expected.

The system should preserve recovery paths, supervisor functions, and audit visibility where possible.

Resource exhaustion must not create authority bypasses.

## Bootstrap Errors

Bootstrap failures belong to the Bootstrap Security Model.

Bootstrap errors should be imported into runtime audit where practical.

Bootstrap error handling must not grant runtime authority.

## Storage And Persistence Errors

Storage errors may require replay, rollback, quarantine, scrub, import refusal, or manual intervention.

Unknown storage state must remain visible.

Partially committed audit or object state must be replayed, discarded, or quarantined deterministically by future Storage and Recovery specifications.

## Crash Recovery

After a crash:

```text
Detect
Audit
Quarantine uncertainty
Reconcile state
Restore services
Record recovery result
```

Unknown state must remain visible until resolved.

## Initial Error Sprint Scope

Error Sprint 1 should implement:

```text
Error categories
Error codes
Severity model
Unknown state support
Quarantine support
Basic tests
```

Error Sprint 1 should not implement full recovery workflows, storage replay, distributed reconciliation, or policy language.

## Required Tests

Initial tests should verify:

```text
Validation error
Authority error
Resource exhaustion
Directory failure
Quarantine transition
Recovery transition
Unknown state handling
Audit generation for errors
Unsupported version failure
Fail-closed authority check
```

## Open Questions

Q-001: What exact error code namespace should be used?

Q-002: Are error records objects?

Q-003: How are errors correlated across services?

Q-004: What error severity is required for supervisor action?

Q-005: How does degraded operation interact with policy?

Q-006: What is the minimum quarantine API?

Q-007: How are bootstrap errors imported into runtime audit?

Q-008: What storage errors require immediate quarantine?

## Summary

The Error Model treats failure, uncertainty, recovery, and quarantine as first-class architectural concepts.

Its central rule is:

```text
Unknown state must be explicit.
```

Its companion rule is:

```text
When authority cannot be verified, fail closed.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-014-Bootstrap-Security-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-014-Bootstrap-Security-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-014-Bootstrap-Security-Model.md`

# ATX-SPEC-014 Bootstrap Security Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Bootstrap Security Model.

The Bootstrap Security Model describes how Atarix establishes trust from reset, validates early firmware and boot artifacts, constrains pre-runtime authority, records bootstrap evidence, enters recovery, and hands control to the runtime security architecture.

Bootstrap exists to establish runtime trust.

Bootstrap is not runtime.

## Core Rule

```text
Bootstrap authority is not runtime authority.
```

Bootstrap authority exists only to validate, initialize, measure, discover, load, recover, and transfer control.

It must not become permanent administrative authority.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model

This specification informs:

- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Power-On Assumptions

At power-on there is no runtime:

```text
Directory Service
Capability system
Runtime Policy engine
Runtime Audit service
Runtime Service registry
User authority
General-purpose operating system
```

Therefore early trust must come from a minimal root of trust, immutable or protected boot code, hardware identity, signed boot manifests, measured artifacts, and recovery policy.

## Bootstrap Authority

Bootstrap authority may:

```text
Initialize minimal hardware
Read hardware profile information
Validate firmware
Validate FPGA bitstreams
Validate boot manifests
Acquire advisory time
Acquire boot images
Record bootstrap audit events
Start supervisor services
Enter recovery mode
Transfer control to runtime
```

Bootstrap authority may not:

```text
Grant arbitrary runtime capabilities
Create permanent runtime administrator authority
Modify runtime policy without recovery authorization
Access user data as a convenience fallback
Become a general remote administration shell
Silently persist after runtime handoff
```

## Chain Of Trust

Conceptual chain:

```text
Root of Trust / ROM
    -> Supervisor Boot Firmware
    -> Fabric Firmware / Bitstream
    -> Bootstrap Loader
    -> Boot Manifest
    -> Kernel Image
    -> Runtime Supervisor
    -> Runtime Services
```

Each stage must validate or measure the next stage before transferring control.

If validation cannot be completed, the system must enter degraded, recovery, quarantine, failed, or unknown boot state rather than continuing silently.

## Hardware-Bound Boot Artifacts

Boot artifacts must be bound to an authorized hardware profile and signed provenance identity.

A boot artifact signed for one hardware profile must not boot on a different hardware profile unless explicit compatibility policy permits it.

Hardware binding should support layers such as:

```text
Hardware family
Hardware revision
CPU card type
Fabric board type
Supervisor hardware type
Security profile
Optional device identity
```

Binding only to an exact board serial should be optional, because per-machine images are not always desirable.

## Signed Source And Build Provenance

Boot artifacts should include signed source and build provenance.

A boot manifest should contain at least:

```text
Boot manifest version
Hardware profile ID
Hardware profile version
Hardware revision
CPU card type
Fabric board type
Supervisor firmware target
FPGA bitstream target
Boot image hash
Source commit hash
Source reference signature
Build manifest hash
Build toolchain identity
Signature key ID
Signature
```

A boot artifact without verifiable source or build provenance is untrusted unless explicit recovery policy permits a restricted recovery boot.

## Boot Verification Flow

Conceptual flow:

```text
Power on
  -> Read hardware identity / profile
  -> Load boot manifest
  -> Verify manifest version
  -> Verify manifest signature
  -> Verify artifact hashes
  -> Verify hardware target compatibility
  -> Verify source and build provenance
  -> Record bootstrap audit
  -> Boot, degrade, recover, quarantine, or fail
```

## Signature Failure Rules

The following are bootstrap security failures:

```text
Manifest signature failure
Boot artifact hash mismatch
Hardware profile mismatch
Source provenance mismatch
Build manifest mismatch
Unsupported boot manifest version
Unsupported hardware profile version
Unknown signing key
Revoked signing key
```

Authority-bearing boot must fail closed when these failures cannot be resolved by explicit recovery policy.

## Measurements

Bootstrap should measure:

```text
Root firmware hash
Supervisor firmware hash
Fabric bitstream hash
Bootstrap loader hash
Boot manifest hash
Kernel image hash
Configuration hash
Recovery image hash
```

Measurements should be recorded in bootstrap audit buffers and imported into runtime audit where practical.

## Early Audit

Before runtime audit exists, bootstrap audit uses early buffers.

Conceptual path:

```text
ROM / early boot buffer
  -> Supervisor audit buffer
  -> Runtime audit import
```

Bootstrap audit must preserve provenance.

Bootstrap audit import must not create runtime authority.

## Early Time

Time before runtime is uncertain.

Preferred order:

```text
RTC
  -> Supervisor time
  -> Network time
  -> Runtime time service
```

Bootstrap NTP or SNTP is advisory unless validated by policy.

Authority decisions must not depend solely on untrusted early network time.

Time uncertainty must be visible in audit records.

## Pre-OS Network Services

Bootstrap networking is narrowly scoped.

Allowed functions:

```text
Link setup
Static IP or DHCP
ARP
UDP
TFTP
HTTP or HTTPS boot fetch
NTP or SNTP
Boot log upload where policy permits
```

Forbidden functions:

```text
General remote shell
General socket API
User applications
Unmediated remote administration
Runtime network authority
```

Pre-OS networking exists only to bring the system up or recover it.

It is not the runtime network stack.

## Supervisor Startup

The Supervisor Management Fabric should be available before normal runtime services.

Conceptual order:

```text
Power on
  -> Supervisor startup
  -> Fabric initialization
  -> Bootstrap services
  -> Kernel verification
  -> Runtime handoff
```

Supervisor failure must be auditable where possible and must not silently fall back to insecure runtime boot.

## Recovery Mode

Recovery mode is a bootstrap function.

Recovery must be available when normal runtime cannot start.

Examples:

```text
Kernel missing
Boot image invalid
Storage corrupted
Network unavailable
Directory unavailable
Policy database corrupted
Supervisor detects unsafe state
```

Recovery mode is restricted.

Recovery mode must not become permanent administrative runtime.

## Recovery Netboot Image Generation

Recovery mode may produce a signed, hardware-bound netboot image sufficient to restore normal boot.

Conceptual flow:

```text
Boot fails
  -> Enter recovery mode
  -> Validate hardware profile
  -> Fetch or build boot source
  -> Create boot manifest
  -> Bind manifest to hardware profile
  -> Include source and build provenance
  -> Sign manifest and image
  -> Netboot signed recovery-produced image
  -> Import recovery audit trail into runtime audit
```

Recovery signing keys are not normal runtime keys.

Recovery-built images must be hardware-profile-bound.

Recovery-built images must include source and build provenance where practical.

Recovery build and signing actions must be audited.

Recovery-produced boot images may grant only the minimum authority required to boot and reconcile state.

Recovery mode must not become a general-purpose build farm, remote shell, or unrestricted administrator environment.

## Recovery Signing Authority

Recovery signing authority is a distinct authority class.

It may sign:

```text
Recovery netboot manifests
Recovery boot images
Recovery metadata
```

It may not sign arbitrary runtime authority grants unless explicit recovery policy permits and audit records the action.

Recovery signing keys should be protected by supervisor or hardware-backed policy where practical.

## Bootstrap States

Suggested bootstrap states:

```text
BOOT_OK
BOOT_DEGRADED
BOOT_RECOVERY
BOOT_QUARANTINED
BOOT_FAILED
BOOT_UNKNOWN
```

Unknown boot state must be explicit.

## Measured Degradation

Not every bootstrap failure has the same consequence.

Example:

```text
Firmware valid, network unavailable
  -> BOOT_DEGRADED may be permitted.

Firmware validation unavailable
  -> BOOT_RECOVERY or BOOT_QUARANTINED is required.
```

Degraded boot must be policy-controlled and auditable.

## Bootstrap Cleanup And Handoff

When runtime starts, bootstrap authority expires.

Runtime handoff should include:

```text
Boot state
Hardware profile
Verified manifest identity
Image hashes
Measurement records
Time confidence
Network boot source
Recovery status
Bootstrap audit buffer
```

After handoff, bootstrap interfaces must be disabled, restricted, or placed under supervisor control.

## Versioning Requirements

Bootstrap records must be versioned.

Versioned items include:

```text
Boot manifest
Hardware profile
Measurement record
Recovery manifest
Firmware metadata
Netboot protocol profile
```

Unknown versions fail closed for authority-bearing boot decisions unless explicit recovery policy permits restricted recovery.

## Policy Requirements

Bootstrap policy determines:

```text
Allowed hardware profiles
Allowed signing keys
Allowed source references
Allowed recovery images
Allowed degraded boot states
Allowed network boot sources
Allowed recovery signing behavior
```

If bootstrap policy cannot be evaluated, authority-bearing boot must fail closed or enter recovery.

## Audit Requirements

Bootstrap must audit:

```text
Power-on event
Hardware profile detected
Manifest verification result
Signature verification result
Hash verification result
Source provenance result
Build provenance result
Boot decision
Degraded boot decision
Recovery entry
Recovery image generation
Recovery signing action
Runtime handoff
```

## Error Requirements

The Error Model must support bootstrap errors such as:

```text
BOOT_MANIFEST_INVALID
BOOT_SIGNATURE_INVALID
BOOT_HARDWARE_PROFILE_MISMATCH
BOOT_SOURCE_PROVENANCE_INVALID
BOOT_BUILD_PROVENANCE_INVALID
BOOT_POLICY_UNAVAILABLE
BOOT_RECOVERY_REQUIRED
BOOT_UNKNOWN_STATE
RECOVERY_SIGNING_DENIED
```

## Initial Bootstrap Sprint Scope

Bootstrap Sprint 1 should define:

```text
Boot manifest structure
Hardware profile identifiers
Signature verification result codes
Boot state enum
Bootstrap audit event types
Recovery netboot manifest concept
Basic tests
```

Bootstrap Sprint 1 should not implement:

```text
Full secure boot hardware integration
Remote attestation
Full recovery build system
Policy language
Distributed boot federation
General remote administration
```

## Required Tests

Initial tests should verify:

```text
Valid manifest accepted
Invalid signature rejected
Hardware profile mismatch rejected
Unsupported manifest version rejected
Source provenance mismatch rejected
Recovery mode entered after boot failure
Recovery-produced manifest includes hardware profile
Recovery-produced image requires recovery signing authority
Bootstrap authority expires at runtime handoff
Bootstrap audit records generated
Unknown boot state fails closed
```

## Required Future Work

- Define boot manifest wire format.
- Define hardware profile schema.
- Define signing key lifecycle.
- Define recovery signing key storage.
- Define bootstrap audit import format.
- Define supervisor integration in ATX-SPEC-016.
- Define persistent recovery storage in ATX-SPEC-017.
- Define reconciliation after recovery in ATX-SPEC-018.

## Summary

The Bootstrap Security Model establishes trust from reset without turning bootstrap authority into runtime authority.

Its central rules are:

```text
Bootstrap authority is not runtime authority.
Boot artifacts must be hardware-profile-bound and provenance-verifiable.
Recovery may generate signed netboot images, but only under constrained recovery authority.
When bootstrap trust cannot be verified, fail closed or enter recovery.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-016-Supervisor-Management-Fabric.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-016-Supervisor-Management-Fabric.md -->
### Integrated source: `docs/architecture/ATX-SPEC-016-Supervisor-Management-Fabric.md`

# ATX-SPEC-016 Supervisor Management Fabric

## Status

Draft v0.1

## Purpose

This document defines the Atarix Supervisor Management Fabric.

The Supervisor Management Fabric is the isolated management and recovery fabric responsible for reset, power, watchdogs, RTC, health monitoring, fault logging, recovery coordination, secure bootstrap support, firmware validation, and supervisor audit export.

The Supervisor Management Fabric is not the Operational Fabric.

Supervisor authority is isolated.

Supervisor observability is shared only through controlled audit or observation paths.

## Core Rule

```text
The Supervisor Fabric is control-isolated but audit-visible.
```

The Operational Fabric may request supervisor actions.

The Operational Fabric may observe supervisor events where policy permits.

The Operational Fabric may not directly control supervisor authority.

Observation is not control.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model

This specification informs:

- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Design Philosophy

Traditional systems often make the management plane either too powerful and reachable, or too opaque and unauditable.

Atarix separates:

```text
Supervisor control authority
Supervisor observation authority
Operational runtime authority
Recovery authority
Audit export authority
```

The Supervisor Management Fabric exists to preserve recovery and trustworthy observation even when the Operational Fabric is compromised, crashed, wedged, or not yet running.

## Fabric Separation

Atarix recognizes at least two fabric classes:

```text
Operational Fabric
Supervisor Management Fabric
```

### Operational Fabric

The Operational Fabric provides runtime discovery, control, service, mailbox, DMA, memory, storage, networking, and data-plane operations.

### Supervisor Management Fabric

The Supervisor Management Fabric provides management, recovery, and health functions.

It should be physically or logically isolated from normal operational traffic.

The exact Rev A implementation may use RP2350-class supervisor hardware, sideband links, GPIOs, serial links, management mailboxes, or other constrained channels.

## Supervisor Authority

Supervisor authority may include:

```text
Assert reset
Release reset
Power rail control
Watchdog arm / disarm / service
RTC read / constrained write
Enter recovery mode
Validate firmware state
Validate boot state
Record fault state
Export supervisor audit
Coordinate safe shutdown
Coordinate emergency isolation
```

Supervisor authority must not be granted to arbitrary runtime objects.

## Operational Requests

Operational objects may request supervisor actions only through mediated request paths.

Conceptually:

```text
Operational Object
  -> Supervisor Request Gateway
  -> Policy Check
  -> Supervisor Command Evaluator
  -> Supervisor Action or Denial
```

A request is not a command.

A valid operational capability is not supervisor authority.

Supervisor remains the final authority for supervisor-controlled actions.

## Forbidden Direct Control

The Operational Fabric must not directly control:

```text
Reset lines
Power rails
Watchdog disable
Firmware validation state
Recovery mode override
Supervisor signing keys
Supervisor audit buffer mutation
Secure boot decision state
```

Any operational path that can modify these functions must be treated as a supervisor request path and policy-controlled.

## Supervisor Resources

Supervisor resources include:

```text
Reset lines
Power rails
Watchdog timers
RTC
Health sensors
Fault logs
Recovery controls
Firmware validation state
Boot validation state
Recovery signing state
Supervisor audit buffers
Supervisor-local storage
```

Supervisor resources are not ordinary operational resources.

Supervisor RAM and supervisor storage are not operational scratch space.

## Supervisor Audit Bridge

Supervisor events may be exported into the Operational Audit Service through a Supervisor Audit Bridge.

Conceptually:

```text
Supervisor Fabric
  -> Supervisor Audit Buffer
  -> Audit Bridge
  -> Operational Audit Service
```

The Audit Bridge is observational.

It must not allow operational control over supervisor state.

## Supervisor Audit Events

Supervisor audit should include:

```text
Power-on event
Reset asserted
Reset released
Watchdog armed
Watchdog fired
Watchdog serviced
Recovery mode entered
Firmware validation passed
Firmware validation failed
Boot manifest accepted
Boot manifest rejected
RTC adjusted
Power rail fault
Health threshold exceeded
Supervisor request received
Supervisor request denied
Supervisor request executed
Supervisor audit export event
```

Audit events must preserve provenance.

## Command Model

Supervisor commands must be minimal and explicit.

Each command should define:

```text
Command ID
Command version
Required authority
Required policy
Allowed caller class
Expected state
Result code
Audit requirement
Failure behavior
```

Supervisor commands must not be open-ended scripting interfaces.

## Request Results

Supervisor request results may include:

```text
EXECUTED
DENIED
DEFERRED
REQUIRES_APPROVAL
QUARANTINED
UNSUPPORTED
UNKNOWN
FAILED
```

Authority-bearing uncertainty must fail closed.

## Supervisor Lifecycle

Supervisor components participate in lifecycle management.

Suggested states:

```text
RESET
INITIALIZING
ACTIVE
DEGRADED
RECOVERY
QUARANTINED
FAILED
UNKNOWN
```

Supervisor state transitions must be auditable where possible.

## Watchdog Model

Watchdogs are supervisor resources.

Watchdog disable or reconfiguration requires supervisor authority and policy approval.

Operational services may report liveness, but they may not unilaterally disable watchdog protection.

## Reset And Power Model

Reset and power controls are supervisor authority.

Operational runtime may request reset, power cycle, or shutdown.

Supervisor policy decides whether to execute the request.

Unsafe power or reset operations should require audit and may require approval depending on policy.

## RTC And Time Model

RTC is a supervisor resource.

Runtime may observe supervisor time or request time adjustment.

Time modification requires policy.

Bootstrap time uncertainty must be visible in audit records.

## Firmware And Boot Validation

Supervisor participates in firmware and boot validation as defined by the Bootstrap Security Model.

Supervisor may track:

```text
Firmware version
Firmware hash
FPGA bitstream hash
Boot manifest identity
Hardware profile
Recovery signing state
```

Supervisor boot validation state must not be modifiable by normal operational services.

## Recovery Coordination

Supervisor may enter or coordinate recovery when:

```text
Boot validation fails
Watchdog fires
Operational fabric is wedged
Runtime audit is unavailable
Storage state is unsafe
Policy database is unavailable
Hardware fault is detected
```

Recovery coordination must not silently grant runtime authority.

## Recovery Signing Interaction

Recovery signing authority is distinct from runtime authority.

Supervisor may protect recovery signing state or mediate recovery signing requests.

Recovery signing actions must be audited.

Recovery signing keys must not be exposed as operational resources.

## Off-Board Or Remote Management

Off-board supervisor access, if supported, must use stronger protection than on-board sideband wiring.

Requirements may include:

```text
Cryptographic authentication
Integrity protection
Replay protection
Rate limiting
Explicit enablement
Audit logging
Physical presence policy where applicable
```

No unauthenticated remote supervisor control is permitted.

## Encryption And Authentication

Encryption alone is not isolation.

On-board paths may rely on physical/logical isolation plus authenticated commands.

Off-board or remote paths require cryptographic authentication and integrity protection.

Confidentiality should be added where supervisor data exposure is sensitive.

## Failure Behavior

Supervisor failure must be explicit.

Supervisor failure states include:

```text
Supervisor unavailable
Supervisor degraded
Supervisor command path unavailable
Supervisor audit unavailable
Supervisor policy unavailable
Supervisor recovery unavailable
```

If supervisor state cannot be trusted, runtime authority-bearing operations depending on supervisor state must fail closed or enter recovery.

## Policy Requirements

Supervisor policy determines:

```text
Allowed supervisor commands
Allowed requesters
Approval requirements
Recovery triggers
Watchdog behavior
Power behavior
RTC adjustment rules
Remote access rules
Audit export rules
```

Policy failure must not create supervisor authority.

## Versioning Requirements

Supervisor protocols, commands, audit exports, firmware metadata, and recovery metadata must be versioned.

Unknown supervisor protocol versions fail closed for authority-bearing operations unless explicit recovery policy permits restricted recovery behavior.

## Security Requirements

The Supervisor Management Fabric must satisfy:

```text
Control isolation from Operational Fabric
Minimal command surface
Explicit request mediation
Policy-controlled execution
Audit-visible decisions
No ambient supervisor authority
No operational access to supervisor keys
No operational scratch use of supervisor memory
Fail-closed uncertainty
```

## Initial Supervisor Sprint Scope

Supervisor Sprint 1 should define:

```text
Supervisor state enum
Supervisor request result enum
Supervisor command identifiers
Supervisor audit event identifiers
Supervisor request gateway concept
Watchdog event model
Reset request model
Basic tests
```

Supervisor Sprint 1 should not implement:

```text
Full remote management
Cryptographic remote console
Complex policy language
Distributed supervisor federation
General supervisor scripting
```

## Required Tests

Initial tests should verify:

```text
Operational request is not direct command
Unauthorized supervisor command denied
Supervisor request audit generated
Watchdog event audit generated
Reset request denied without authority
Recovery request enters recovery state
Supervisor audit export is observational only
Operational fabric cannot mutate supervisor audit buffer
Unknown supervisor protocol version fails closed
Supervisor authority does not become runtime authority
```

## Required Future Work

- Define supervisor command wire format.
- Define supervisor audit export format.
- Define supervisor request gateway C API.
- Define RP2350 Rev A supervisor mapping.
- Define supervisor key storage requirements.
- Define remote management policy.
- Define integration with Storage and Persistence Model.
- Define integration with Recovery and Reconciliation Model.

## Summary

The Supervisor Management Fabric is the control-isolated management and recovery fabric for Atarix.

Its central rules are:

```text
The Supervisor Fabric is control-isolated but audit-visible.
Observation is not control.
A request is not a command.
Supervisor authority is not runtime authority.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-018-Recovery-and-Reconciliation-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-018-Recovery-and-Reconciliation-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-018-Recovery-and-Reconciliation-Model.md`

# ATX-SPEC-018 Recovery and Reconciliation Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Recovery and Reconciliation Model.

Recovery describes how Atarix returns from failure, degradation, crash, corruption, version mismatch, audit discontinuity, or unsafe state to a known and policy-approved operating state.

Reconciliation describes how Atarix proves that system subsystems agree with each other after failure or recovery.

Recovery is not administrative access.

Recovery is not authority restoration.

## Core Rule

```text
Recovery restores trust.
Recovery does not restore authority.
```

Recovery may verify, repair, reconcile, rebuild, quarantine, replay, or report state.

Recovery may not bypass policy, create ownership, grant arbitrary capabilities, suppress audit, or silently restore authority.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-019 Service Model

This specification informs:

- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-021 Memory and Data Movement Model

## Design Philosophy

Failure is expected.

Recovery must prove safety rather than assume it.

The system may restart, reload, replay, rebuild, or reattach components, but none of those actions automatically restore authority.

The system is not fully recovered until required reconciliation has completed.

## Recovery Domains

Recovery may apply to:

```text
Bootstrap state
Supervisor state
Directory state
Storage state
Service state
Resource state
Capability state
Audit state
Policy state
Compatibility environments
Memory and buffer state
```

Each domain may require different recovery procedures, but all recovery domains must preserve auditability and fail closed when authority is uncertain.

## Recovery Authority

Recovery authority is a distinct authority class.

Recovery authority may:

```text
Inspect state
Verify integrity
Replay journals
Import audit records
Rebuild indexes
Reconcile metadata
Enter quarantine
Produce recovery reports
Request supervisor-mediated recovery
```

Recovery authority may not:

```text
Create administrator authority
Grant arbitrary capabilities
Bypass policy
Suppress audit
Silently override ownership
Silently restore revoked authority
```

Recovery authority is temporary and scoped.

## Recovery States

Suggested recovery states:

```text
RECOVERY_PENDING
RECOVERY_ACTIVE
RECOVERY_DEGRADED
RECOVERY_RECONCILING
RECOVERY_QUARANTINED
RECOVERY_COMPLETE
RECOVERY_FAILED
RECOVERY_UNKNOWN
```

Unknown recovery state must be explicit.

## Recovery Objects

Recovery artifacts are objects.

Examples:

```text
Recovery Manifest
Recovery Journal
Recovery Snapshot
Recovery Report
Recovery Audit Package
Recovery Image
Recovery Policy
Recovery Signing Record
```

Recovery objects are versioned, lifecycle-managed, auditable, and policy-controlled.

## Reconciliation

Recovery and reconciliation are distinct.

Recovery asks:

```text
Can the system operate?
```

Reconciliation asks:

```text
Does the system agree with itself?
```

A system may be booted but not reconciled.

A system may be partially operational but still degraded, quarantined, or pending reconciliation.

## Reconciliation Targets

Reconciliation may compare:

```text
Directory vs Object Store
Directory vs Service Registry
Storage vs Audit
Capabilities vs Objects
Capabilities vs Revocation State
Resources vs Allocations
Services vs Directory Bindings
Supervisor vs Runtime State
Bootstrap Audit vs Runtime Audit
Policy Records vs Runtime Policy Cache
```

Mismatch must be explicit.

## Reconciliation Outcomes

Reconciliation may produce:

```text
CONSISTENT
REPAIRED
REBUILT
QUARANTINED
MANUAL_REVIEW_REQUIRED
FAILED
UNKNOWN
```

Unknown reconciliation state must fail closed for authority-bearing operations that depend on the unreconciled state.

## Journals

Recovery may use journals.

Examples:

```text
Directory Journal
Storage Journal
Audit Journal
Service Registration Journal
Resource Allocation Journal
Policy Journal
```

Journal replay must be:

```text
Version-aware
Generation-aware
Deterministic where practical
Auditable
Policy-controlled
```

Journal replay must not silently restore authority.

## Snapshots

Recovery snapshots are objects.

Recovery snapshots should be:

```text
Read-only by default
Versioned
Auditable
Integrity-checked
Policy-controlled
```

A snapshot preserves state.

A snapshot is not authority.

## Quarantine

Quarantine isolates unsafe or uncertain state while preserving evidence.

Quarantine is required when:

```text
Ownership is uncertain
Authority is uncertain
Integrity is uncertain
Version is unsupported
Policy cannot be evaluated
Audit continuity is broken
Provider behavior is unsafe
Storage provenance is uncertain
```

Core rule:

```text
When trust cannot be established, quarantine beats execution.
```

## Recovery And Audit

Every recovery action should generate audit.

Minimum fields:

```text
Recovery ID
Actor identity label
Recovery authority used
Object or resource identity
Operation
Result
Reason
Timestamp
Version
Generation where applicable
```

Audit continuity must be reconciled before recovered state is considered fully trusted.

## Recovery And Bootstrap

Recovery integrates with the Bootstrap Security Model.

Conceptual flow:

```text
Boot failure
  -> Recovery mode
  -> Signed recovery netboot image where needed
  -> Runtime startup
  -> Runtime recovery import
  -> Reconciliation
  -> Normal, degraded, or quarantined operation
```

A system booted from a recovery-generated image is not fully recovered until reconciliation completes.

## Recovery And Supervisor

Supervisor may trigger or coordinate recovery.

Examples:

```text
Watchdog fired
Power failure detected
Firmware validation failure
Storage fault detected
Runtime failed to report liveness
Operational fabric wedged
```

Supervisor-triggered recovery must be auditable.

Supervisor triggering recovery does not automatically determine the recovery outcome.

## Recovery And Storage

Storage recovery may include:

```text
Checksum verification
Scrubbing
Journal replay
Snapshot recovery
Replication repair
Object reconciliation
Quarantine of unsafe objects
```

Corrupted storage must never become authority.

## Recovery And Services

A service restart is not reconciliation.

Before a recovered service becomes active, recovery must verify:

```text
Service identity
Service provider identity
Service version
Service lifecycle state
Service policy
Service authority
Service resource ownership
Audit continuity where applicable
```

## Recovery And Directory

Directory recovery may include:

```text
Binding validation
Generation verification
Namespace rebuild
Alias validation
Tombstone verification
Object target verification
```

Directory corruption must not create authority.

Lookup remains not access.

## Recovery And Capabilities

Capabilities must be revalidated after recovery when their backing state may have changed.

Recovery must verify:

```text
Capability exists
Capability version is supported
Capability target still exists
Capability is not revoked
Capability has not expired
Capability policy still permits use
```

Recovery must not silently re-enable revoked capabilities.

## Recovery And Resources

Resource recovery may include:

```text
Allocation reconstruction
Lease expiration
Quota reconciliation
Orphan detection
Resource quarantine
Resource release
```

Resource leaks discovered during recovery are audit-relevant events.

## Recovery Cleanup Escalation

Recovery cleanup builds on the Policy Cleanup Escalation Model.

Suggested recovery cleanup levels:

```text
L1 Recovery Cleanup
L2 Recovery Quarantine
L3 Recovery Snapshot
L4 Recovery Isolation
L5 Supervisor Recovery
L6 Manual Recovery
```

Escalation moves toward:

```text
More audit
Less authority
More containment
Greater evidence preservation
```

Escalation must not create broader authority.

## Replay

Replay may be used for:

```text
Audit import
Directory reconstruction
Storage reconstruction
Service registration reconstruction
Resource allocation reconstruction
```

Replay must be version-aware, generation-aware, auditable, and deterministic where practical.

Replay must not assume compatibility when versioning is unknown.

## Recovery Signing

Recovery-generated artifacts may require Recovery Signing Authority as defined by the Bootstrap Security Model.

Examples:

```text
Recovery netboot images
Recovery manifests
Recovery reports
Recovery audit packages
```

Recovery signing is not runtime signing.

Recovery signing must be auditable.

## Recovery Completion

Recovery is complete only when:

```text
Integrity is verified
Policy is verified
Audit is imported or continuity status is explicit
Required reconciliation is complete
Required services are restored or explicitly degraded
Unsafe state is quarantined
Recovery report is generated where required
```

If these conditions are not satisfied, the system remains degraded, quarantined, failed, or unknown.

## Recovery Failure

Recovery failure must be explicit.

Recovery failure may result in:

```text
Restricted operation
Read-only operation
Recovery shell with limited authority
Supervisor recovery mode
Manual review
Full stop
```

Recovery failure must not silently become normal operation.

## Compatibility Recovery

Compatibility environments must be recovered as objects.

Legacy assumptions must not weaken native recovery rules.

If a compatibility environment cannot be reconciled safely, it must remain quarantined or disabled.

## Initial Recovery Sprint Scope

Recovery Sprint 1 should define:

```text
Recovery state enum
Recovery authority definition
Recovery object metadata
Recovery audit event identifiers
Reconciliation result enum
Basic journal replay model
Basic quarantine transitions
Basic tests
```

Recovery Sprint 1 should not implement:

```text
Distributed recovery
Automated cluster recovery
Complex policy language
Full ZFS integration
Remote recovery federation
```

## Required Tests

Initial tests should verify:

```text
Recovery authority does not grant runtime authority
Unknown recovery state fails closed
Reconciliation mismatch produces explicit result
Quarantine blocks authority-bearing operations
Journal replay is audited
Recovery does not restore revoked capability
Storage corruption triggers quarantine
Service restart does not imply reconciliation
Bootstrap recovery import requires reconciliation
Recovery completion requires audit status
```

## Required Future Work

- Define recovery object wire format.
- Define recovery manifest schema.
- Define recovery report schema.
- Define journal replay metadata.
- Define reconciliation API.
- Define recovery audit import format.
- Define integration with Storage and Persistence Model.
- Define integration with Memory and Data Movement Model.
- Define manual recovery authority rules.

## Summary

Recovery restores trust by proving consistency, not by assuming it.

The central rules are:

```text
Recovery restores trust.
Recovery does not restore authority.
Recovery is incomplete until reconciliation completes.
When trust cannot be established, quarantine beats execution.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/design-principles.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/design-principles.md -->
### Integrated source: `docs/architecture/design-principles.md`

# Atarix Design Principles

## Purpose

This document defines the guiding principles that govern Atarix architectural and implementation decisions.

## Principles

1. Objects first.
2. Human names first.
3. Authority is explicit.
4. Security is layered.
5. Distribution is native.
6. Hardware independence is required.
7. Testability comes first.
8. Architecture precedes implementation.

## Objects First

Everything visible within Atarix is represented as an object: services, devices, mailboxes, storage entities, fabric nodes, and kernel facilities.

## Human Names First

Public interfaces expose human-readable names. Numeric handles, table indexes, memory addresses, and slot numbers are implementation details.

## Authority Is Explicit

Every operation must have a traceable source of authority. Authority must never arise from undocumented special cases.

## Hardware Independence

Core architecture shall not depend on a specific processor family, memory-management model, address size, or bus architecture.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/directory-service.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/directory-service.md -->
### Integrated source: `docs/architecture/directory-service.md`

# Atarix Directory Service Architecture

## Purpose

The Directory Service provides name-to-object resolution for Atarix.

Conceptually:

```text
human-readable name
  -> directory binding
  -> object identity
  -> object resolution
```

The Directory Service is not the object system, the capability system, or the mailbox system.

## Responsibilities

The Directory Service is responsible for:

- Registering name bindings.
- Looking up names.
- Removing stale bindings.
- Managing aliases.
- Enumerating namespace entries.
- Supporting service discovery.
- Supporting namespace watch notifications.
- Supporting future distributed directory caches.

## Non-Responsibilities

The Directory Service does not grant permissions, manage capabilities, schedule tasks, route messages, validate ring security, create objects, or destroy objects.

Security and authority checks remain the responsibility of the Security, Authority, Capability, and Object models.

## Security Rule

Names are not capabilities.

Lookup success does not imply access.

Access still requires ring validation, capability validation, and object-state validation.

## Binding Lifetime

When an object is destroyed, directory bindings to that object become stale and must be removed or marked stale.

Destroyed object identities shall not be reused to satisfy stale bindings.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/pillars.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/pillars.md -->
### Integrated source: `docs/architecture/pillars.md`

# Atarix Architectural Pillars

## Purpose

This document defines the architectural pillars upon which the Atarix operating system is built.

All subsystems shall identify which pillars they depend upon, which pillars they modify, and which pillars they must not violate.

## Foundational Pillars

- Object Model
- Namespace Model
- Security Model
- Authority Model

## Core Runtime Pillars

- Capability Model
- Mailbox Model
- Lifecycle Model
- State Model
- Resource Model

## Operational Pillars

- Error Model
- Audit Model
- Time Model
- Versioning Model
- Policy Model

## Engineering Pillars

- Portability Model
- Test Model
- Development Process

## Core Rule

Architecture defines implementation. Implementation shall not silently redefine architecture.

<!-- AEMS-MIGRATED-SOURCE: docs/boot-sequence-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/boot-sequence-v1.md -->
### Integrated source: `docs/boot-sequence-v1.md`

# ATARIX Boot Sequence v1

## Status

Draft boot architecture specification.

This document defines the expected power-on, reset, supervisor, FPGA, ROM monitor, discovery, and handoff sequence for ATARIX Rev A through Rev C.

## Purpose

The boot sequence must make the system recoverable, observable, and debuggable from the first hardware revision onward.

A failed boot should produce useful diagnostic information rather than a silent dead board.

## Design Goals

1. Bring up Rev A with minimal dependencies.
2. Permit supervisor-assisted recovery.
3. Permit FPGA configuration before CPU release in later revisions.
4. Keep ROM monitor entry deterministic.
5. Support discovery and capability initialization.
6. Allow safe fallback to golden ROM or recovery mode.

## Boot Actors

Primary boot participants:

```text
Supervisor Controller
FPGA Fabric
W65C816 CPU
ROM Monitor
UART Console
Future Network Service
```

## Boot Phases

```text
Phase 0 Power Stable
Phase 1 Supervisor Start
Phase 2 Fabric Preparation
Phase 3 CPU Reset Release
Phase 4 ROM Monitor Start
Phase 5 Hardware Diagnostics
Phase 6 Discovery Scan
Phase 7 Service Initialization
Phase 8 Loader or Kernel Handoff
```

---

## Phase 0: Power Stable

Power rails must settle before logic is released.

Required checks:

- +5V within tolerance.
- +3.3V within tolerance.
- Clock source stable.
- Reset asserted.

If power is unstable, the supervisor must hold the CPU and FPGA in reset.

---

## Phase 1: Supervisor Start

The supervisor controller starts before the W65C816.

Responsibilities:

- Validate power-good state.
- Select normal ROM or golden ROM.
- Check recovery request input.
- Initialize fault log buffer.
- Prepare reset sequencing.

Recovery triggers may include:

```text
Manual recovery switch
Previous boot failure
ROM checksum failure
Supervisor command
Watchdog timeout
```

---

## Phase 2: Fabric Preparation

Rev A may not require FPGA fabric.

Rev C and later should configure the FPGA before CPU release when the FPGA owns address decode, interrupt routing, or other critical services.

Fabric preparation includes:

- Load FPGA bitstream.
- Verify configuration done signal.
- Initialize fabric control registers.
- Disable DMA by default.
- Disable accelerators by default.
- Enable only required boot services.

Default security state:

```text
DMA disabled
Accelerators disabled
Discovery read-only
Interrupt routing minimal
Supervisor mailbox enabled
```

---

## Phase 3: CPU Reset Release

After required services are ready, the supervisor releases the W65C816 reset line.

The CPU begins execution from the reset vector in ROM.

Rev A reset path:

```text
Power Stable -> CPU Reset Release -> ROM Monitor
```

Rev C reset path:

```text
Power Stable -> Supervisor -> FPGA Config -> CPU Reset Release -> ROM Monitor
```

---

## Phase 4: ROM Monitor Start

The ROM monitor is the first W65C816 program executed.

Initial monitor tasks:

- Enter known processor mode.
- Initialize stack.
- Initialize direct page.
- Initialize UART.
- Print boot banner.
- Report ROM version.
- Report reset reason if available.

Minimum banner fields:

```text
ATARIX ROM Monitor
ROM Version
Reset Reason
CPU Mode
Detected RAM
```

---

## Phase 5: Hardware Diagnostics

Early diagnostics should be conservative.

Recommended checks:

1. ROM checksum.
2. Basic RAM pattern test.
3. UART loopback or transmit test.
4. Supervisor mailbox ping.
5. Fabric status read if present.

Diagnostics should be skippable by command or jumper once hardware is stable.

---

## Phase 6: Discovery Scan

The monitor scans known discovery sources.

Discovery sources may include:

```text
Fixed ROM tables
Supervisor-provided tables
FPGA discovery block
Slot discovery ROMs
Device-local EEPROMs
```

The monitor must:

1. Verify discovery magic.
2. Validate version.
3. Read fixed identity header.
4. Iterate TLV records.
5. Register devices in a simple device table.
6. Ignore unknown optional records.

See:

```text
docs/discovery-rom-format.md
```

---

## Phase 7: Service Initialization

After discovery, the monitor initializes boot-critical services.

Candidate service order:

1. UART console.
2. Supervisor mailbox.
3. Timer service.
4. Interrupt controller.
5. Network service if present.
6. Storage service if present.

DMA must remain disabled until a valid capability model exists for the requested transfer.

---

## Phase 8: Loader or Kernel Handoff

The monitor may enter one of several modes:

```text
Interactive Monitor
Serial Loader
Network Loader
Storage Boot
Kernel Handoff
Recovery Mode
```

Kernel handoff requires:

- Memory map.
- Device table.
- Capability root or boot token.
- Interrupt state.
- Loader parameters.

## Recovery Mode

Recovery mode should provide:

- UART console.
- ROM inspection.
- RAM test.
- Firmware upload.
- FPGA reload if supported.
- Supervisor fault log access.

Recovery mode must avoid enabling DMA, accelerators, or untrusted device services by default.

## Fault Reporting

Boot faults should include:

```text
Boot Phase
Fault Code
Reset Reason
Supervisor Status
Fabric Status
Last Successful Step
```

Fault data should be visible through:

- UART monitor.
- Supervisor mailbox.
- Fault log command.

## Boot Security Rules

Default boot posture:

```text
Least privilege
No DMA by default
No accelerator execution by default
No unverified service startup
Supervisor root authority
```

## Open Questions

- Final ROM monitor command set.
- Whether network boot appears in Rev B or later.
- Exact supervisor fault-log layout.
- Whether golden ROM is physically separate or bank-selected.
- Whether the FPGA bitstream is supervisor-loaded or ROM-loaded.
- Kernel handoff data-structure format.

## Design Rule

The machine must always have a path back to a human-visible monitor prompt unless hardware itself has failed.

<!-- AEMS-MIGRATED-SOURCE: docs/bus-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/bus-architecture.md -->
### Integrated source: `docs/bus-architecture.md`

# ATARIX Bus Architecture

## Status

Draft architectural decision record.

This document records the selected direction for the ATARIX card and backplane architecture.

## Design Decision

ATARIX will use a hybrid modular bus architecture inspired by Sun workstation expansion buses and Apple NuBus-style self-description.

The backplane should not simply expose the raw W65C816 bus to every slot.

Instead, the W65C816 bus remains local to the CPU card where possible, while the backplane exposes a structured fabric-oriented interface for devices, services, accelerators, and future CPU modules.

## Influences

### Sun SPARCstation-Style Expansion

ATARIX should take inspiration from Sun workstation expansion systems such as SPARCstation-class modular I/O.

Relevant ideas:

- Workstation-oriented modular expansion.
- Stable card/backplane mechanical model.
- Clean separation between host processor and expansion devices.
- Device slots as first-class architectural participants.

### NuBus-Style Self Description

ATARIX should also take inspiration from NuBus-style cards.

Relevant ideas:

- Cards identify themselves.
- Cards describe resources.
- Driver matching can rely on structured metadata.
- Expansion devices are more than simple fixed-address peripherals.

ATARIX discovery is defined separately in:

```text
docs/discovery-rom-format.md
```

## Hybrid ATARIX Approach

The selected model is:

```text
W65C816 local bus
        |
CPU card local glue / bridge
        |
ATARIX fabric interface
        |
Active backplane
        |
Service cards / fabric cards / accelerator cards
```

The CPU card may expose selected bus-derived transactions, but the backplane should prefer a controlled fabric interface rather than every raw CPU signal.

## Why Not a Raw 65C816 Backplane?

A raw CPU bus backplane is simple, but it creates long-term problems:

- Poor scaling to multiple CPU cards.
- Difficult signal integrity.
- Weak fault isolation.
- Difficult DMA mediation.
- No natural discovery model.
- No clean authority boundary.
- Harder accelerator integration.

ATARIX is intended to become more than a single-board 65C816 machine, so the bus should not be designed around the simplest possible first revision only.

## Why Not a Fully Abstract Fabric Only?

A fully abstract fabric would lose the value of the W65C816 architecture and make early bring-up harder.

ATARIX should preserve:

- Observable CPU behavior.
- Simple Rev A diagnostics.
- ROM monitor access.
- Direct hardware understanding.

Therefore, the CPU-local bus remains important, especially in Rev A and Rev B.

## Layered Bus Model

### Layer 0: CPU Local Bus

Scope:

- CPU card only.

Contains:

- W65C816 address bus.
- W65C816 data bus.
- CPU control signals.
- Local ROM or boot mapping.
- Local SRAM if used.
- Local glue logic.

### Layer 1: CPU Bridge Interface

Scope:

- Boundary between CPU card and system fabric.

Responsibilities:

- Translate CPU bus cycles into fabric transactions.
- Isolate CPU card timing from backplane timing.
- Provide wait-state or ready signaling.
- Support debug visibility.

### Layer 2: ATARIX Fabric Interface

Scope:

- Backplane-level logical interface.

Responsibilities:

- Addressed transactions.
- Mailbox delivery.
- Interrupt routing.
- Discovery access.
- DMA request mediation.
- Capability-aware access control.

### Layer 3: Active Backplane

Scope:

- Slot-to-slot system interconnect.

Responsibilities:

- Slot identification.
- Reset distribution.
- Clock distribution.
- Power distribution.
- Fabric routing.
- Fault containment.

### Layer 4: Service and Accelerator Modules

Scope:

- Cards attached to the backplane.

Examples:

- Network card.
- Storage card.
- FPGA service card.
- ARM compute module.
- Future accelerator.
- Future secondary CPU card.

## Card Identity

Every nontrivial card should provide discovery information.

Required identity fields:

- Vendor ID.
- Device ID.
- Device class.
- Revision.
- Capability records.
- Optional driver binding string.

## Transaction Model

The fabric interface should support these transaction classes:

```text
Register Read
Register Write
Mailbox Send
Mailbox Receive
Discovery Read
DMA Request
Interrupt Signal
Fault Report
```

Early revisions may implement only a subset.

## CPU Card Role

The primary CPU card is not just a passive bus master.

It is a participant in a larger system fabric.

Responsibilities:

- Boot monitor execution.
- Service discovery.
- Fabric register access.
- Mailbox participation.
- Capability broker participation in later revisions.

## FPGA Role

The FPGA is the natural fabric bridge and enforcement assistant.

Responsibilities may include:

- Address decode.
- Transaction routing.
- Interrupt routing.
- Mailbox handling.
- Discovery table exposure.
- DMA mediation.
- Fault logging.

The FPGA should not be treated as an undocumented black box.

## Backplane Design Implications

The backplane should include signal groups for:

- Power.
- Ground.
- Reference clock.
- Reset.
- Slot identification.
- Interrupt or event signaling.
- Fabric data path.
- Fabric control path.
- Discovery access.
- Debug and recovery.

The exact pinout is deferred to:

```text
docs/backplane-pinout-v1.md
```

## CPU Card Design Implications

The CPU card should include:

- Local W65C816 bus.
- Local bridge or glue logic.
- Debug access.
- Reset control.
- Clock input or local clock selection.
- Discovery record.
- Fabric-facing interface.

The exact pinout is deferred to:

```text
docs/cpu-card-pinout-v1.md
```

## Security Implications

Because the backplane is not a raw shared CPU bus, ATARIX can enforce stronger rules:

- Devices are not automatically trusted.
- DMA requests can be mediated.
- Discovery can declare required capabilities.
- Faults can be isolated per slot.
- Accelerators can receive explicit memory grants.

See:

```text
docs/capability-model.md
docs/dma-engine-v1.md
```

## Development Phasing

### Rev A

Raw W65C816 system for bring-up.

### Rev B

CPU card preparation and early bridge concepts.

### Rev C

FPGA-mediated fabric interface begins.

### Rev D

Multi-CPU and mediated resource experiments.

### Rev E

Accelerator and heterogeneous compute framework.

## Design Rule

Keep the W65C816 bus local where possible.

Expose a structured, discoverable, fabric-oriented interface to the backplane.

This preserves early debuggability while allowing ATARIX to grow into a modular workstation architecture.

<!-- AEMS-MIGRATED-SOURCE: docs/capability-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/capability-model.md -->
### Integrated source: `docs/capability-model.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/capability-record-format-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/capability-record-format-v1.md -->
### Integrated source: `docs/capability-record-format-v1.md`

# Capability Record Format v1

Status: Draft v1

## Purpose

Capability Records define authorized operations against resources and services discovered through the Enumeration Fabric.

Discovery describes what exists.

Capabilities authorize what may be done.

Rings constrain who may exercise that authority.

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Non-Goals

Capability Records do not:

- identify hardware
- enumerate resources
- define physical addresses
- define IRQ numbers
- define DMA channels
- replace supervisor safety policy
- override Ring -2 authority

## Handle Binding

Capabilities target handles produced by Discovery and later resolved by Directory Services.

A capability target is a handle, not an address.

## Capability Record Layout

All numeric fields are little-endian.

All records are 8-byte aligned.

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Capability ID
0x04    4     Operation ID
0x08    8     Target Handle
0x10    2     Minimum Ring
0x12    2     Maximum Ring
0x14    4     Flags
0x18    8     Delegation Mask
0x20    8     Expiration / Lifetime
0x28    8     Issuer Handle
0x30    4     Trust Requirement
0x34    4     Reserved
```

Total size:

```text
56 bytes
```

## Capability ID

Capability ID identifies the capability class.

Examples:

```text
READ_HEALTH
READ_COUNTERS
CAPTURE_TRACE
CONTROL_MAILBOX
PERFORM_DMA
BOOT_FROM_RESOURCE
ENTER_RECOVERY
DRIVE_PATTERN_GENERATOR
```

A formal capability ID registry may be defined later.

## Operation ID

Operation ID identifies the specific operation authorized by this record.

Examples:

```text
READ
WRITE
CONTROL
RESET
CAPTURE
STIMULATE
BOOT
RECOVER
PROVISION
```

## Target Handle

Target Handle identifies the resource or service to which the capability applies.

The handle uses the Discovery ROM handle model.

## Minimum Ring and Maximum Ring

Rings constrain who may exercise the capability.

Initial rings:

```text
-2  Hardware Safety / Supervisor Authority
-1  Fabric / Platform Firmware Authority
 0  Kernel / Executive Authority
 1  Driver / Service Authority
 2  User Service Authority
 3  Application Authority
```

Encoding recommendation:

```text
Ring -2 = 0xFFFE
Ring -1 = 0xFFFF
Ring  0 = 0x0000
Ring  1 = 0x0001
Ring  2 = 0x0002
Ring  3 = 0x0003
```

## Flags

Initial flags:

```text
Bit 0   Read allowed
Bit 1   Write allowed
Bit 2   Control allowed
Bit 3   Delegation allowed
Bit 4   Revocation supported
Bit 5   Time limited
Bit 6   Development only
Bit 7   Recovery only
Bit 8   Destructive operation
Bit 9   Requires supervisor approval
Bits 10-31 reserved
```

## Delegation Mask

Delegation Mask describes which lower rings, if any, may receive delegated access.

A zero mask means no delegation.

## Expiration / Lifetime

Expiration may represent:

- zero for no expiration
- monotonic deadline
- policy-defined lifetime
- provisioning epoch

Exact interpretation is policy-defined.

## Issuer Handle

Issuer Handle identifies the provider, supervisor, firmware, or authority that issued the capability.

Issuer authority must be validated by trust and ring policy.

## Trust Requirement

Trust Requirement indicates the minimum trust state required before the capability may be accepted.

Initial values:

```text
0  UNTRUSTED
1  IDENTIFIED
2  VALIDATED
3  TRUSTED
4  PRIVILEGED
5  RESERVED
6  RESERVED
7  REVOKED
```

A capability requiring TRUSTED shall not be accepted from a merely IDENTIFIED entity.

A REVOKED entity shall not grant usable capabilities.

## Security Rules

```text
Discovery describes.
Capabilities authorize.
Rings constrain authority.
Supervisor safety authority overrides capability grants.
```

A valid capability record is not sufficient by itself.

The platform must also evaluate:

- identity
- discovery validity
- trust state
- ring authority
- supervisor policy
- recovery state
- revocation state

## Homebrew Default Policy

Homebrew cards may advertise capabilities, but default platform policy should reject or restrict dangerous operations.

Default denials:

```text
DMA
Boot authority
Recovery authority
Supervisor authority
Memory-controller authority
Privileged instrumentation stimulus
```

## Related Documents

- discovery-rom-format-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md
- resource-type-registry-v1.md
- service-id-registry-v1.md
- service-directory-format-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/card-health-model-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/card-health-model-v1.md -->
### Integrated source: `docs/card-health-model-v1.md`

# Card Health Model v1

Status: Draft v1

## Purpose

This document defines the common ATARIX card health model.

The health model provides a shared vocabulary for supervisors, validation tools, provisioning tools, monitors, firmware, and operating systems.

The health model answers:

```text
Is this card safe?
Is this card ready?
If not, why?
```

## Architectural Principles

```text
Measurement must not depend on the thing being measured.
```

```text
Every subsystem shall be:

Discoverable
Observable
Testable
Instrumentable
Recoverable
```

## Overall Health States

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

## State Definitions

### ABSENT

No card is detected.

### PRESENT_UNVALIDATED

A card appears physically present, but identity and health validation are incomplete.

### IDENTITY_VALID

Identity EEPROM validation succeeded.

### DISCOVERY_VALID

Discovery ROM validation succeeded.

### RESTRICTED

The card is valid but policy restricts participation.

Common causes:

- homebrew policy
- development mode
- insufficient trust
- missing capability authorization
- ring restriction

### READY

The card has passed required validation and is available for normal participation.

### WARNING

The card is operational but has a warning condition.

Examples:

- elevated temperature
- rail margin warning
- degraded link
- missing optional service

### FAULTED

The card has a fault that prevents normal operation.

### SAFE_MODE

The card is held in a safe operating state pending diagnosis or policy decision.

### RECOVERY_MODE

The card is in a recovery or provisioning workflow.

## Health Domains

Each card may report health in multiple domains.

Required domains where applicable:

```text
Identity Health
Discovery Health
Power Health
Current Health
Thermal Health
Clock Health
Reset Health
Watchdog Health
Communication Health
Fabric Health
CPU Health
Instrumentation Health
```

## Domain States

Recommended per-domain states:

```text
UNKNOWN
NOT_PRESENT
OK
WARNING
CRITICAL
FAULT
RECOVERING
DISABLED_BY_POLICY
```

## Power Health

Power health may include:

- input voltage present
- regulator output valid
- rail sequencing complete
- under-voltage state
- over-voltage state
- power-good state

Common rails:

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

## Current Health

Current health may include:

- input current
- per-rail current
- inrush event count
- over-current state
- fuse or eFuse state
- power budget class

## Thermal Health

Thermal health may include:

- board temperature
- regulator temperature
- FPGA temperature
- CPU temperature
- memory temperature
- warning threshold
- critical threshold

## Clock Health

Clock health may include:

- reference clock present
- fabric clock present
- CPU clock present
- PLL lock state
- frequency within tolerance
- missing-clock state

## Reset and Watchdog Health

Reset and watchdog health may include:

- reset asserted state
- reset reason
- reset hold reason
- watchdog enabled
- watchdog timeout count
- last watchdog event

## Communication Health

Communication health may include:

- sideband bus state
- mailbox state
- fabric link state
- timeout count
- transaction error count

## CPU Health

CPU cards additionally use the CPU Observability Contract.

Common CPU health indicators:

```text
CPU_POWER_GOOD
CPU_CLOCK_GOOD
CPU_RESET_ASSERTED
CPU_RESET_RELEASED
CPU_FETCHING
CPU_EXECUTING
CPU_ENUMERATED
CPU_MONITOR_RUNNING
CPU_OS_RUNNING
CPU_FAULTED
```

## Instrumentation Health

Instrumentation health may include:

- logic analyzer available
- trace buffer available
- protocol decoder available
- counter bank available
- trigger router available
- capture active
- capture fault

## Event Reporting

Health transitions should generate supervisor event log records.

Recommended fields:

```text
timestamp_us
slot_or_card_id
health_domain
previous_state
new_state
reason_code
raw_status
```

## Recovery Rule

Recovery must not depend on the failed subsystem.

The supervisor must retain enough visibility to distinguish:

- absent card
- invalid identity
- invalid discovery
- power fault
- thermal fault
- clock fault
- communication fault
- policy restriction
- revoked trust

## Ring Security Notes

Health observation is not the same as health control.

Passive health visibility may be delegated to lower rings by policy.

Health control, recovery entry, provisioning entry, and reset authority generally remain Ring -2 or Ring -1 functions.

## Related Documents

- supervisor-observability-contract-v1.md
- cpu-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md

<!-- AEMS-MIGRATED-SOURCE: docs/card-identity-eeprom.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/card-identity-eeprom.md -->
### Integrated source: `docs/card-identity-eeprom.md`

# ATARIX Card Identity EEPROM

## Status

Draft architecture specification.

This document defines the small nonvolatile identity device that should be present on each ATARIX card, and the minimum independent management objects expected on every card.

## Purpose

Each ATARIX card should include a small identity EEPROM that allows the supervisor, boot firmware, or fabric controller to identify the card installed in a slot before the main card logic is fully initialized.

The identity EEPROM provides a stable, low-power, early-boot source of information for:

- Card identity.
- Vendor and product identifiers.
- Hardware revision.
- Serial number.
- Board capabilities.
- Power requirements.
- Required firmware or bitstream versions.
- Discovery-record location.
- Manufacturing data.
- Recovery and safe-mode information.

## Historical Influences

The design is inspired by:

- NuBus declaration-ROM style self-description.
- EISA-style configuration records.
- Modern SPD EEPROMs used on memory modules.
- Server FRU inventory EEPROMs.

ATARIX should borrow the self-description idea, not copy any legacy format directly.

## Design Rule

A card should be identifiable before it is trusted.

A card should be identifiable before its main processor, FPGA, CPLD, or device logic is allowed to perform privileged fabric activity.

Attachment does not imply trust.

## Minimum Card Management Standard

Every ATARIX card should include a small independent management cluster that can be accessed before the card is fully trusted by the fabric.

These management objects should function independently from the card's main processing logic whenever practical.

A failed CPU, FPGA function, accelerator core, or service engine should not prevent the supervisor from reading identity, basic health, power, temperature, and fault state.

### Mandatory Management Objects

Every card should provide:

```text
Identity EEPROM / FRU EEPROM
Temperature Sensor
Voltage Monitor
Current Monitor
Status / Control Register
```

The status / control register should expose at minimum:

```text
READY
FAULT
DEGRADED
SERVICE_REQUEST
SELFTEST_PASS
SELFTEST_FAIL
WRITE_PROTECT_STATUS
LAST_RESET_REASON
```

### Recommended Management Objects

Most nontrivial cards should provide:

```text
Fan Tachometer
Fan Controller
Additional ADC Channels
Environmental Sensors
```

Additional ADC channels may monitor local rails, references, battery or supercapacitor rails, analog supplies, or sensor inputs.

Environmental sensors may include board temperature, inlet temperature, outlet temperature, humidity, or other card-relevant conditions.

Cards expected to dissipate significant power should treat fan tachometer, fan control, and thermal sensing as mandatory rather than merely recommended.

### Optional / High-Reliability Management Objects

Cards may provide:

```text
Secure Element
Calibration EEPROM
Management MCU
```

A secure element may support signed identity, secure boot, certificate storage, firmware authorization, or asset tracking.

A calibration EEPROM may store factory calibration, ADC calibration, temperature offsets, power-monitor calibration, board-specific tuning, and field-service data.

A management MCU may aggregate sensors, run card-local self-tests, supervise local resets, or communicate with the system supervisor.

### Independence Requirement

Each management object should fail independently where practical.

Examples:

- Identity EEPROM remains readable even if the main card logic is held in reset.
- Temperature and power monitors remain readable before card enable.
- Status / control register can report FAULT even when the main data plane is disabled.
- Write-protect state can be observed without booting the card.
- Calibration data can be read during maintenance or recovery mode.

A card that cannot report its own identity and basic health should not be granted normal operating authority.

## Relationship to Discovery Records

The identity EEPROM is not a replacement for the full discovery system.

Instead:

```text
Identity EEPROM
    Small, early, low-level, always readable where practical.

Discovery Record
    Larger, richer, versioned, fabric-visible system description.
```

The EEPROM may contain either:

1. A compact identity record directly, or
2. A pointer to a richer discovery record stored elsewhere, or
3. Both.

Possible richer discovery locations:

```text
On-card ROM
On-card flash
FPGA block RAM
Supervisor-provided table
Fabric-generated record
Boot-updated record cache
```

## Bus Interface

Recommended Rev A interface:

```text
I2C-compatible serial EEPROM
```

Rationale:

- Simple wiring.
- Low pin count.
- Easy supervisor access.
- Similar operational model to SPD EEPROMs.
- Easy read access before full card initialization.

The backplane should provide an identity-management path that can be accessed by the supervisor and/or fabric controller.

The identity-management path should also support the minimum card management objects where practical.

## Slot Addressing

The system must be able to distinguish EEPROMs and management objects by slot.

Possible approaches:

- Slot-specific I2C mux.
- Slot address pins into device address pins.
- Supervisor-controlled per-slot enable.
- Fabric-controlled identity bus arbitration.
- Per-slot management-bus isolation.

Rev A should prefer the simplest electrically reliable scheme.

## Write Policy

The identity EEPROM should contain manufacturing identity that is normally read-only in system operation.

Writable regions may be allowed for:

- Boot-generated cached discovery data.
- Calibration data.
- Board configuration.
- Field-service data.
- Last-known-good configuration pointer.

However, writes must be controlled.

Recommended policy:

```text
Manufacturing region: write-protected after programming.
System region: writable only by supervisor-authorized process.
Recovery region: writable only in explicit maintenance mode.
```

The normal operating system should not freely rewrite identity EEPROM contents.

## Identity Record v1

All multi-byte fields are little-endian and follow:

```text
docs/data-model-and-endianness.md
```

Recommended fixed header:

```text
Offset  Size  Field
+00     4     Magic: "ATID"
+04     1     Format Major Version
+05     1     Format Minor Version
+06     2     Header Length, u16
+08     2     Total Length, u16 or extended form
+0A     2     Header Checksum, u16
+0C     2     Vendor ID, u16
+0E     2     Product ID, u16
+10     2     Device Class, u16
+12     2     Device Subclass, u16
+14     2     Hardware Revision, u16
+16     2     Board Revision, u16
+18     8     Serial Number or Board ID, u64
+20     8     Capability Summary, u64
+28     8     Required Power Summary, u64
+30     8     Discovery Pointer or Resource ID, u64
+38     8     Manufacturing Timestamp, u64
+40     8     Reserved, must be zero
```

Minimum header size:

```text
72 bytes
```

The EEPROM may then contain TLV records.

## Standard TLV Records

Recommended TLV record form:

```text
Offset  Size  Field
+00     1     Type
+01     1     Flags
+02     2     Length, u16
+04     N     Value
```

For longer values, use a TLV type that explicitly contains a u32 or u64 length inside the value.

Recommended TLV types:

```text
$00 End
$01 Human-readable card name
$02 Manufacturer name
$03 Product name
$04 Firmware compatibility
$05 FPGA bitstream compatibility
$06 Power requirements
$07 Clock requirements
$08 Reset requirements
$09 Discovery record location
$0A Required capability classes
$0B Supported safe modes
$0C Diagnostic entry points
$0D Manufacturing data
$0E Field-service data
$0F Calibration data
$10 Boot cache pointer
$11 Management object inventory
$12 Thermal sensor description
$13 Power monitor description
$14 Fan controller description
$15 Environmental sensor description
$16 Secure element description
$17 Calibration EEPROM description
$80-$FF Vendor-specific
```

## Required Fields

Every card identity EEPROM should provide at minimum:

```text
Magic
Format version
Vendor ID
Product ID
Device class
Hardware revision
Serial number or board ID
Capability summary
Power requirement summary
Management object inventory
```

## Capability Summary

The capability summary is not a security grant.

It is only an early-boot hint describing what the card claims to support.

Actual access still requires capability authorization.

Examples:

```text
DMA capable
Mailbox capable
Interrupt capable
Memory service capable
Storage service capable
Accelerator capable
Supervisor visible
Fabric configuration required
Management sensor capable
Secure identity capable
Calibration data present
```

## Boot Flow

Recommended boot sequence:

```text
1. Supervisor powers identity and management bus.
2. Supervisor scans slots.
3. Supervisor reads each identity EEPROM.
4. Supervisor validates checksum and version.
5. Supervisor reads mandatory management objects.
6. Supervisor records slot inventory and health baseline.
7. Fabric controller receives slot identity summary.
8. Boot firmware queries fabric identity table.
9. Full discovery records are read or generated.
10. Capability broker assigns permissions.
11. Devices become usable only after authorization.
```

## Identity Fabric

The identity fabric is the early-boot inventory path that collects identity information from cards.

It may be implemented by:

- Supervisor firmware.
- Fabric-controller logic.
- A dedicated identity bus.
- A combination of supervisor and fabric support.

The identity fabric should expose a stable inventory table to boot firmware and diagnostics.

## Security Considerations

Identity is not authority.

A card may claim capabilities in its EEPROM, but the system must not grant access solely because the EEPROM says so.

Security rules:

1. Treat EEPROM data as untrusted until validated.
2. Verify checksums and versions.
3. Apply policy before granting capabilities.
4. Do not allow unrestricted EEPROM writes.
5. Record identity mismatches in supervisor logs.
6. Quarantine cards with invalid or inconsistent identity data where appropriate.
7. Treat sensor data as advisory unless validated by policy.
8. Secure elements may strengthen identity but do not replace capability authorization.

## Diagnostics

The supervisor and firmware monitor should support commands similar to:

```text
IDENT LIST
IDENT SHOW <slot>
IDENT RAW <slot>
IDENT VERIFY <slot>
IDENT CACHE REFRESH
HEALTH SHOW <slot>
SENSOR SHOW <slot>
POWER SHOW <slot>
FAN SHOW <slot>
CALIBRATION SHOW <slot>
```

## Relationship to Testing

Testing should include:

- Valid identity EEPROM parsing.
- Invalid checksum handling.
- Unknown version handling.
- Missing EEPROM handling.
- Conflicting discovery-record handling.
- Write-protection checks.
- Slot inventory stability.
- Mandatory management-object detection.
- Temperature sensor readout.
- Voltage and current monitor readout.
- Fan tachometer and fan controller behavior where present.
- Calibration EEPROM readout where present.
- Secure-element presence and failure handling where present.

See:

```text
docs/testing-strategy.md
```

## Design Principle

The identity EEPROM provides early, minimal, stable identity.

Management objects provide early, minimal, independent health information.

Discovery records provide rich, versioned system description.

Capabilities provide authority.

These must remain separate.

<!-- AEMS-MIGRATED-SOURCE: docs/core-doctrines.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/core-doctrines.md -->
### Integrated source: `docs/core-doctrines.md`

# ATARIX Core Doctrines

Status: Baseline

## Purpose

This document records the non-negotiable architectural doctrines that guide ATARIX design, implementation, simulation, testing, and hardware validation.

These doctrines are intentionally short. They are review rules, not implementation details.

## Doctrines

### 1. No Direct Hardware Access

Software components do not directly access protected hardware, registers, buses, slots, or physical addresses.

All protected access occurs through services, capabilities, resources, and implementation layers.

### 2. Discovery Grants Visibility

Discovery reveals that resources or services exist.

Discovery does not grant authority to use them.

### 3. Capabilities Grant Authority

Capabilities are the authorization mechanism for protected operations.

A request without a valid capability is denied.

### 4. Deny By Default

The default answer to every protected operation is DENY.

Access is allowed only after all applicable validation, trust, ring, ownership, delegation, revocation, expiration, resource, and operation checks pass.

### 5. Hardware Is An Implementation Detail

Software targets services, not hardware placement.

A service may be implemented in FPGA logic, a CPU card, a peripheral card, a simulator, or a future node without changing the public contract.

### 6. Failures Must Be Observable

Failures must be detected, classified, reported, and traceable.

Silent failure is not acceptable.

### 7. Transport Independence

Services must not depend on a specific transport unless the transport itself is the service contract.

Mailbox, local calls, simulated links, FPGA paths, and future inter-node transports should preserve the same semantics.

### 8. Revocation Must Be Immediate

A revoked capability ceases to authorize operations immediately.

Revocation overrides ownership, trust, and convenience.

### 9. Least Privilege

Capabilities and services grant only the authority required for the requested function.

Delegation may reduce authority only.

### 10. Security Before Convenience

Architectural security takes precedence over shortcuts, performance hacks, simulator convenience, or direct-access workarounds.

### 11. No Address-Based Authority

Knowledge of an address, slot, register, mailbox endpoint, handle, service name, or transport identifier grants zero authority.

Authority is conveyed only by valid capabilities evaluated by policy.

### 12. Simulator Equals Hardware

The simulator is a reference implementation, not a shortcut or mock that bypasses security.

Simulator security rules, capability checks, mailbox validation, and service boundaries must match the future hardware model.

### 13. Ring Security Is Mandatory

ATARIX uses ring-based security boundaries.

Higher-trust rings may supervise, delegate, constrain, or revoke authority from lower-trust rings.

Lower-trust rings may not directly command, inspect, mutate, bypass, or impersonate higher-trust rings.

Ring boundaries are enforced by Supervisor policy, capability checks, mailbox validation, service dispatch, and fabric routing rules.

Capabilities grant authority only within the maximum authority permitted by the requester's ring and the target's ring policy.

## Required Access Path

The permitted protected-access path is:

```text
Application
    -> Service
    -> Capability Policy
    -> Ring Policy
    -> Resource
    -> Implementation
```

The forbidden path is:

```text
Application
    -> Address
    -> Hardware
```

## Security Decision Chain

Protected operations must satisfy the full security chain:

```text
Deny by default
    -> Ring boundary check
    -> Capability validation
    -> Revocation check
    -> Mailbox validation
    -> Service policy
    -> Resource policy
    -> Authorized operation
```

Failure at any step denies the operation.

## Suggested Ring Model

The initial ATARIX ring model is:

```text
Ring 0: Supervisor / Root Authority
Ring 1: Fabric Management / Trusted System Services
Ring 2: Device Services / Card Firmware
Ring 3: Applications / User Workloads
Ring 4: Untrusted or Quarantined Nodes
```

The exact ring count and names may evolve by ADR, but the existence of ring boundaries is mandatory.

## Review Rule

Every significant ATARIX change should answer:

```text
Which doctrine does this preserve?
Which ADR justifies this behavior?
Which public API expresses the contract?
Which test or bench procedure enforces it?
```

If the answer is unclear, the design is not ready.

## Result

These doctrines define the ATARIX architectural baseline.

Future subsystems may extend the system, but they must not weaken these rules without an explicit ADR.

<!-- AEMS-MIGRATED-SOURCE: docs/cpu-card-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/cpu-card-architecture-v1.md -->
### Integrated source: `docs/cpu-card-architecture-v1.md`

# ATARIX CPU Card Architecture v1

## Status

Draft hardware architecture specification.

This document defines the first-generation ATARIX CPU card architecture. It is informed by the Vega816 CPU Shim, CPU Buffer, DMA Controller, and Vector Pull Rewrite KiCad projects, but adapts those ideas for the ATARIX workstation-style fabric architecture.

## Purpose

The CPU card provides the primary W65C816 execution environment while isolating the raw CPU bus from the system backplane.

The CPU card should be independently bring-up capable, diagnosable, and serviceable.

## Design Rule

The W65C816 local bus stays on the CPU card.

The backplane sees a structured ATARIX fabric interface, not every raw W65C816 signal.

The CPU card shall be able to self-boot into a recovery monitor, but when a valid ATARIX backplane BIOS is discovered and validated, the CPU card shall hand off system boot policy to the backplane BIOS.

## Architectural Position

```text
W65C816 CPU
    |
Local CPU Bus
    |
Local ROM / RAM / I/O decode
    |
CPU Shim and Buffer Logic
    |
CPU-Card Bridge FPGA or CPLD
    |
ATARIX Fabric Interface
    |
DIN41612 Backplane
```

## Design Goals

1. Preserve simple W65C816 bring-up.
2. Keep the CPU bus observable.
3. Avoid exposing the full raw CPU bus across the backplane.
4. Support Vega816-derived CPU shim and buffer concepts.
5. Support vector-pull rewrite or equivalent interrupt assist logic.
6. Support wait-state generation and CPU pausing through RDY.
7. Support local boot even when the wider fabric is unavailable.
8. Support fabric-mediated DMA, discovery, interrupts, and mailboxes.
9. Prepare for future secondary CPU cards.
10. Support backplane BIOS discovery and handoff.
11. Populate the W65C816 native address space with the maximum practical CPU-local RAM.

## Required Functional Blocks

### W65C816 CPU

Primary processor:

```text
WDC W65C816S
```

Required CPU signals to preserve locally:

```text
A0-A23
D0-D7 or D0-D15 depending on board strategy
RWB
PHI2
RDY
IRQB
NMIB
ABORTB
RESB
VDA
VPA
VPB
MLB
BE
E
MX
```

The CPU card should expose these signals for diagnostics, but they are not all backplane signals.

### Local ROM / Bootstrap ROM

The CPU card should contain local boot ROM or a reliable ROM mapping path.

ROM responsibilities:

- Reset vector.
- Minimal CPU-card initialization.
- ROM monitor.
- Serial/network loader support.
- Recovery diagnostics.
- Basic memory and fabric tests.
- Backplane BIOS discovery.
- Backplane BIOS validation.
- Backplane BIOS handoff.

The CPU-card ROM is not the final system BIOS when a valid backplane BIOS is present.

Instead:

```text
CPU-Card ROM
    Local bootstrap and recovery authority.

Backplane BIOS
    System boot and platform policy authority after discovery and validation.
```

ROM may later become selectable between:

```text
Normal ROM
Golden ROM
Network-loaded image
Supervisor-selected image
```

### Backplane BIOS Handoff

At boot, the CPU-card ROM should determine whether it is running as:

```text
Standalone CPU-card recovery environment
```

or:

```text
CPU card installed in an ATARIX backplane
```

If a valid backplane BIOS is discovered, the CPU-card ROM should subordinate system boot policy to it.

Handoff requirements:

```text
Backplane BIOS identity verified
Backplane BIOS checksum or signature validated where practical
Supervisor state queried
Fabric identity queried
Memory baseline established
Recovery override checked
```

If validation fails, the CPU-card ROM must remain capable of entering recovery mode.

This design allows a CPU card to be self-testable outside the system while still allowing the backplane to own platform policy inside a complete ATARIX machine.

### Local RAM

The CPU card should provide the maximum practical CPU-local RAM directly addressable by the W65C816.

Target:

```text
16 MiB CPU-local addressable RAM
```

Rationale:

```text
The W65C816 exposes a 24-bit native address space.
The CPU card should not artificially constrain that address space below 16 MiB where practical.
```

The CPU card should still preserve deterministic regions for:

- Stack.
- Direct page.
- Monitor work area.
- Upload buffer.
- Netboot staging.
- Diagnostics.
- Recovery operation.

The first CPU card should be useful even if no external memory card exists.

Implementation options may include:

```text
SRAM
Pseudo-SRAM
SDRAM behind bridge logic
Mixed fast SRAM plus larger bridge-managed RAM
```

The architecture requires the addressability target, not a specific memory technology.

### Bank Zero Isolation

Each CPU card should own its own bank-zero environment.

Bank zero includes:

```text
$000000-$00FFFF
```

This preserves:

- Direct page behavior.
- Stack behavior.
- Interrupt-vector assumptions.
- ROM monitor assumptions.
- Per-CPU local execution state.

Future multi-CPU systems must not require every CPU to share the same physical bank zero.

### CPU Shim / Bridge Logic

The CPU card should include shim or bridge logic inspired by Vega816.

Responsibilities:

- CPU bus observation.
- Address decoding assistance.
- Wait-state insertion.
- RDY control.
- Local versus fabric access decision.
- Vector Pull Rewrite support if implemented.
- Fabric request generation.
- Debug-state reporting.
- Backplane BIOS aperture support.

This may be implemented with:

```text
Discrete logic for Rev A simplicity
CPLD
Small FPGA
```

The preferred long-term implementation is FPGA or CPLD-based.

### CPU Buffering

The card should buffer CPU bus signals where useful.

Goals:

- Improve signal integrity.
- Permit debug observation.
- Isolate CPU timing from downstream logic.
- Support controlled DMA or bridge access.

The Vega816 CPU Buffer project is the reference inspiration for this block.

### Vector Pull Rewrite

The CPU card should reserve support for Vector Pull Rewrite or an equivalent interrupt-assist mechanism.

Purpose:

- Detect W65C816 vector fetches.
- Support expanded interrupt source handling.
- Permit fabric interrupt-controller integration.
- Reduce interrupt dispatch ambiguity.

This should be documented further in:

```text
docs/vector-pull-rewrite-v1.md
```

### Fabric Bridge

The CPU-card bridge translates CPU activity into ATARIX fabric transactions.

Supported transaction types:

```text
Register Read
Register Write
Mailbox Send
Mailbox Receive
Discovery Read
Interrupt Acknowledge
DMA Request Submission
Fault Report Read
Backplane BIOS Read
```

The bridge should be able to stall the CPU with RDY while a fabric transaction completes.

## Local Address Map

The CPU card should implement a local map compatible with the Rev A memory model while targeting the full W65C816 native address space.

Candidate Rev A local map:

```text
$000000-$0000FF  Direct page
$000100-$0001FF  Stack
$000200-$00BFFF  Local RAM / monitor workspace
$00C000-$00DFFF  Local and fabric I/O aperture
$00E000-$00FFFF  ROM / vectors
$010000-$EFFFFF  CPU-local RAM
$F00000-$F7FFFF  Fabric / service windows, configurable
$F80000-$FBFFFF  Backplane BIOS aperture, configurable
$FC0000-$FFFFFF  Recovery ROM / vectors / high monitor region, configurable
```

Exact boundaries remain subject to memory-map specification and hardware decode constraints.

Fabric windows and backplane BIOS windows may be remapped or bank-selected by the bridge logic.

See:

```text
docs/memory-map.md
docs/address-space-architecture.md
docs/backplane-bios-v1.md
```

## Fabric Interface

The CPU card should expose a fabric-facing interface rather than a raw CPU bus.

Required logical groups:

```text
Power
Ground
Clock Reference
Reset Control
Management Plane
Control Plane
Event Plane
Data Plane
Discovery Interface
Backplane BIOS Interface
Diagnostic Signals
```

The exact connector pinout is deferred to:

```text
docs/backplane-pinout-v1.md
```

## Network Support

For Rev A, a network service path is mandatory because netboot and NTP are baseline capabilities.

Acceptable Rev A implementation:

```text
W5500 Ethernet Controller on CPU card
```

Alternative Rev A implementation:

```text
Dedicated boot-critical network service card
```

Requirement:

```text
The base Rev A system must not depend on a missing optional network card in order to perform netboot.
```

Recommended early network controller:

```text
W5500 Ethernet Controller
```

Reason:

- Simplifies TCP/IP burden.
- Supports netboot.
- Supports NTP.
- Supports remote diagnostics.
- Reduces dependency on a separate service card during bring-up.

Later revisions may move networking to a service card, but Rev A must provide a concrete network service path.

See:

```text
docs/netboot-ntp-v1.md
```

## Diagnostic Requirements

The CPU card must be highly observable.

Required diagnostic access:

```text
CPU Clock
CPU Reset
RDY
IRQB
NMIB
ABORTB
RWB
VDA
VPA
VPB
MLB
ROM Chip Select
RAM Chip Select
I/O Chip Select
Fabric Access Active
Backplane BIOS Access Active
Vector Pull Rewrite Active
Bank Zero Select
```

Recommended diagnostic access:

```text
Buffered address bus sample header
Buffered data bus sample header
Logic analyzer control-signal header
Power rail test points
Ground probe loops
CPU running LED
Wait-state LED
IRQ LED
Fault LED
Backplane BIOS handoff LED or status bit
```

See:

```text
docs/diagnostic-access-v1.md
```

## Reset and Clocking

The CPU card accepts reset control from the supervisor or backplane reset tree.

Required reset modes:

```text
Power-On Reset
Supervisor Reset
Manual Reset
Slot Reset
Recovery Hold-In-Reset
Software-Requested Reset
```

Clock modes:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Normal
14 MHz Experimental
Single-Step or Gated Debug Clock
```

See:

```text
docs/clock-reset-spec.md
```

## Interrupt Model

The CPU card receives normal IRQ/NMI/ABORT signaling from local logic and the fabric interrupt controller.

Expected behavior:

- IRQB handles normal maskable events.
- NMIB is reserved for severe or recovery-class events.
- ABORTB is reserved for future protection and bus-fault experiments.
- Vector Pull Rewrite may assist dispatch.

See:

```text
docs/interrupt-architecture-v1.md
```

## DMA Model

The CPU card does not grant unrestricted raw bus-master access to external cards.

DMA should be mediated by the fabric controller.

CPU-card-local DMA concepts from Vega816 may be useful, but long-term ATARIX DMA must follow the capability model.

See:

```text
docs/dma-engine-v1.md
docs/capability-model.md
```

## Discovery Record

The CPU card should expose a discovery record describing:

```text
Vendor ID
Device ID
CPU type
Card revision
ROM version
Local RAM size
Supported clock modes
Supported interrupt features
Supported fabric features
Backplane BIOS handoff support
Diagnostic capability flags
```

See:

```text
docs/discovery-rom-format.md
```

## Management Plane

The CPU card should include a low-speed management interface to the supervisor card.

Candidate functions:

- CPU heartbeat.
- Reset reason reporting.
- Temperature or voltage reporting if sensors exist.
- ROM version readout.
- Fault-state reporting.
- Hold-in-reset support.
- Backplane BIOS handoff status.

This management interface must remain useful even when the main CPU is not booting.

See:

```text
docs/card-identity-eeprom.md
```

## Relationship to Vega816

Keep:

- CPU shim idea.
- CPU buffer idea.
- RDY-based wait-state and pause control.
- Vector Pull Rewrite idea.
- Bank-zero awareness.
- Explicit DMA request/grant thinking.

Modify:

- BB816 connector assumptions.
- Raw shared-bus multiprocessing.
- Direct DMA-channel wiring.
- Interrupt routing details.

Avoid:

- Exposing the full CPU bus as the long-term system bus.

See:

```text
docs/vega816-analysis.md
```

## Initial Rev A CPU Card Recommendation

The first ATARIX CPU card should contain:

```text
W65C816S
16 MiB target CPU-local addressable RAM
Local bootstrap / recovery ROM or flash
CPU-local bus buffer/shim logic
Small FPGA or CPLD bridge
W5500 Ethernet or equivalent boot-critical network path
UART console path
Diagnostic headers and test points
Card identity EEPROM and management sensor cluster
DIN41612 backplane connector
```

## Open Questions

- Exact local RAM technology and topology.
- Exact ROM or flash part.
- Exact backplane BIOS aperture and validation scheme.
- Whether the first bridge is CPLD, small FPGA, or discrete logic.
- Whether W5500 lives on Rev A CPU card or a boot-critical service card.
- Exact management-plane electrical interface.
- Exact DIN41612 pin usage.
- Whether Vector Pull Rewrite is mandatory in Rev A or reserved for Rev B.
- Whether the CPU card includes its own oscillator or derives clock from the backplane.
- Whether local RAM is always bank zero or can be remapped by the bridge.

## Design Principle

The CPU card is not the whole computer.

It is a deterministic W65C816 execution node attached to a workstation-style ATARIX fabric.

It can recover itself locally, but in a complete system it should recognize and hand off platform boot policy to the backplane BIOS.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-002-Architectural-Invariants.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-002-Architectural-Invariants.md -->
### Integrated source: `docs/design/ATX-DESIGN-002-Architectural-Invariants.md`

# ATX-DESIGN-002: Architectural Invariants

## Purpose

This document records architectural properties that must remain true unless explicitly changed by formal architecture review.

## Initial Invariants

- Object identity is never authority.
- Authority derives from capabilities and policy.
- Ambient authority is prohibited.
- Compatibility personalities must not weaken native security semantics.
- Inter-object communication occurs through defined transport mechanisms.
- State transitions must be auditable.
- Hardware acceleration may speed checks but must not bypass checks.
- Engineering changes require specification, review, verification, and gate evidence.

## Relationship To ATX-100

Stable invariants should be integrated into ATX-100 as canonical architectural constraints.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-006-Performance-Philosophy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-006-Performance-Philosophy.md -->
### Integrated source: `docs/design/ATX-DESIGN-006-Performance-Philosophy.md`

# ATX-DESIGN-006: Performance Philosophy

## Purpose

This document defines how Atarix evaluates and applies performance optimizations.

## Principle

Performance is valuable only when it preserves architecture semantics.

Optimizations may reduce latency, memory use, copy cost, or CPU overhead. They must not weaken authority, policy, audit, recovery, human readability, or compatibility behavior.

## Initial Rules

- Acceleration is not authority.
- Cache state is not authority.
- Reachability is not permission.
- Fast paths must preserve slow-path semantics.
- Benchmarks should include correctness and auditability evidence.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-007-Security-Philosophy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-007-Security-Philosophy.md -->
### Integrated source: `docs/design/ATX-DESIGN-007-Security-Philosophy.md`

# ATX-DESIGN-007: Security Philosophy

## Purpose

This document defines the security philosophy behind Atarix.

## Principle

Atarix is deny-by-default and capability-oriented.

Names, handles, addresses, descriptors, cache entries, and lookup results may identify or accelerate access. They do not grant authority.

## Initial Security Rules

- Authority must be explicit.
- Capability checks must precede protected operations.
- Policy may further restrict capability-derived authority.
- Hardware may accelerate validation but may not bypass validation.
- Compatibility personalities must map foreign authority models into native Atarix authority rules.

<!-- AEMS-MIGRATED-SOURCE: docs/development-philosophy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/development-philosophy.md -->
### Integrated source: `docs/development-philosophy.md`

# ATARIX Development Philosophy

## Purpose

This document defines how ATARIX is developed.

Architecture decisions are influenced by four major traditions:

- Vega816 and BB816 practical 65C816 engineering
- Sun/NuBus/UPA workstation architecture
- curl development discipline
- OpenBSD security philosophy

## Documentation First

Architecture first.

Specification second.

Implementation third.

No major hardware, FPGA, firmware, or operating-system component should exist without a corresponding specification.

## Version Everything

Interfaces are versioned.

Examples:

- Discovery Format v1
- Mailbox Protocol v1
- DMA Descriptor v1
- Fabric Controller v1

Existing interfaces should not be silently changed.

## Small Changes

Prefer:

- Small commits
- Small pull requests
- Small reviews

Large architectural changes should be decomposed into reviewable units.

## Observable Systems

Every subsystem should expose:

- Status
- Counters
- Fault history
- Health metrics
- Recovery information

Debuggability is a first-class design goal.

## Security by Design

Borrowing from OpenBSD:

- Secure by default
- Least privilege
- Privilege separation
- Capability revocation
- Explicit trust boundaries
- Cryptography by reuse, not invention

## Cryptographic Standards

Preferred primitives:

- Ed25519 for signatures
- ChaCha20-Poly1305 for authenticated encryption
- BLAKE2b and SHA-256 for hashing

Avoid custom cryptography.

## Recovery First

Every subsystem should define:

- Failure modes
- Recovery behavior
- Reset behavior
- Diagnostic visibility

The machine should fail visibly and recover deliberately.

## Architecture Decision Records

Major decisions should be documented as ADRs.

Examples:

- Why ULX3S was selected
- Why RP2350 was selected
- Why the CPU bus remains local
- Why capabilities are used
- Why the supervisor is outside the ring model

## Ring and Capability Model

ATARIX uses:

- Rings for execution authority
- Capabilities for resource authority

Neither mechanism is sufficient alone.

## Design Principle

Prefer systems that are understandable, observable, auditable, recoverable, and secure over systems that are merely clever.

<!-- AEMS-MIGRATED-SOURCE: docs/dma-engine-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/dma-engine-v1.md -->
### Integrated source: `docs/dma-engine-v1.md`

# ATARIX DMA Engine v1

## Status

Draft specification for the ATARIX DMA subsystem.

This document defines DMA operation, descriptor formats, ownership rules, capability integration, fault handling, interrupt behavior, and the distinction between W65C816-native transfers and fabric/object-space transfers.

## Purpose

The DMA engine provides controlled movement of data between CPU-local memory, memory services, devices, FPGA services, storage services, and future accelerators.

Unlike traditional bus-mastering systems, DMA is not automatically trusted.

DMA operations require authorization.

## Data Model

DMA descriptors follow:

```text
docs/data-model-and-endianness.md
```

All multi-byte fields are little-endian.

The W65C816 uses u24 CPU-local addresses, but ATARIX DMA must also support u64 resource identifiers, u64 offsets, u64 object sizes, and future 32-bit / 64-bit CPU participants.

## Design Goals

1. Eliminate unrestricted DMA.
2. Permit future accelerators.
3. Support mailbox-driven operation.
4. Support capability validation.
5. Remain practical for a W65C816-based system.
6. Permit future FPGA enforcement.
7. Avoid treating u24 CPU-local addresses as universal system addresses.
8. Support large memory-service and storage-service objects without changing the core security model.

## DMA Architecture

The DMA engine is initially implemented as an FPGA fabric service.

The engine performs transfers on behalf of authorized requestors.

Requestors may include:

```text
W65C816 CPU Card
Future 32-bit CPU Card
Future 64-bit CPU Card
Supervisor
Network Service
Storage Service
Memory Service
Accelerator
Future Devices
```

## DMA Channels

Initial implementation:

```text
4 logical channels
```

Future revisions may increase channel count.

## Transfer Types

Supported transfers:

```text
CPU-Local Memory -> CPU-Local Memory
CPU-Local Memory -> Device
Device -> CPU-Local Memory
Memory Object -> CPU-Local Window
CPU-Local Window -> Memory Object
Memory Object -> Device
Device -> Memory Object
Memory Object -> Accelerator
Accelerator -> Memory Object
Memory Object -> Memory Object
```

Unsupported transfers must return an error.

## Descriptor Classes

ATARIX defines two descriptor classes:

```text
Native Descriptor
    Compact descriptor for W65C816-native u24 CPU-local transfers.

Fabric Descriptor
    Extended descriptor for fabric resources, memory objects, storage extents, accelerators, and future CPU cards.
```

Native descriptors are useful for early bring-up and local CPU-card transfers.

Fabric descriptors are required for large memory, storage, accelerator, and future multi-processor transfers.

## Native Descriptor Format

Version 1 native descriptor:

```text
Offset  Size  Field
+00     1     Descriptor Type = $01
+01     1     Flags
+02     1     Channel
+03     1     Priority
+04     3     Source CPU Address, u24
+07     3     Destination CPU Address, u24
+0A     4     Length, u32
+0E     2     Reserved
```

Descriptor size:

```text
16 bytes
```

Native addresses are W65C816 CPU-local addresses.

Native descriptor addresses are not universal ATARIX fabric addresses.

## Fabric Descriptor Format

Version 1 fabric descriptor:

```text
Offset  Size  Field
+00     1     Descriptor Type = $02
+01     1     Flags
+02     1     Channel
+03     1     Priority
+04     8     Source Resource ID, u64
+0C     8     Source Offset, u64
+14     8     Destination Resource ID, u64
+1C     8     Destination Offset, u64
+24     4     Length, u32 or extended-length escape
+28     8     Capability Reference, u64
+30     8     Transaction ID, u64
```

Descriptor size:

```text
56 bytes
```

The source and destination resource IDs may refer to:

- CPU-local memory apertures.
- Memory-service objects.
- Storage-service extents.
- Device buffers.
- Accelerator buffers.
- Fabric-managed queues.

## Extended Length Rule

The fabric descriptor length field normally uses u32.

```text
Length != $FFFFFFFF
    Length is encoded directly as u32.

Length == $FFFFFFFF
    Actual length is supplied by an associated extended descriptor or descriptor-chain record using u64.
```

Formats expected to represent persistent large objects should use u64 sizes in the owning object metadata.

DMA transfer lengths may remain u32 for common-case transfers, because very large transfers should usually be split into bounded operations for scheduling, watchdog, fault isolation, and recovery.

## Descriptor Flags

```text
Bit 0 Generate Completion Interrupt
Bit 1 Verify Capability
Bit 2 Source Is CPU-Local Address
Bit 3 Destination Is CPU-Local Address
Bit 4 Uses Extended Length
Bit 5 Scatter/Gather Descriptor
Bit 6 Reserved
Bit 7 Reserved
```

## Capability Validation

DMA operations require:

1. Valid DMA capability.
2. Valid source-resource capability.
3. Valid destination-resource capability.
4. Matching ownership chain.
5. Ring/security policy allowing the request.

Failure of any validation step causes immediate rejection.

Capability references are u64.

## DMA Lifecycle

### Step 1

Requestor creates a native or fabric descriptor.

### Step 2

Descriptor is submitted through mailbox service or a descriptor queue.

### Step 3

DMA engine validates descriptor format.

### Step 4

DMA engine validates capabilities.

### Step 5

DMA engine validates source and destination ranges.

### Step 6

Transfer executes.

### Step 7

Completion status is recorded.

### Step 8

Interrupt or mailbox completion is generated if requested.

## Register Interface

See:

```text
docs/register-map-v1.md
```

DMA block base:

```text
$00D300
```

Minimal native-register interface:

```text
+00 Control
+01 Status
+02 Source Address Low
+03 Source Address Mid
+04 Source Address High
+05 Destination Address Low
+06 Destination Address Mid
+07 Destination Address High
+08 Length Byte 0
+09 Length Byte 1
+0A Length Byte 2
+0B Length Byte 3
```

Future revisions may use descriptor queues instead of direct registers.

Register blocks exposing 32-bit or 64-bit fields must define snapshot and commit semantics in `docs/register-map-v1.md`.

## Status Codes

```text
$00 Idle
$01 Pending
$02 Active
$03 Complete
$04 Invalid Descriptor
$05 Invalid Capability
$06 Invalid Address
$07 Access Denied
$08 Timeout
$09 Internal Error
$0A Unsupported Descriptor Type
$0B Invalid Resource ID
$0C Range Exceeds Resource
```

## Interrupt Behavior

DMA completion may generate:

```text
DMA Complete
DMA Fault
DMA Timeout
```

Interrupt routing is controlled through the fabric interrupt controller.

## Ownership Rules

A requestor may only submit DMA work for resources it controls.

Examples:

Allowed:

```text
Network Service -> Network Buffer
Memory Service -> Granted CPU Window
Accelerator -> Granted Result Buffer
```

Denied:

```text
Network Service -> Arbitrary Kernel Memory
Accelerator -> CPU-local Bank Zero
Unknown Device -> Memory Service Object
```

## Accelerator Integration

Accelerators never receive unrestricted DMA authority.

Instead:

1. Memory capability issued.
2. DMA capability issued.
3. Job executed.
4. Results returned.
5. Capabilities revoked.

## Fault Reporting

DMA faults should report:

```text
Requestor
Descriptor Type
Capability Reference, u64
Source Resource ID or CPU Address
Destination Resource ID or CPU Address
Length
Transaction ID, if present
Fault Code
Timestamp, if available
```

Fault reports should be accessible through mailbox services and supervisor diagnostics.

## Future Enhancements

Potential future features:

```text
Scatter/Gather
Descriptor Queues
Priority Scheduling
Rate Limiting
Capability Caching
FPGA Enforcement Tables
u64 Extended-Length Descriptor Records
IOMMU-like Resource Windows
```

## Design Rule

DMA exists to move data.

DMA does not confer authority.

Authority is provided only through capabilities.

Native u24 DMA is a compatibility and bring-up path, not the full ATARIX memory model.

## Open Questions

- Final descriptor queue design.
- Maximum single transfer size.
- Channel scheduling algorithm.
- Capability lookup mechanism.
- Hardware enforcement strategy.
- Accelerator-specific DMA extensions.
- Whether extended-length transfers should be required to split at watchdog-safe boundaries.
- Whether resource IDs are assigned by fabric, supervisor, or capability broker.

<!-- AEMS-MIGRATED-SOURCE: docs/doctrine/ARC-REVIEW.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/doctrine/ARC-REVIEW.md -->
### Integrated source: `docs/doctrine/ARC-REVIEW.md`

# Atarix Architecture Review Doctrine

## Purpose

This document defines the review process used to evaluate Atarix architecture and implementation changes.

Architecture review exists to prevent implementation details from becoming accidental doctrine.

## Required Review Sections

Every architecture proposal and implementation pull request shall include:

```text
Affected Pillars:
Modified Pillars:
Relevant ARC Checks:
Security Doctrine Compliance:
Lifecycle and Cleanup Impact:
Authority Impact:
Persistence Impact:
Required Tests:
Known Exceptions:
```

## Pre-Code Review

Before implementation begins, answer:

1. Does this feature create authority?
2. Does this feature transfer authority?
3. Does this feature persist authority?
4. Does this feature expose authority?
5. Does this feature create new attack surface?
6. Does this feature create objects?
7. Does this feature create garbage?
8. Does this feature survive reboot?
9. Does this feature survive uninstall?
10. Does this feature comply with ARC-SEC?

If any answer is unknown, the specification must be written or updated before code is written.

## Human Comprehensibility

A system administrator should be able to answer:

- What is this object?
- Who owns it?
- Why does it exist?
- What authority does it possess?
- How do I remove it?

Human-readable namespace and object observability are security features.

## Implementation Detail Rule

Implementation details shall not be exposed as public contracts.

Examples include table slots, memory addresses, mailbox numbers, internal indexes, CPU-local pointers, physical node addresses, and raw device locations.

<!-- AEMS-MIGRATED-SOURCE: docs/doctrine/ARC-SEC.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/doctrine/ARC-SEC.md -->
### Integrated source: `docs/doctrine/ARC-SEC.md`

# Atarix Security Doctrine

## Status

Normative draft v0.1

## Purpose

This document defines the core security doctrine for Atarix. All specifications, subsystems, services, protocols, runtimes, and applications shall be reviewed against this doctrine.

The purpose of this doctrine is not merely to prevent compromise. The purpose is to ensure that compromise does not automatically become authority.

## Core Security Philosophy

Atarix assumes:

- Software defects exist.
- Remote code execution will occur.
- Services may be compromised.
- User identities may be stolen.
- Internal nodes are not inherently trusted.
- Networks are hostile.
- Cleanup failures become security failures.

Therefore, Atarix protects authority, not merely code.

## ARC-SEC Rules

1. Authority must never be inferred from execution.
2. Connectivity does not imply trust.
3. Compromise is expected and must be containable.
4. No ambient authority.
5. Explicit lifecycle.
6. No orphans.
7. Leases by default.
8. Crash recovery is architectural.
9. Persistence must be requested.
10. Authority cleanup.
11. Garbage must be observable.
12. The system is not exempt.
13. Minimize attack surface.
14. No privilege escalation by identity.
15. Identity theft is not authority theft.
16. Secure by default.
17. Fail closed.
18. Verifiable state.
19. Reproducible recovery.
20. Least persistence.

## Summary

- Authority is explicit.
- Trust is explicit.
- Ownership is explicit.
- Persistence is explicit.
- Cleanup is explicit.
- System objects are not exempt.
- Identity is not authority.
- Connectivity is not trust.
- Compromise is expected.
- Containment is mandatory.

<!-- AEMS-MIGRATED-SOURCE: docs/engineering/Engineering-Gate.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/engineering/Engineering-Gate.md -->
### Integrated source: `docs/engineering/Engineering-Gate.md`

# Engineering Gate

## Status

Draft v0.2

## Purpose

The Atarix Engineering Gate is the single validation entry point for repository health.

Local command:

```sh
bash scripts/engineering/run_gate.sh
```

GitHub Actions workflow:

```text
.github/workflows/engineering-gate.yml
```

## Current Gate Phases

```text
1. Environment report
2. Repository structure checks
3. RTL file inventory checks
4. Architecture text invariant checks
5. Public header self-compile in C
6. Public header self-compile in C++
7. Warning-as-error enforcement for public headers
8. Verilator RTL lint
9. ATX-SPEC-020 RTL simulation gate
10. Artifact and summary generation
```

## Required Invariants

The gate currently verifies these architecture invariants:

```text
Lookup acceleration is not authority.
Every accelerated lookup must be reconstructable inside a bounded audit window.
POSIX is a compatibility personality.
```

## Current Build Tools

```text
gcc
g++
iverilog
vvp
verilator
```

## Current Gate Scripts

```text
scripts/engineering/run_gate.sh
scripts/engineering/check_headers.sh
scripts/engineering/check_rtl_lint.sh
scripts/rtl/run_atx_spec_020_sims.sh
```

## Current Artifacts

```text
build/engineering-gate/artifacts/summary.txt
build/engineering-gate/artifacts/header-checks.log
build/engineering-gate/artifacts/verilator-lint.log
build/engineering-gate/artifacts/atx_spec_020_rtl/
```

The nested RTL artifact directory may include:

```text
summary.txt
atx_spec_020_accelerator_tb.log
atx_spec_020_modules_tb.log
atx_spec_020_accelerator_tb.vcd
atx_spec_020_modules_tb.vcd
```

## Header Gate

Every public header under:

```text
include/atarix/*.h
```

is compiled independently in C and C++ mode.

The intent is to catch:

```text
missing includes
include-order dependency bugs
C/C++ ABI wrapper mistakes
warnings in public headers
undeclared public types
```

## RTL Gate

The RTL gate currently runs:

```text
Verilator lint
Icarus Verilog accelerator simulation
Icarus Verilog module validation simulation
```

The RTL simulation validates:

```text
authorization gates
audit-before-response behavior
SIMD-style control-byte probe path
scalar baseline probe path
Krapivin stepper math
Elias-Fano bounded decode
audit log window readback
```

## Extension Plan

Future gate phases should add:

```text
Markdown link validation
Architecture review reference validation
C source compilation
Unit tests
ECP5 synthesis smoke test
Timing report generation
Security invariant tests
Policy invariant tests
Recovery invariant tests
Performance regression thresholds
Randomized RTL tests
Coverage reporting
```

## Rule

The gate should become stricter over time, but it should remain one command.

```text
One project.
One gate.
One answer.
```

<!-- AEMS-MIGRATED-SOURCE: docs/hardware/bench-validation-plan-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/hardware/bench-validation-plan-v1.md -->
### Integrated source: `docs/hardware/bench-validation-plan-v1.md`

# Hardware Bench Validation Plan v1

Status: Draft

## Purpose

This plan defines the first ATARIX physical validation sequence.

The goal is not to boot the W65C816 first.

The goal is to prove the Supervisor-to-Fabric architecture on the bench before expensive CPU-card integration begins.

Primary target:

```text
RP2350 Supervisor
        <->
ECP5 Fabric
```

## Architectural Requirements

Bench validation must preserve the ATARIX doctrines:

```text
Supervisor owns trust.
Fabric enforces trust.
Everything is a service.
Discovery advertises.
Directory describes.
Capabilities authorize.
No direct hardware access.
Addresses are implementation details.
Deny by default.
Knowledge is not authority.
```

A hardware test is not complete merely because communication works.

It must also avoid creating bypass paths around the service and capability model.

## Phase 0: Bench Infrastructure

Goal:

```text
Establish reliable programming, power, measurement, and recovery tools.
```

Verify:

```text
ULX3S powers reliably.
RP2350 powers reliably.
USB serial access works.
Programming chain works.
Logic analyzer captures expected signals.
Oscilloscope probes expected rails and clocks.
Bench supply current limits are set.
Grounding is known and stable.
```

Success criteria:

```text
Both devices can be reflashed repeatedly.
Power cycling is repeatable.
Serial console is available.
Logic analyzer capture is repeatable.
```

## Phase 1: Supervisor Bring-Up

Goal:

```text
RP2350 boots and owns basic machine-control responsibilities.
```

Tests:

```text
Heartbeat LED
Serial boot banner
Watchdog setup
Reset output assertion
Reset output release
Power-good input readback
Fault input readback
```

Measurements:

```text
Boot time
Reset pulse width
Watchdog timeout
Power-good debounce time
Heartbeat period
```

Success criteria:

```text
Supervisor can boot, report status, assert reset, release reset, and recover from watchdog timeout.
```

## Phase 2: Fabric Bring-Up

Goal:

```text
ECP5 configures and becomes visible to Supervisor.
```

Minimum Fabric register block:

```text
0x0000 FABRIC_ID
0x0004 FABRIC_VERSION
0x0008 BUILD_ID
0x000C STATUS
0x0010 SCRATCH0
0x0014 SCRATCH1
0x0018 HEARTBEAT
0x001C FAULT_STATUS
```

Tests:

```text
Configuration done indication
Version register read
Build register read
Scratch register write
Scratch register readback
Heartbeat register changes
Fault status read
```

Success criteria:

```text
Supervisor can read and write the minimal Fabric register block repeatedly without protocol failure.
```

## Phase 3: Supervisor-Fabric Mailbox Validation

Goal:

```text
Establish reliable packet transport between Supervisor and Fabric.
```

Candidate mailbox header:

```c
struct atarix_mailbox_header_v1 {
    uint16_t type;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
};
```

Initial message types:

```text
PING
PONG
READ_REGISTER
WRITE_REGISTER
STATUS_QUERY
STATUS_REPLY
RESET_REQUEST
FAULT_REPORT
DISCOVERY_RECORD_PUSH
CAPABILITY_EVALUATE
```

Tests:

```text
Send packet
Receive packet
Sequence increment
CRC validation
Timeout handling
Retry handling
Duplicate packet handling
Out-of-order packet handling
Oversized packet rejection
Malformed length rejection
```

Fault injection:

```text
Drop packet
Duplicate packet
Corrupt CRC
Corrupt length
Corrupt sequence
Reset Fabric mid-transaction
Reset Supervisor mid-transaction
Hold Fabric busy
Send unknown message type
```

Success criteria:

```text
100,000 packets exchanged with zero unhandled protocol failures.
All injected faults are detected and reported.
No malformed packet grants authority or causes unsafe hardware action.
```

## Phase 4: Discovery Hardware Validation

Goal:

```text
Transport and validate Discovery records over the Supervisor-Fabric link.
```

Synthetic records:

```text
UART Service
GPIO Service
RTC Service
Fabric Status Service
Supervisor Health Service
```

Tests:

```text
BEGIN record accepted
SERVICE record accepted
END record accepted
CRC checked
Enumeration works
Iteration works
Lookup by handle works
Duplicate handles rejected
Missing END rejected
Invalid length rejected
Unknown record handled safely
```

Security requirement:

```text
Discovery reveals existence only.
Discovery does not grant authority.
```

Success criteria:

```text
Hardware transport can carry Discovery records equivalent to the structures covered by CI tests.
Discovery-only access cannot operate a protected resource.
```

## Phase 5: Capability Policy Hardware Validation

Goal:

```text
Exercise Capability Policy through the hardware transport before CPU-card integration.
```

Synthetic principals:

```text
Supervisor
Fabric
Kernel
Driver
Service
Application
```

Tests:

```text
Valid capability allowed
Invalid capability denied
Owner mismatch denied
Ring violation denied
Delegation violation denied
Revoked capability denied
Expired capability denied
Unknown handle denied
Unsupported operation denied
```

Security validation:

```text
Discovery only        -> DENY
Directory only        -> DENY
Known handle only     -> DENY
Known address only    -> DENY
Valid capability      -> EVALUATE
Invalid capability    -> DENY
Revoked capability    -> DENY
Expired capability    -> DENY
```

Success criteria:

```text
The bench proves that Discovery and Directory do not bypass Capability Policy.
Fabric enforcement remains mandatory for protected operations.
```

## Phase 6: First CPU Card Integration

Goal:

```text
Connect the W65C816 CPU card only after Supervisor, Fabric, mailbox, Discovery, and Capability paths are validated.
```

Preconditions:

```text
Supervisor validated
Fabric register block validated
Mailbox transport validated
Discovery transport validated
Capability policy validation complete
Fault injection complete
```

Initial CPU-card tests:

```text
Reset hold
Reset release
Clock present
ROM visible locally
SRAM visible locally
Vector fetch
Diagnostic heartbeat
Fabric service request
```

Success criteria:

```text
CPU card interacts with ATARIX through service paths rather than direct global hardware access.
```

## Bench Issues

Recommended issue sequence:

```text
Bench-001 RP2350 Bring-Up
Bench-002 ECP5 Bring-Up
Bench-003 Supervisor-Fabric Mailbox
Bench-004 Discovery Record Transport
Bench-005 Capability Policy Hardware Validation
Bench-006 First CPU Card Integration
```

## Required Instruments

Minimum bench setup:

```text
Bench power supply with current limiting
USB serial access
Logic analyzer
Oscilloscope
Known-good USB cables
Known-good programming cables
ESD-safe work surface
Common ground reference
```

Recommended measurements:

```text
3.3 V rail stability
1.2 V FPGA core rail stability
Clock stability
Reset pulse width
Mailbox signal timing
SPI/UART/parallel timing depending on selected link
Current draw during configuration
Current draw during mailbox stress
```

## Exit Criteria

Hardware Validation Sprint 1 is complete when:

```text
RP2350 boots.
ECP5 configures.
Supervisor can reset Fabric.
Supervisor can read Fabric version.
Supervisor can write and read scratch registers.
Mailbox packet exchange works.
Fault injection is detected safely.
Discovery records can be transported.
Capability decisions can be exercised over the link.
Unauthorized access attempts are denied.
```

## Result

This plan turns the ATARIX architecture into measurable bench work.

The first physical milestone is reliable Supervisor-Fabric communication with security-preserving service abstraction.

<!-- AEMS-MIGRATED-SOURCE: docs/interrupt-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/interrupt-architecture-v1.md -->
### Integrated source: `docs/interrupt-architecture-v1.md`

# ATARIX Interrupt Architecture v1

## Status

Draft interrupt architecture specification.

This document defines interrupt sources, routing, priority, masking, acknowledgement, supervisor interaction, and future multi-CPU delivery for ATARIX.

## Purpose

Interrupts connect the W65C816 CPU, supervisor controller, FPGA fabric, mailbox engine, DMA engine, timers, network services, and future accelerators.

The interrupt architecture must remain simple enough for Rev A bring-up while allowing a fabric-mediated interrupt controller in later revisions.

## Design Goals

1. Keep Rev A interrupt handling simple and observable.
2. Permit Rev C FPGA-mediated interrupt routing.
3. Support interrupt masking and prioritization.
4. Support mailbox and DMA completion events.
5. Prepare for future multi-CPU routing.
6. Preserve a deterministic recovery path.

## W65C816 Interrupt Context

The W65C816 supports:

```text
IRQ
NMI
ABORT
RESET
```

For ATARIX:

- RESET is controlled by the supervisor and board reset logic.
- NMI is reserved for high-priority fault or recovery events.
- IRQ is used for normal maskable device and fabric events.
- ABORT is reserved for future bus fault or protection experiments.

## Interrupt Classes

```text
Class 0 Reset / Boot Control
Class 1 Non-Maskable Fault
Class 2 Fabric Fault
Class 3 Timer
Class 4 Mailbox
Class 5 DMA
Class 6 Network
Class 7 Storage
Class 8 Accelerator
Class 9 External Slot Device
Class 10 Debug / Monitor
```

## Initial Interrupt Sources

```text
0 UART
1 Supervisor
2 Timer
3 Mailbox
4 DMA
5 Network
6 Accelerator
7 External Slot
8 Fabric Fault
9 Debug Request
```

These source numbers extend the initial interrupt list in `docs/register-map-v1.md` and should be treated as the preferred v1 source list.

## Interrupt Controller Register Block

See:

```text
docs/register-map-v1.md
```

Base:

```text
$00D100
```

Minimum registers:

```text
+00 Pending Low
+01 Pending High
+02 Mask Low
+03 Mask High
+04 Priority Select
+05 Acknowledge
+06 Route Target
+07 Status
```

The original four-register draft is considered the minimal compatibility subset.

## Pending Register

Each bit corresponds to an interrupt source.

```text
Bit 0 UART
Bit 1 Supervisor
Bit 2 Timer
Bit 3 Mailbox
Bit 4 DMA
Bit 5 Network
Bit 6 Accelerator
Bit 7 External Slot
Bit 8 Fabric Fault
Bit 9 Debug Request
```

## Mask Register

A set bit masks the corresponding interrupt source.

Masking prevents delivery to CPU IRQ but does not necessarily clear the pending bit.

## Priority Model

Initial priority order:

```text
Highest: NMI / Supervisor Fault
Fabric Fault
DMA Fault
Mailbox Fault
Timer
UART
Network
Storage
Accelerator Completion
External Slot
Lowest: Debug Request
```

The priority model may later become programmable.

## Acknowledge Model

Interrupts should be acknowledged explicitly.

Acknowledge sequence:

1. CPU enters interrupt handler.
2. CPU reads pending register.
3. CPU services highest-priority unmasked source.
4. CPU writes source number or bit mask to acknowledge register.
5. Controller clears pending status if the source is no longer asserted.

Level-triggered sources may reassert until the device condition is cleared.

## IRQ Routing

Rev A:

```text
Device IRQ lines may be directly ORed or simply decoded.
```

Rev C and later:

```text
FPGA fabric routes device events to CPU IRQ.
```

The FPGA interrupt controller should expose pending, mask, priority, and acknowledgement state.

## NMI Use

NMI is reserved for conditions requiring immediate attention.

Candidate NMI sources:

- Supervisor emergency fault.
- Fatal fabric fault.
- Recovery button.
- Watchdog expiration.

NMI must not be used for routine device completion.

## ABORT Use

ABORT is reserved for future protection and bus-error experiments.

Possible future uses:

- Invalid memory window access.
- Capability violation.
- Bus timeout.
- Fabric protection fault.

ABORT should remain disabled or unused until the system can report faults reliably.

## Mailbox Interrupts

Mailbox interrupts indicate:

- Message received.
- Response available.
- Mailbox fault.

Mailbox handlers should read mailbox status before reading payload data.

See:

```text
docs/mailbox-protocol-v1.md
```

## DMA Interrupts

DMA interrupts indicate:

- Transfer complete.
- Transfer fault.
- Capability validation failure.
- Timeout.

See:

```text
docs/dma-engine-v1.md
```

## Timer Interrupts

Timer interrupts support:

- Monitor delays.
- Scheduler ticks.
- Watchdog service.
- Benchmarking.

Timer interrupts should be maskable.

## Supervisor Interrupts

Supervisor-generated interrupts may indicate:

- Reset reason available.
- Fault log update.
- Recovery request.
- Power or thermal warning.
- Configuration event.

Supervisor emergency events may use NMI depending on severity.

## Multi-CPU Routing

Future Rev D systems may route interrupts to specific CPU cards.

Routing targets:

```text
$00 Broadcast
$01 Primary CPU
$02 Secondary CPU
$03 Supervisor
$04 Fabric Service
```

Future routing policy may include:

- Fixed CPU ownership.
- Round-robin delivery.
- Priority CPU delivery.
- Supervisor-directed routing.

## Fault Reporting

Interrupt-related faults should report:

```text
Source
Class
Pending Mask
Delivery Target
Fault Code
Timestamp or Counter
```

## Boot Defaults

On reset:

```text
All maskable interrupts disabled
Pending state cleared if safe
NMI reserved for recovery/fault
DMA interrupts disabled until DMA initialized
Mailbox interrupts disabled until mailbox initialized
Timer interrupts disabled until monitor enables them
```

## Design Rule

Interrupts notify.

They do not transfer ownership, grant authority, or imply trust.

Authority remains controlled by the capability model.

## Open Questions

- Final number of interrupt sources.
- Whether priorities are fixed or programmable.
- Whether interrupt status should be 16-bit or 32-bit.
- Whether ABORT should be used for capability faults.
- Multi-CPU delivery policy.
- Exact relationship between fabric faults and NMI.

<!-- AEMS-MIGRATED-SOURCE: docs/mailbox-protocol-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/mailbox-protocol-v1.md -->
### Integrated source: `docs/mailbox-protocol-v1.md`

# ATARIX Mailbox Protocol v1

Status: Baseline

## Purpose

The ATARIX Mailbox Protocol defines the first transport contract between Supervisor, Fabric, simulator components, and future hardware implementations.

The mailbox is not an authority mechanism. It transports requests and responses. Authority remains with Capability Policy and Fabric enforcement.

## Header Format

Mailbox v1 uses a fixed 16-byte header:

```c
struct atarix_mailbox_header_v1 {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
};
```

Current constants:

```text
Magic:              ATARIX_MAILBOX_MAGIC_V1
Header size:        16 bytes
Maximum payload:    4096 bytes
```

## Message Types

Mailbox v1 defines the following message types:

```text
INVALID
PING
PONG
VERSION_QUERY
VERSION_REPLY
STATUS_QUERY
STATUS_REPLY
READ_REGISTER
WRITE_REGISTER
RESET_REQUEST
FAULT_REPORT
DISCOVERY_RECORD_PUSH
CAPABILITY_EVALUATE
```

Unknown or invalid message types are rejected.

## Validation Order

Receivers validate mailbox traffic before dispatch.

Required validation order:

```text
Header present
Payload present when length is nonzero
Magic valid
Message type known
Length valid
Length matches payload buffer
CRC valid
Sequence accepted
Capability valid if required
Dispatch
```

Invalid traffic must not reach service dispatch.

## CRC Rules

The CRC covers the payload.

A CRC mismatch rejects the message.

CRC failure is observable and must return an explicit failure status.

## Length Rules

The header length must match the actual payload length.

Payload length must not exceed the protocol maximum.

Oversized payloads are rejected.

Malformed lengths are rejected.

## Sequence Rules

Mailbox sequences are tracked per receiver window.

Rules:

```text
First sequence accepted.
Exact duplicate rejected.
Older sequence rejected as out-of-order.
Sequence gaps rejected.
Next expected sequence accepted.
```

Sequence handling exists to detect replay, duplicate delivery, out-of-order delivery, and transport corruption.

## Dispatch Rules

Message dispatch occurs only after validation.

Preferred implementation for larger message sets is table-based dispatch:

```text
handler = dispatch_table[message_type]
```

Dispatch tables must not bypass validation.

## Security Rules

Mailbox routing does not grant authority.

The following are not authority:

```text
Valid mailbox header
Valid CRC
Known message type
Known sequence
Known endpoint
Known service handle
```

Capability Policy grants authority.

Protected operations must validate capabilities before resource access.

## Failure Rules

Failures must be observable.

Mailbox implementations must explicitly report:

```text
NULL input
Invalid header
Invalid length
CRC mismatch
Sequence error
Duplicate packet
Out-of-order packet
Unknown message type
Capability failure
```

Silent failure is not acceptable.

## Simulator Requirements

The simulator must follow the same mailbox rules as hardware.

Simulator implementations must reject malformed mailbox traffic in the same way future hardware-facing implementations are expected to reject it.

## Current Test Coverage

Mailbox v1 is covered by tests for:

```text
CRC validation
General validation
Sequence validation
Duplicate rejection
Out-of-order rejection
Unknown type rejection
Maximum payload handling
Oversized payload rejection
```

## Result

Mailbox Protocol v1 provides the first ATARIX transport contract.

It is intentionally small, deterministic, security-first, and suitable for simulation before hardware exists.

<!-- AEMS-MIGRATED-SOURCE: docs/netboot-ntp-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/netboot-ntp-v1.md -->
### Integrated source: `docs/netboot-ntp-v1.md`

# ATARIX Netboot and NTP Boot Services v1

## Status

Draft boot-services specification.

This document records the decision that ATARIX Rev A should support FPGA-assisted networking services early enough to enable netboot and automatic time synchronization at boot.

## Purpose

Because ATARIX includes an FPGA fabric controller from Rev A, networking should be treated as a core bring-up and recovery feature rather than a late operating-system feature.

Netboot allows firmware and kernel images to be loaded without repeatedly programming ROMs or local storage.

NTP allows the RTC and fault logs to be corrected automatically at boot when a network is available.

## Design Goals

1. Support network-assisted bring-up.
2. Support automatic time setting at boot.
3. Keep recovery possible without a network.
4. Keep the boot path observable through UART diagnostics.
5. Avoid making network success mandatory for local boot.
6. Preserve a safe fallback path using RTC, ROM monitor, and recovery mode.

## Hardware Assumptions

Early networking may be provided by:

```text
W5500 Ethernet controller
FPGA-assisted network interface
Service card network controller
```

The W5500 remains the preferred early candidate because it minimizes TCP/IP stack burden on the W65C816.

## Boot-Time Network Services

Required boot-time services:

```text
Link detection
MAC address readout
DHCP or static IP configuration
NTP time synchronization
TFTP or simple netboot image retrieval
Boot-status reporting over UART
```

Optional later services:

```text
BOOTP
PXE-like boot descriptors
HTTP boot
Remote monitor commands
Remote fault-log retrieval
```

## Boot Order Integration

Network services fit into the boot sequence after minimal hardware initialization and before kernel handoff.

Recommended order:

```text
Power stable
Supervisor start
FPGA fabric ready
CPU reset release
ROM monitor start
Basic diagnostics
Discovery scan
Network service discovery
RTC read
NTP sync attempt
Netboot attempt if configured
Fallback to local boot or monitor
```

See:

```text
docs/boot-sequence-v1.md
```

## Time Initialization Policy

Boot-time time source priority:

```text
1. Valid NTP time
2. Valid RTC time
3. Supervisor retained timestamp
4. Build-time firmware timestamp
5. Unknown time
```

NTP should update the RTC only after a sanity check.

Sanity checks should include:

```text
NTP response valid
Time newer than firmware build time
Time not wildly outside expected range
Network source accepted by configuration
```

## RTC Role

The RTC remains required even with NTP.

RTC responsibilities:

```text
Maintain time without network
Timestamp watchdog resets
Timestamp fault logs
Timestamp recovery-mode entry
Provide boot-time fallback time
```

NTP corrects the RTC when available; it does not replace it.

## Netboot Modes

Candidate netboot modes:

```text
Disabled
Manual Only
Try Netboot Then Local
Require Netboot
Recovery Netboot
```

These may be selected by:

```text
DIP switch
Supervisor configuration
ROM monitor command
Stored configuration EEPROM
```

## Netboot Image Flow

A minimal netboot flow:

```text
1. Initialize network device
2. Acquire IP configuration
3. Locate boot server
4. Request boot image
5. Load image into RAM
6. Verify checksum
7. Prepare handoff structure
8. Jump to loader or kernel
```

## Boot Image Requirements

A boot image should include:

```text
Magic number
Image type
Load address
Entry point
Image length
Checksum or hash
Version string
Optional capability requirements
```

## Failure Policy

Network failure must not brick the system.

If NTP fails:

```text
Use RTC if valid
Mark time as unsynchronized
Continue boot unless policy requires synchronized time
```

If netboot fails:

```text
Fallback to local boot if permitted
Enter monitor if no valid boot source exists
Record failure reason
```

## Security Policy

Rev A may begin with checksum-based validation.

Future revisions should support:

```text
Signed boot images
Trusted boot-server configuration
Capability-restricted boot services
Network-service isolation
```

Netboot must not automatically grant DMA, accelerator, or privileged fabric access.

## Diagnostic Output

ROM monitor boot logs should show:

```text
Network device detected
Link status
MAC address
IP address
NTP result
RTC status
Netboot mode
Boot server
Image name
Image verification result
Fallback reason if any
```

## FPGA Role

The Rev A FPGA fabric controller may assist with:

```text
Network device register windowing
Interrupt routing
DMA gating for network buffers
Boot status LEDs
Timeout enforcement
Fault reporting
```

## Design Rule

Network boot and time synchronization are convenience and reliability features.

They must improve bring-up and maintenance without becoming single points of failure.

## Open Questions

- DHCP versus static IP as the Rev A default.
- TFTP versus simpler custom loader protocol.
- Whether W5500 is on CPU card, service card, or backplane service area.
- Boot image header format.
- Whether signed images are required before Rev B.
- How NTP sanity thresholds are configured.
- Whether RTC is on the supervisor card or fabric-controller/backplane card.

<!-- AEMS-MIGRATED-SOURCE: docs/operation-id-registry-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/operation-id-registry-v1.md -->
### Integrated source: `docs/operation-id-registry-v1.md`

# Operation ID Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Operation ID registry.

Operation IDs identify actions that may be authorized by Capability Records.

Operation IDs are used by:

- Capability Records
- Supervisor policy
- Ring security policy
- Service Directory binding
- Validation tools
- Monitor commands

An Operation ID identifies an action.

An Operation ID does not grant authority.

## Architectural Rules

```text
Identity identifies.
Discovery describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

## Operation ID Ranges

```text
0x00000000              Reserved / invalid
0x00000001-0x00000FFF   Core operations
0x00001000-0x00001FFF   Instrumentation operations
0x00002000-0x00002FFF   CPU operations
0x00003000-0x00003FFF   Network operations
0x00004000-0x00004FFF   Storage operations
0x80000000-0xFFFFFFFF   Experimental / homebrew operations
```

## Core Operations

```text
0x00000001  OP_READ
0x00000002  OP_WRITE
0x00000003  OP_CONTROL
0x00000004  OP_RESET
0x00000005  OP_CAPTURE
0x00000006  OP_STIMULATE
0x00000007  OP_BOOT
0x00000008  OP_RECOVER
0x00000009  OP_PROVISION
0x0000000A  OP_ENUMERATE
0x0000000B  OP_VALIDATE
0x0000000C  OP_MONITOR
0x0000000D  OP_BIND
0x0000000E  OP_DELEGATE
0x0000000F  OP_REVOKE
```

## Instrumentation Operations

```text
0x00001000  OP_TRACE_START
0x00001001  OP_TRACE_STOP
0x00001002  OP_TRACE_EXPORT
0x00001003  OP_TRIGGER_ARM
0x00001004  OP_TRIGGER_FIRE
0x00001005  OP_PATTERN_LOAD
0x00001006  OP_PATTERN_START
0x00001007  OP_PATTERN_STOP
```

## CPU Operations

```text
0x00002000  OP_CPU_READ_HEALTH
0x00002001  OP_CPU_READ_BOOT_STATUS
0x00002002  OP_CPU_HOLD_RESET
0x00002003  OP_CPU_RELEASE_RESET
0x00002004  OP_CPU_CAPTURE_TRACE
0x00002005  OP_CPU_VALIDATE
```

## Security Notes

Operation IDs are interpreted through Capability Records and ring policy.

Examples:

```text
OP_READ
    may be safe for lower rings depending on target

OP_STIMULATE
    generally requires Ring -2 or Ring -1

OP_PROVISION
    generally requires Ring -2

OP_BOOT
    requires explicit boot authority
```

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Operation IDs are allocated by registry update.

## Related Documents

- capability-record-format-v1.md
- ADR-RING-SECURITY-MODEL.md
- service-directory-format-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/resource-type-registry-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/resource-type-registry-v1.md -->
### Integrated source: `docs/resource-type-registry-v1.md`

# Resource Type Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Resource Type ID registry.

Resource Type IDs identify classes of resources exposed through the Enumeration Fabric.

Resource Type IDs are used by:

- Discovery ROM
- Enumeration services
- Capability records
- Directory services
- Validation services
- Instrumentation services

A Resource Type ID identifies a class of resource.

A Resource Type ID does not identify a specific instance.

Specific instances are identified through handles.

## Architectural Rules

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

Resource Type IDs do not grant authority.

Resource Type IDs do not imply trust.

Resource Type IDs do not imply capability.

## Resource ID Ranges

```text
0x0000          Reserved / invalid
0x0001-0x0FFF   Core resources
0x1000-0x1FFF   Instrumentation resources
0x2000-0x2FFF   CPU resources
0x3000-0x3FFF   Network resources
0x4000-0x4FFF   Storage resources
0x5000-0x5FFF   Reserved
0x6000-0x6FFF   Accelerator resources
0x7000-0x7FFF   Reserved
0x8000-0xFFFF   Experimental / homebrew resources
```

## Core Resources

```text
0x0001  RESOURCE_MAILBOX
0x0002  RESOURCE_DMA
0x0003  RESOURCE_TIMER
0x0004  RESOURCE_INTERRUPT_CONTROLLER
0x0005  RESOURCE_GPIO
0x0006  RESOURCE_UART
0x0007  RESOURCE_FABRIC_MEMORY
0x0008  RESOURCE_SHARED_MEMORY
0x0009  RESOURCE_EVENT_QUEUE

0x000A  RESOURCE_ENUMERATION_FABRIC
0x000B  RESOURCE_MANAGEMENT_FABRIC
0x000C  RESOURCE_TRANSPORT_FABRIC

0x0010  RESOURCE_SUPERVISOR
0x0011  RESOURCE_HEALTH_MONITOR
0x0012  RESOURCE_PROVISIONING_ENGINE
0x0013  RESOURCE_VALIDATION_ENGINE
```

## Instrumentation Resources

```text
0x1000  RESOURCE_LOGIC_ANALYZER
0x1001  RESOURCE_OSCILLOSCOPE
0x1002  RESOURCE_DSP
0x1003  RESOURCE_PATTERN_GENERATOR
0x1004  RESOURCE_PROTOCOL_DECODER
0x1005  RESOURCE_COUNTER_BANK
0x1006  RESOURCE_TRIGGER_ROUTER
0x1007  RESOURCE_TRACE_BUFFER
```

## CPU Resources

```text
0x2000  RESOURCE_CPU
0x2001  RESOURCE_CPU_DIAGNOSTICS
0x2002  RESOURCE_CPU_TRACE
0x2003  RESOURCE_CPU_HEALTH
0x2004  RESOURCE_CPU_BOOT_STATUS
```

## Network Resources

```text
0x3000  RESOURCE_NETWORK_INTERFACE
0x3001  RESOURCE_PACKET_ENGINE
0x3002  RESOURCE_NETWORK_BOOT
```

## Storage Resources

```text
0x4000  RESOURCE_BLOCK_STORAGE
0x4001  RESOURCE_FILE_STORAGE
0x4002  RESOURCE_ROM_STORAGE
0x4003  RESOURCE_CONFIGURATION_STORAGE
```

## Accelerator Resources

```text
0x6000  RESOURCE_GPU
0x6001  RESOURCE_AI_ACCELERATOR
0x6002  RESOURCE_VECTOR_ENGINE
0x6003  RESOURCE_FPGA_COMPUTE
0x6004  RESOURCE_CRYPTO_ENGINE
```

## Experimental / Homebrew Resources

```text
0x8000 - 0xFFFF
```

Reserved for:

- prototype cards
- homebrew hardware
- experimental firmware
- development systems

No central registry assignment is required for this range.

Products intended for general release should migrate to assigned Resource Type IDs.

## Ring Security Notes

Resource Type IDs may be associated with default ownership, visibility, and control rings in future security policy documents.

Examples:

```text
RESOURCE_SUPERVISOR
    Owner:   Ring -2
    Visible: Ring -2 / Ring -1
    Control: Ring -2

RESOURCE_PATTERN_GENERATOR
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0 by policy
    Control: Ring -2 / Ring -1 unless delegated
```

These notes are advisory in this registry.

Actual authorization belongs to capability and ring policy.

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Resource Type IDs are allocated by registry update.

## Related Documents

- discovery-rom-format-v1.md
- service-id-registry-v1.md
- ADR-RING-SECURITY-MODEL.md
- capability-record-format-v1.md
- card-health-model-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/ATX-SPEC-015-Reconciliation-Review.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/ATX-SPEC-015-Reconciliation-Review.md -->
### Integrated source: `docs/reviews/ATX-SPEC-015-Reconciliation-Review.md`

# ATX-SPEC-015 Reconciliation Review

## Status

Review complete

## Scope

This review reconciles ATX-SPEC-015 POSIX Compatibility Model against the architecture areas it most directly touches:

```text
ATX-SPEC-005 Mailbox Model
ATX-SPEC-010 Audit Model
ATX-SPEC-011 Error Model
ATX-SPEC-018 Recovery and Reconciliation Model
ATX-SPEC-019 Service Model
ATX-SPEC-021 Memory and Data Movement Model
```

The goal is to verify that POSIX compatibility remains a compatibility projection and does not become native Atarix authority.

## Review Result

```text
Overall Result: ACCEPTED WITH FOLLOW-UP REGISTRIES
```

ATX-SPEC-015 is coherent with the current Atarix architecture baseline.

No blocking conflict was identified.

Several follow-up registries should be created before implementation begins.

## Core Compatibility Rule Confirmed

ATX-SPEC-015 correctly establishes:

```text
POSIX is a compatibility personality.
POSIX is not native authority.
```

This aligns with the architecture-wide principles:

```text
Discovery is not authority.
Lookup is not access.
Observation is not control.
Persistence is not authority.
Recovery is not administrative access.
Compatibility is not authority.
```

## Reconciliation Against ATX-SPEC-005 Mailbox Model

ATX-SPEC-015 now states:

```text
Native IPC is mailbox-based.
Compatibility IPC is a projection.
```

This is consistent with the Atarix mailbox-centered IPC architecture.

POSIX-facing events, streams, queues, and local endpoints are modeled as projections over mailboxes or mailbox-backed service endpoints.

### Result

```text
Aligned
```

### Follow-Up

Define a compatibility IPC mapping table:

```text
Compatibility event -> mailbox event
Compatibility stream -> mailbox-backed stream endpoint
Compatibility queue -> mailbox queue
Compatibility local endpoint -> mailbox-backed local service endpoint
Network descriptor -> Network Service endpoint plus mailbox-backed descriptor
```

## Reconciliation Against ATX-SPEC-010 Audit Model

ATX-SPEC-015 correctly requires audit records to preserve both:

```text
Compatibility view
Native Atarix authority decision
```

The model also requires native error context to survive compatibility projection.

This prevents POSIX-style simplified return codes from erasing security-relevant audit context.

### Result

```text
Aligned
```

### Follow-Up

Add POSIX compatibility entries to the audit event registry:

```text
POSIX_ENV_CREATED
POSIX_PROFILE_SELECTED
POSIX_PATH_LOOKUP
POSIX_DESCRIPTOR_OPENED
POSIX_DESCRIPTOR_DUPLICATED
POSIX_DESCRIPTOR_INHERITED
POSIX_PROGRAM_LOADED
POSIX_PROCESS_CREATED
POSIX_IPC_EVENT_SENT
POSIX_NETWORK_ENDPOINT_CREATED
POSIX_ERROR_MAPPED
POSIX_RECOVERY_STARTED
POSIX_RECOVERY_RECONCILED
POSIX_ENV_QUARANTINED
```

## Reconciliation Against ATX-SPEC-011 Error Model

ATX-SPEC-015 correctly treats compatibility errors as projections of native Atarix errors.

The native error remains authoritative.

The specification explicitly states that unknown native errors must not be silently converted to success.

### Result

```text
Aligned
```

### Follow-Up

Create a POSIX compatibility error mapping table.

The table should preserve:

```text
Native error code
Compatibility error code
Object or service identity
Operation
Policy result
Lifecycle state
Audit event identity
```

## Reconciliation Against ATX-SPEC-018 Recovery and Reconciliation Model

ATX-SPEC-015 correctly states:

```text
Compatibility recovery restores execution.
Compatibility recovery does not restore authority.
```

Recovery validates descriptors, paths, processes, network endpoints, audit continuity, and quarantine outcomes against native Atarix state.

This is aligned with ATX-SPEC-018's recovery principle that recovery restores trust by proving consistency, not by assuming it.

### Result

```text
Aligned
```

### Follow-Up

Add POSIX recovery tests to the recovery test plan:

```text
Descriptor reconciliation fails closed
Path reconciliation does not grant access
Process reconciliation requires native lifecycle validity
Network reconciliation detects broken endpoints
Audit discontinuity becomes explicit
Quarantine blocks normal execution
Unknown recovery outcome fails closed
```

## Reconciliation Against ATX-SPEC-019 Service Model

ATX-SPEC-015 models device-like endpoints, network endpoints, storage-like endpoints, terminal-like endpoints, and entropy-like endpoints as service-bound compatibility endpoints rather than native device authority.

This is aligned with the Service Model principle that service location is not service identity.

Compatibility endpoint names are bindings to native services.

### Result

```text
Aligned
```

### Follow-Up

Define the initial service binding map:

```text
Null-like endpoint -> Null Service
Entropy-like endpoint -> Entropy Service
Terminal-like endpoint -> Terminal Service
Storage-like endpoint -> Storage Service
Network-like endpoint -> Network Service
```

## Reconciliation Against ATX-SPEC-021 Memory and Data Movement Model

ATX-SPEC-015 defers shared memory and memory mapping semantics to ATX-SPEC-021.

This is the correct architectural placement.

Shared memory must not become a path around capability checks, policy checks, audit, or recovery boundaries.

### Result

```text
Aligned, pending ATX-SPEC-021
```

### Follow-Up

When ATX-SPEC-021 is drafted, include a section for POSIX shared memory projection.

## Findings

### Finding 001: POSIX Compatibility Is Properly Contained

ATX-SPEC-015 consistently treats POSIX as a projection over native Atarix semantics.

This prevents POSIX identity, paths, descriptors, and process identifiers from becoming native authority.

### Finding 002: Mailbox-Centered IPC Is The Correct Native Mapping

ATX-SPEC-015 avoids creating separate native subsystems for POSIX events, streams, queues, local endpoints, and network descriptors.

This keeps authority, audit, recovery, and resource accounting unified.

### Finding 003: Error And Audit Registries Are Now Required

The POSIX model is coherent, but implementation should not begin until compatibility audit events and error mappings are registered.

### Finding 004: Profiles Need Object Format

The profile model is sound, but the profile registry needs a concrete object format before implementation.

### Finding 005: Shared Memory Must Wait For ATX-SPEC-021

The deferment of shared memory semantics is correct.

Implementing compatibility shared memory before ATX-SPEC-021 would risk creating unreviewed authority and data movement behavior.

## Required Follow-Up Work

```text
1. Create POSIX compatibility profile registry object format.
2. Create POSIX compatibility error mapping table.
3. Add POSIX compatibility audit event identifiers.
4. Define compatibility IPC mapping table against ATX-SPEC-005.
5. Define initial service binding map against ATX-SPEC-019.
6. Add POSIX recovery tests against ATX-SPEC-018.
7. Defer shared memory semantics to ATX-SPEC-021.
```

## Review Decision

ATX-SPEC-015 is accepted as coherent with the current architecture baseline.

It should remain marked as draft until the follow-up registries and mapping tables are created.

## Summary

ATX-SPEC-015 successfully preserves the central Atarix architecture rule:

```text
Compatibility is not authority.
```

The specification is ready to inform implementation planning after registries are created for profiles, errors, audit events, IPC mappings, and service bindings.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/capability-sprint-1-review.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/capability-sprint-1-review.md -->
### Integrated source: `docs/reviews/capability-sprint-1-review.md`

# Capability Sprint 1 Review

Status: Complete

## Objective

Capability Sprint 1 established the first tested capability-policy foundation for ATARIX.

The sprint translated the accepted capability evaluation model into public headers, implementation code, and automated regression tests.

## Implemented Components

Public API:

```text
include/atarix/capability.h
include/atarix/capability_engine.h
include/atarix/capability_policy.h
```

Implementation:

```text
src/capability/capability_evaluator.c
src/capability/capability_policy.c
```

Tests:

```text
tests/capability/test_capability_validation.c
tests/capability/test_capability_ownership.c
tests/capability/test_capability_ring_boundary.c
tests/capability/test_capability_delegation.c
tests/capability/test_capability_revocation.c
```

## Security Invariants Verified

Capability Sprint 1 verifies the following security invariants:

```text
Validation
Ownership
Ring boundaries
Delegation
Revocation
Expiration
Deny-by-default behavior
```

## Validation

Capability records are rejected when structurally invalid.

The validation tests cover:

```text
NULL capability records
Invalid capability identifiers
Invalid operations
Invalid ring ranges
Missing target handles
Missing issuer handles
```

## Ownership

Capabilities are principal-bound.

The ownership tests cover:

```text
Caller-owner match allowed
Caller-owner mismatch denied
Revoked owner trust denied
Untrusted issuer denied
```

## Ring Boundaries

Ring boundaries are enforced as absolute authorization boundaries.

The ring tests cover:

```text
Application self-access
Service self-access
Driver self-access
Kernel self-access
Fabric self-access
Supervisor self-access
Application denied Kernel authority
Service denied Driver authority
Driver denied Fabric authority
Kernel denied Supervisor authority
```

## Delegation

Delegation may reduce authority only.

The delegation tests cover:

```text
Kernel to Driver allowed
Driver to Service allowed
Service to Application allowed
Application to Service denied
Service to Driver denied
Driver to Kernel denied
Delegation without delegation flag denied
Supervisor authority delegation denied
Delegation that adds flags denied
```

## Revocation and Expiration

Revocation and expiration are enforced before ordinary trust, ownership, ring, resource, and operation checks.

The revocation tests cover:

```text
Valid capability allowed
Revoked capability denied
Expired capability denied
Expiration boundary allowed
Zero lifetime denied
Revocation overrides ownership
Revocation overrides trust
```

## CI Result

Capability Sprint 1 completed with the following automated test result:

```text
TOTAL: 14
PASS:  14
SKIP:  0
XFAIL: 0
FAIL:  0
XPASS: 0
ERROR: 0
```

Test inventory:

```text
Discovery tests:   9
Capability tests:  5
Total tests:      14
```

## Architectural Decisions Implemented

The sprint implements the capability evaluation model established by:

```text
docs/adr/ADR-00X-capability-evaluation-model.md
```

Core decisions implemented:

```text
Supervisor is the root of trust.
Fabric is the enforcement authority.
Capabilities are principal-bound.
Capabilities are non-transferable.
Delegation may only reduce authority.
Revocation is authoritative.
Expiration is enforced.
Default decision is DENY.
```

## Deferred Work

The following work is intentionally deferred to later capability or directory sprints:

```text
Global atarix_status_t
Capability issuance workflow
Capability chain storage
Capability revocation registry
Supervisor-to-Fabric revocation propagation
Cryptographic capability integrity
Audit log integration
Capability serialization hardening
Cross-subsystem Directory integration
```

## Sprint 2 Recommendations

Recommended next capability work:

```text
Define capability issuance semantics.
Add capability attenuation helpers.
Define revocation registry representation.
Integrate capability policy with Directory Sprint 1.
Introduce global status handling when statuses cross subsystem boundaries.
Add audit-friendly reason codes.
```

## Result

Capability Sprint 1 is complete.

The ATARIX repository now has a tested capability-policy baseline with validation, ownership, ring, delegation, revocation, and expiration coverage.

<!-- AEMS-MIGRATED-SOURCE: docs/ring-security-test-plan.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ring-security-test-plan.md -->
### Integrated source: `docs/ring-security-test-plan.md`

# ATARIX Ring Security Test Plan

Status: Planned

## Purpose

This document records the test additions required to make ATARIX ring security executable rather than merely documented.

Existing tests should not be rewritten. The ring-security work adds new tests that prove ring policy is enforced before capability authority, mailbox delivery, fabric routing, and Supervisor service access.

## Security Model Under Test

ATARIX uses two related but separate protection mechanisms:

```text
Ring security       = coarse trust boundary
Capability security = fine-grained authority boundary
```

The ring boundary is the large-gauge net.

Capabilities are the fine mesh.

A capability may reduce, delegate, expire, or revoke authority, but it may not grant authority that violates ring policy.

Required decision chain:

```text
Deny by default
    -> Ring boundary check
    -> Capability validation
    -> Revocation check
    -> Mailbox validation
    -> Service policy
    -> Resource policy
    -> Authorized operation
```

Failure at any step denies the operation.

## Baseline Ring Model

Initial rings:

```text
Ring 0: Supervisor / Root Authority
Ring 1: Fabric Management / Trusted System Services
Ring 2: Device Services / Card Firmware
Ring 3: Applications / User Workloads
Ring 4: Untrusted or Quarantined Nodes
```

The exact ring set may evolve by ADR. The existence of ring boundaries is mandatory.

## Current Tests That Remain Valid

The following existing test families remain valid and should not be rewritten simply because ring policy is being added:

```text
discovery/*
capability/*
mailbox/*
sim/*
```

Ring security adds new test coverage. It does not invalidate the current 29-test baseline.

## New Ring Manager Tests

Planned test files:

```text
tests/ring/test_ring_self_access.c
tests/ring/test_ring_upward_denied.c
tests/ring/test_ring_downward_allowed.c
tests/ring/test_ring_quarantine_denied.c
tests/ring/test_ring_supervisor_override.c
```

### test_ring_self_access

Proves that an actor may access resources within its own ring when operation policy allows it.

Expected behavior:

```text
Ring N -> Ring N: allowed for permitted operation classes
```

### test_ring_upward_denied

Proves lower-trust rings cannot directly command, inspect, mutate, bypass, or impersonate higher-trust rings.

Expected behavior:

```text
Ring 3 -> Ring 0: denied
Ring 2 -> Ring 0: denied unless explicitly modeled as a supervised request path
Ring 4 -> any trusted ring: denied
```

### test_ring_downward_allowed

Proves higher-trust rings may supervise, constrain, or revoke authority from lower-trust rings.

Expected behavior:

```text
Ring 0 -> Ring 1: allowed for Supervisor operations
Ring 0 -> Ring 2: allowed for Supervisor operations
Ring 1 -> Ring 2: allowed for Fabric-management operations
```

### test_ring_quarantine_denied

Proves quarantined nodes cannot reach trusted services or other normal nodes.

Expected behavior:

```text
Ring 4 -> Ring 0: denied
Ring 4 -> Ring 1: denied
Ring 4 -> Ring 2: denied
Ring 4 -> Ring 3: denied unless explicit quarantine-service policy allows it
```

### test_ring_supervisor_override

Proves Ring 0 can perform recovery, revocation, and quarantine actions that lower rings cannot perform.

Expected behavior:

```text
Ring 0 may quarantine lower rings.
Ring 0 may revoke lower-ring authority.
Ring 0 may inspect Supervisor-owned tables.
Lower rings may not override Ring 0 policy.
```

## Capability/Ring Interaction Tests

Planned test files:

```text
tests/capability/test_capability_respects_ring_boundary.c
tests/capability/test_delegation_cannot_cross_ring_boundary.c
tests/capability/test_revoked_capability_still_denied_after_ring_check.c
```

### test_capability_respects_ring_boundary

Proves a syntactically valid capability is still denied if ring policy forbids the request.

Example:

```text
Ring 3 application holds a valid-looking capability for a Ring 0 endpoint.
Ring policy denies before resource access.
```

### test_delegation_cannot_cross_ring_boundary

Proves delegation cannot increase authority across a prohibited ring boundary.

Example:

```text
Ring 2 device service delegates to Ring 3 application.
Delegated capability may only reduce authority.
Delegation cannot grant Ring 3 direct access to Ring 1 or Ring 0 internals.
```

### test_revoked_capability_still_denied_after_ring_check

Proves revocation remains effective even when ring policy would otherwise allow the request.

Example:

```text
Ring 3 -> Ring 2 permitted by ring policy.
Capability exists but is revoked.
Operation is denied.
```

## Mailbox/Ring Interaction Tests

Planned test files:

```text
tests/mailbox/test_mailbox_ring_restricted.c
tests/mailbox/test_mailbox_supervisor_endpoint_denied.c
tests/mailbox/test_mailbox_quarantine_isolation.c
```

### test_mailbox_ring_restricted

Proves mailbox delivery cannot bypass ring policy.

Valid mailbox structure is not enough.

### test_mailbox_supervisor_endpoint_denied

Proves lower rings cannot send arbitrary commands to Supervisor-owned mailbox endpoints.

Supervisor requests must go through explicit service policy.

### test_mailbox_quarantine_isolation

Proves quarantined nodes cannot use otherwise valid mailbox traffic to reach trusted nodes.

## Fabric/Ring Interaction Tests

Planned test files:

```text
tests/sim/test_fabric_ring_routing.c
tests/sim/test_fabric_quarantine_node.c
tests/sim/test_fabric_supervisor_visibility.c
```

### test_fabric_ring_routing

Proves fabric routing consults ring policy before delivery.

A valid route is not sufficient if the ring boundary denies communication.

### test_fabric_quarantine_node

Proves the Supervisor can quarantine a node and the fabric prevents normal traffic to or from that node.

### test_fabric_supervisor_visibility

Proves Ring 0 can observe fabric topology while lower rings see only what service policy exposes.

## Suggested Test Count Milestones

Current baseline:

```text
29 tests passing
```

Ring Manager milestone:

```text
+5 ring tests
34 tests expected
```

Capability/ring interaction milestone:

```text
+3 capability/ring tests
37 tests expected
```

Mailbox/ring interaction milestone:

```text
+3 mailbox/ring tests
40 tests expected
```

Fabric/ring interaction milestone:

```text
+3 fabric/ring tests
43 tests expected
```

## Implementation Ordering

Recommended order:

```text
1. Ring Manager public interface
2. Ring Manager core tests
3. Capability/ring integration
4. Mailbox/ring integration
5. Fabric/ring integration
6. Supervisor service registry
```

Do not build Supervisor service registration before ring policy has executable tests.

## Result

This test plan makes the ring-security doctrine actionable.

Ring security becomes a required, testable boundary rather than an implied design intention.

<!-- AEMS-MIGRATED-SOURCE: docs/routing-design-notes.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/routing-design-notes.md -->
### Integrated source: `docs/routing-design-notes.md`

# ATARIX Routing and Message Transport Design Notes

Status: Design Notes

## Purpose

This document records design considerations for message routing, dispatch, and transport optimization within ATARIX.

The primary objective is not maximum throughput. The primary objectives are:

```text
Predictable behavior
Bounded latency
Security
Capability enforcement
Observable failures
Hardware independence
```

Optimization must never compromise architectural guarantees.

## Architectural Principle

Message routing exists to deliver requests to services.

Routing does not grant authority.

Discovery does not grant authority.

Directory does not grant authority.

Visibility does not grant authority.

Capabilities grant authority.

## Mailbox Routing Goals

Mailbox routing should provide:

```text
O(1) dispatch where practical
Deterministic behavior
Bounded memory consumption
Bounded execution time
Security-first operation
Observable failure reporting
```

The mailbox layer rejects invalid traffic before service dispatch.

Recommended validation path:

```text
Header validation
Length validation
CRC validation
Sequence validation
Message type validation
Capability validation when required
Service dispatch
```

## Dispatch Tables

Preferred future implementation:

```c
handler = dispatch_table[message_type];
```

Advantages:

```text
Constant-time lookup
Simple implementation
Easy auditing
Easy testing
Suitable for firmware and FPGA translation
```

Large switch statements are acceptable early but should be reconsidered once message counts grow.

## Bitset-Based Authorization

Capabilities, permissions, allowed message classes, and fault states may be represented as bitsets where practical.

Example:

```c
if ((allowed_mask & (1u << message_type)) == 0u) {
    deny();
}
```

Advantages:

```text
Constant-time checks
Compact representation
FPGA-friendly implementation
Simple testing
```

## Sequence Windows

Mailbox sequence tracking should use bounded, constant-time validation.

Goals:

```text
Replay detection
Duplicate detection
Out-of-order detection
Gap detection
Future wraparound safety
```

Current Mailbox v1 tests cover duplicate, old, and gap rejection.

Future work should define precise wraparound semantics for 32-bit sequence numbers.

## Priority Classes

Future mailbox implementations may support priority classes.

Suggested classes:

```text
P0 Faults, watchdogs, emergency reset
P1 Security and capability operations
P2 Discovery and topology management
P3 Normal service requests
P4 Diagnostics and bulk transfers
```

Priority scheduling must not bypass validation or capability enforcement.

## Service Lookup

When service counts become large, service lookup should move from linear search to bounded lookup.

Candidates:

```text
Hash table
Perfect hash
Sorted table with binary search
FPGA lookup table
Bitset-indexed dispatch
```

Selection criteria:

```text
Deterministic timing
Predictable memory use
Auditability
Ease of simulation
Ease of hardware implementation
```

## Resource Dependency Graphs

Future ATARIX systems may model services and resources as directed graphs.

Example:

```text
Application
    -> Filesystem Service
    -> Storage Service
    -> Block Device Resource
    -> Driver Resource
    -> Hardware Implementation
```

Potential uses:

```text
Startup ordering
Shutdown ordering
Fault propagation analysis
Recovery planning
Capability reachability analysis
Resource locality optimization
Service dependency validation
```

Status: Future research.

## Multi-Node Routing

Future ATARIX deployments may include multiple Fabric nodes, CPU nodes, service cards, or accelerator cards.

Potential future techniques:

```text
Shortest-path routing
Fault-aware routing
Load-aware routing
Capability-constrained routing
Redundant-path routing
Service-locality routing
```

These techniques are not required for Mailbox Protocol v1.

## Mathematical Techniques Under Consideration

The following topics may provide future value:

```text
Graph theory
Queueing theory
Hashing theory
Bitset algebra
Finite-state machine analysis
Formal verification
Reliability modeling
Capability propagation analysis
Resource dependency graphs
Fault propagation models
```

The following are currently premature:

```text
Neural routing
Adaptive learning dispatch
Dynamic route prediction
Heuristic optimization without measurable need
```

## Optimization Rule

Optimization must never violate the required protected-access path:

```text
Application
    -> Service
    -> Capability Policy
    -> Resource
    -> Implementation
```

No routing optimization may create direct hardware access.

No routing optimization may treat Discovery or Directory visibility as authority.

All routing remains subordinate to capability validation and security policy.

## Current Recommendation

For the current virtual-first ATARIX phase, use the simplest deterministic routing model:

```text
Validate header
Validate length
Validate CRC
Validate sequence
Validate message type
Check capability if required
Dispatch by message type
Return explicit status
```

Do not introduce complex routing until a real scaling pressure exists.

## Result

These notes preserve future routing ideas without burdening Mailbox v1 with premature complexity.

<!-- AEMS-MIGRATED-SOURCE: docs/security.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/security.md -->
### Integrated source: `docs/security.md`

# ATARIX Security Architecture

## Status

Draft security architecture specification.

This document defines the ATARIX security model, including privilege rings, capability-mediated access, DMA controls, device trust boundaries, accelerator isolation, supervisor recovery, and future multiprocessor considerations.

## Principle

Devices are not trusted by default.

Security is designed into the architecture rather than added later.

ATARIX uses two complementary concepts:

```text
Privilege Rings
    define execution authority.

Capabilities
    define resource authority.
```

A privilege ring answers:

```text
What kind of code is this?
```

A capability answers:

```text
What specific resource may this code or device access?
```

Both are required for a complete security model.

## Privilege Ring Model

ATARIX should define four logical privilege rings.

These rings are inspired by conventional Ring 0 through Ring 3 operating-system models, but they must be adapted to the W65C816 and FPGA-fabric environment.

The W65C816 does not provide native x86-style hardware rings. Therefore, rings are initially a software and fabric-enforced design discipline, with increasing hardware assistance added over time.

## Ring 0 - Kernel and Core Authority

Ring 0 is the most privileged normal execution ring.

Responsibilities:

- Kernel execution.
- Memory-management policy.
- Capability broker.
- Device-driver authority assignment.
- Interrupt-core management.
- DMA grant creation.
- Fabric configuration policy.

Ring 0 may request privileged fabric operations, but it should still use explicit capability records rather than relying on implicit trust.

Ring 0 does not replace the supervisor management plane.

## Ring 1 - System Services and Trusted Drivers

Ring 1 contains trusted but separable system services.

Examples:

- Filesystem service.
- Network service.
- Storage service.
- Console service.
- Trusted block-device drivers.
- Memory-service manager.

Ring 1 code may receive broad capabilities, but it should not receive unrestricted access by default.

Ring 1 services should be restartable where practical.

## Ring 2 - Device Services and Semi-Trusted Runtime Components

Ring 2 contains semi-trusted services and device-facing components.

Examples:

- Specific hardware drivers.
- Accelerator runtime services.
- Protocol workers.
- Debug agents.
- User-session support services.

Ring 2 components should receive narrow capabilities tied to specific devices, buffers, mailboxes, and interrupts.

A Ring 2 failure should not normally crash the whole machine.

## Ring 3 - User Programs and Untrusted Code

Ring 3 contains ordinary user programs and experimental code.

Ring 3 code should not directly access:

- Raw device registers.
- DMA engines.
- Fabric configuration registers.
- Supervisor controls.
- Privileged interrupt routing.
- Unmediated memory windows.

Ring 3 code must request services through system calls, mailboxes, or mediated service APIs.

## Supervisor Plane - Outside the Ring Model

The supervisor controller is outside the normal Ring 0 through Ring 3 execution model.

The supervisor is a management and recovery authority, not an application execution ring.

Responsibilities:

- RTC.
- Watchdogs.
- Reset control.
- Fault logging.
- Power-good monitoring.
- Slot quarantine.
- Recovery-mode entry.
- Golden ROM or golden bitstream selection.

The supervisor should not silently perform normal data-plane work.

The supervisor may reset or quarantine Ring 0 failures, fabric failures, CPU-card failures, or device failures.

See:

```text
docs/supervisor-card-v1.md
docs/management-anomaly-detection-v1.md
```

## Capability Model

Capabilities define specific resource authority.

Capabilities describe:

- Resource ownership.
- Allowed operations.
- Address windows.
- DMA permissions.
- Mailbox permissions.
- Device permissions.
- Revocation policy.

Capabilities prevent the ring model from becoming too coarse.

For example, two Ring 1 services may both be trusted services, but only the storage service should receive access to storage-controller DMA buffers.

See:

```text
docs/capability-model.md
```

## Ring and Capability Interaction

Access decisions should consider both ring and capability.

Example policy:

```text
Ring 0 + valid fabric capability
    -> may configure fabric service

Ring 1 + valid device capability
    -> may operate assigned device

Ring 2 + valid mailbox capability
    -> may submit commands to assigned service

Ring 3 + no capability
    -> denied
```

Being in a privileged ring is not enough to access every resource.

Having a capability is not enough if the caller is executing in a ring that is not allowed to exercise that kind of capability.

## DMA Security

DMA access must be mediated.

DMA requests require:

1. Valid DMA capability.
2. Valid memory capability.
3. Valid ownership chain.
4. Ring policy allowing the request.

Future FPGA fabric components may participate in:

- DMA authorization.
- DMA isolation.
- DMA revocation.
- Fault reporting.
- Timeout enforcement.

No device should receive unrestricted DMA access to system memory.

See:

```text
docs/dma-engine-v1.md
```

## Device Trust Model

Devices receive explicit access grants.

No device should automatically gain unrestricted memory visibility or register access simply because it is physically installed.

Device discovery should describe:

- Device identity.
- Required capabilities.
- DMA needs.
- Mailbox endpoints.
- Interrupt sources.
- Driver binding requirements.

See:

```text
docs/discovery-rom-format.md
```

## Fabric Security

The FPGA fabric is an enforcement assistant.

The fabric may implement:

- Register access filtering.
- DMA bounds checking.
- Interrupt routing control.
- Mailbox endpoint control.
- Discovery visibility.
- Fault counters.
- Capability lookup acceleration.

The fabric should not become an undocumented black box.

Security-relevant fabric behavior must be described by stable register, mailbox, and discovery specifications.

## Multiprocessor Security

Future CPU modules should communicate through mediated interfaces.

Examples:

- Mailboxes.
- Shared queues.
- Controlled memory windows.
- Capability-backed DMA buffers.

Each CPU card should retain local bank-zero isolation.

A future secondary CPU should not automatically obtain Ring 0 authority merely because it is physically installed.

CPU-card identity, boot state, and assigned authority should be explicit.

## Accelerator Security

Accelerators are treated as powerful but untrusted participants.

They receive explicit capabilities and controlled access to resources.

Accelerator jobs should follow this pattern:

```text
1. Discover accelerator.
2. Allocate buffers.
3. Grant narrow memory capability.
4. Grant accelerator execution capability.
5. Submit job through mailbox or command queue.
6. Validate completion.
7. Revoke temporary capabilities.
```

Accelerators should never receive unrestricted access to CPU-local memory or kernel memory.

## Recovery and Fault Isolation

Security failures should degrade safely.

Possible recovery actions:

- Revoke capability.
- Disable DMA channel.
- Reset service.
- Reset slot.
- Quarantine card.
- Enter recovery mode.
- Preserve fault log for diagnosis.

The supervisor controller should assist recovery and fault isolation.

## Rev A Enforcement Strategy

Rev A does not need to implement full hardware isolation.

Rev A should establish the policy structure and make violations visible.

Recommended Rev A behavior:

- Define Rings 0-3 in software architecture.
- Keep supervisor outside the ring model.
- Require capabilities in firmware interfaces where practical.
- Prevent unrestricted DMA.
- Log denied or invalid accesses.
- Keep dangerous features disabled by default.
- Provide recovery mode.

## Future Enforcement Strategy

Later revisions may add:

- FPGA access-control tables.
- Hardware-assisted DMA bounds checking.
- MMU or window-mapping experiments.
- Per-ring memory windows.
- Capability-tagged mailbox endpoints.
- ABORT-based protection faults.
- Slot quarantine.

## Design Principle

ATARIX should not rely on a single security mechanism.

Use rings for broad execution privilege.

Use capabilities for precise resource authority.

Use the supervisor for recovery and accountability.

Use the FPGA fabric for mediation and enforcement assistance.

## Open Questions

- How Ring 0 through Ring 3 transitions are represented in the OS.
- Whether Ring 3 is enforced by software convention, memory windows, or future MMU logic.
- Whether ABORT should signal protection violations.
- How capabilities are stored and revoked.
- Whether capability tables live in system RAM, supervisor memory, or fabric memory.
- How secondary CPU cards acquire initial authority.
- Whether signed boot images are required before multi-user operation.

<!-- AEMS-MIGRATED-SOURCE: docs/service-directory-format-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/service-directory-format-v1.md -->
### Integrated source: `docs/service-directory-format-v1.md`

# Service Directory Format v1

Status: Draft v1

## Purpose

The Service Directory maps discovered and authorized services to bindable runtime entries.

The Service Directory answers:

```text
Where is it?
Who provides it?
Which handle implements it?
How do I bind to it?
```

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Non-Goals

The Service Directory does not:

- identify hardware
- establish trust
- grant capabilities
- override ring policy
- replace supervisor safety authority

## Directory Header

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Magic
0x04    2     Version Major
0x06    2     Version Minor
0x08    4     Total Length
0x0C    4     Entry Count
0x10    4     Flags
0x14    4     Header CRC32
0x18    4     Directory CRC32
0x1C    4     Reserved
```

Magic:

```text
ATDR
```

Meaning:

```text
ATARIX Directory
```

Header size:

```text
32 bytes
```

## Directory Entry

All entries are 8-byte aligned.

```text
Offset  Size  Field
------  ----  ----------------------
0x00    8     Service Handle
0x08    8     Provider Handle
0x10    4     Service ID
0x14    4     Provider ID
0x18    4     Flags
0x1C    4     Binding Type
0x20    8     Binding Handle
0x28    8     Required Capability Handle
0x30    4     Minimum Ring
0x34    4     Reserved
```

Entry size:

```text
56 bytes
```

## Binding Type

Initial binding types:

```text
0x00000000  Invalid
0x00000001  Mailbox
0x00000002  Transport handle
0x00000003  Shared memory
0x00000004  Supervisor call
0x00000005  Fabric service
0x00000006  Network endpoint
0x00000007  Storage endpoint
```

## Flags

Initial flags:

```text
Bit 0   Available
Bit 1   Authorized
Bit 2   Restricted
Bit 3   Recovery only
Bit 4   Development only
Bit 5   Hotplug capable
Bit 6   Instrumented
Bits 7-31 reserved
```

## Directory Construction

Recommended construction sequence:

```text
Identity validation
    ↓
Discovery enumeration
    ↓
Trust evaluation
    ↓
Capability evaluation
    ↓
Ring policy filtering
    ↓
Directory construction
```

The Directory should contain the authorized platform view appropriate to the requesting ring.

## Ring Filtering

Different rings may receive different directory views.

Example:

```text
Ring -2
    Full supervisor view

Ring -1
    Platform firmware view

Ring 0
    Kernel-authorized services

Ring 3
    Application-safe services only
```

## Recovery Rule

Recovery must not depend on Directory Services.

The supervisor must retain direct diagnostic access when the directory is unavailable, corrupted, or incomplete.

## Security Rules

Directory entries do not grant authority.

Directory entries describe authorized binding results after capability and ring policy have already been evaluated.

## Related Documents

- discovery-rom-format-v1.md
- service-id-registry-v1.md
- capability-record-format-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md

<!-- AEMS-MIGRATED-SOURCE: docs/service-id-registry-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/service-id-registry-v1.md -->
### Integrated source: `docs/service-id-registry-v1.md`

# Service ID Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Service ID registry.

Service IDs identify software-visible services exposed through the Enumeration Fabric and later bound through Directory Services and Transport mechanisms.

Service IDs are used by:

- Discovery ROM
- Service Directory
- Capability Records
- Backplane BIOS
- Monitor
- Operating system services
- Supervisor and instrumentation tooling

A Service ID identifies a class of service.

A Service ID does not identify a specific instance.

Specific instances are identified through handles and directory records.

## Architectural Rules

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

Service IDs do not grant authority.

Service IDs do not imply trust.

Service IDs do not imply capability.

## Service ID Ranges

```text
0x0000          Reserved / invalid
0x0001-0x0FFF   Core platform services
0x1000-0x1FFF   Instrumentation services
0x2000-0x2FFF   CPU services
0x3000-0x3FFF   Network services
0x4000-0x4FFF   Storage services
0x5000-0x5FFF   Reserved
0x6000-0x6FFF   Accelerator services
0x7000-0x7FFF   Reserved
0x8000-0xFFFF   Experimental / homebrew services
```

## Core Platform Services

```text
0x0001  SERVICE_DISCOVERY
0x0002  SERVICE_DIRECTORY
0x0003  SERVICE_PROVISIONING
0x0004  SERVICE_VALIDATION
0x0005  SERVICE_MONITOR
0x0006  SERVICE_HEALTH
0x0007  SERVICE_FAULT_LOG
0x0008  SERVICE_EVENT_LOG
0x0009  SERVICE_TIMEBASE
0x000A  SERVICE_MAILBOX
0x000B  SERVICE_DMA
0x000C  SERVICE_INTERRUPT_ROUTING
0x000D  SERVICE_RECOVERY
```

## Instrumentation Services

```text
0x1000  SERVICE_INSTRUMENTATION
0x1001  SERVICE_CAPTURE
0x1002  SERVICE_ANALYSIS
0x1003  SERVICE_TRACE
0x1004  SERVICE_LOGIC_ANALYZER
0x1005  SERVICE_OSCILLOSCOPE
0x1006  SERVICE_PATTERN_GENERATOR
0x1007  SERVICE_PROTOCOL_DECODER
0x1008  SERVICE_COUNTERS
0x1009  SERVICE_TRIGGER_ROUTER
```

## CPU Services

```text
0x2000  SERVICE_CPU_DIAGNOSTICS
0x2001  SERVICE_CPU_HEALTH
0x2002  SERVICE_CPU_TRACE
0x2003  SERVICE_CPU_BOOT_STATUS
0x2004  SERVICE_CPU_VALIDATION
```

## Network Services

```text
0x3000  SERVICE_NETWORK_INTERFACE
0x3001  SERVICE_NETWORK_BOOT
0x3002  SERVICE_PACKET_TRANSPORT
0x3003  SERVICE_TIME_SYNC
```

## Storage Services

```text
0x4000  SERVICE_BLOCK_STORAGE
0x4001  SERVICE_FILE_STORAGE
0x4002  SERVICE_ROM_STORAGE
0x4003  SERVICE_CONFIGURATION_STORAGE
```

## Accelerator Services

```text
0x6000  SERVICE_GPU_COMPUTE
0x6001  SERVICE_AI_ACCELERATION
0x6002  SERVICE_VECTOR_COMPUTE
0x6003  SERVICE_FPGA_COMPUTE
0x6004  SERVICE_CRYPTO_ACCELERATION
```

## Experimental / Homebrew Services

```text
0x8000 - 0xFFFF
```

Reserved for:

- prototype services
- homebrew services
- experimental firmware
- development systems

No central registry assignment is required for this range.

Products intended for general release should migrate to assigned Service IDs.

## Ring Security Notes

Service IDs may be associated with default ownership, visibility, and control rings in future security policy documents.

Examples:

```text
SERVICE_RECOVERY
    Owner:   Ring -2
    Visible: Ring -2 / Ring -1
    Control: Ring -2

SERVICE_PATTERN_GENERATOR
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0 by policy
    Control: Ring -2 / Ring -1 unless delegated

SERVICE_CPU_HEALTH
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0
    Control: Ring -2 / Ring -1
```

These notes are advisory in this registry.

Actual authorization belongs to capability and ring policy.

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Service IDs are allocated by registry update.

## Related Documents

- discovery-rom-format-v1.md
- resource-type-registry-v1.md
- ADR-RING-SECURITY-MODEL.md
- capability-record-format-v1.md
- service-directory-format-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/service-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/service-model.md -->
### Integrated source: `docs/service-model.md`

# ATARIX Service Model

Status: Baseline

## Purpose

This document defines the ATARIX service model.

ATARIX is service-addressed, not address-addressed. Applications and higher-level software do not directly access hardware. They access services. Services operate on resources. Resources are implemented by concrete hardware, firmware, simulation, or future node implementations.

## Required Protected-Access Path

All protected access follows this path:

```text
Application
    -> Service
    -> Capability Policy
    -> Resource
    -> Implementation
```

This means:

```text
Applications request service operations.
Services validate capabilities.
Capabilities authorize resource access.
Resources hide implementation details.
Implementations touch hardware or simulated hardware.
```

## Forbidden Access Path

The following pattern is forbidden as a public software contract:

```text
Application
    -> Physical Address
    -> Register
    -> Hardware
```

Examples of forbidden direct access:

```c
*(volatile unsigned int *)0xF0001000u = value;
```

```c
write_register(0xD123u, value);
```

unless such code exists inside a resource implementation layer that is already protected by service and capability policy.

## Visibility Is Not Authority

Discovery grants visibility.

Directory resolves and describes.

Capabilities authorize.

Knowing any of the following does not grant access:

```text
Service name
Handle
Physical address
Register offset
Slot number
Mailbox endpoint
Discovery record
Directory entry
```

## Example: Storage

The caller sees:

```text
Storage Service
```

The protected path is:

```text
Application
    -> Storage Service
    -> Storage Capability
    -> Block Resource
    -> Storage Implementation
```

The implementation may be:

```text
CF card
SD card
SATA device
Network block service
Simulator file
Future Fabric storage card
```

The caller must not depend on the implementation.

## Example: Display

The caller sees:

```text
Display Service
```

The protected path is:

```text
Application
    -> Display Service
    -> Display Capability
    -> Framebuffer Resource
    -> Display Implementation
```

The implementation may be:

```text
FPGA framebuffer
Local video card
Remote display node
Simulator framebuffer
```

## Example: GPIO

The caller sees:

```text
GPIO Service
```

The protected path is:

```text
Application
    -> GPIO Service
    -> GPIO Capability
    -> GPIO Resource
    -> Pin Controller Implementation
```

The caller does not directly manipulate pin registers.

## Simulator Rule

The simulator follows the same model as hardware.

Simulator code must not introduce privileged shortcuts that would be invalid on real hardware.

A simulated resource is still a resource. It is not permission to bypass services or capabilities.

## Service Contracts

A service contract should define:

```text
Service name
Operation identifiers
Request format
Response format
Required capability
Failure statuses
Versioning rules
Observability requirements
```

## Security Requirements

All protected services must:

```text
Deny by default.
Validate capability before access.
Reject malformed requests.
Report explicit failures.
Avoid exposing implementation addresses.
Avoid treating handles as authority.
```

## Result

The ATARIX service model enforces abstraction and security.

Hardware may change. Simulators may replace hardware. Future nodes may provide services. The caller still uses the same service-oriented access path.

<!-- AEMS-MIGRATED-SOURCE: docs/software.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/software.md -->
### Integrated source: `docs/software.md`

# ATARIX Software Architecture

## Philosophy

Software should grow alongside the hardware.

The project begins with monitor firmware and eventually evolves into a workstation operating system.

## Development Stages

### Stage 1

- ROM monitor
- Serial console
- Memory diagnostics
- Device diagnostics

### Stage 2

- Program loader
- Modular firmware services
- Networking support

### Stage 3

- Kernel experimentation
- Driver framework
- Device discovery

### Stage 4

- Capability broker
- DMA manager
- Multiprocessor services

### Stage 5

- Accelerator runtime
- Distributed services
- Heterogeneous compute framework

## Toolchain

Candidate tools:

- 64TASS
- WDC tools
- LLVM research paths
- Open-source cross-development tools

## Development Environment

- Vim
- Make
- Git
- Automated testing
- FPGA simulation

## Long-Term Goals

- Native operating system
- Capability-oriented services
- Hardware-assisted security
- Modular driver architecture

<!-- AEMS-MIGRATED-SOURCE: docs/supervisor-card-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/supervisor-card-v1.md -->
### Integrated source: `docs/supervisor-card-v1.md`

# ATARIX Supervisor Card Architecture v1

## Status

Draft hardware architecture specification.

This document defines the first-generation ATARIX supervisor card. The supervisor card implements the independent management plane for the ATARIX workstation architecture.

## Purpose

The supervisor card keeps the system observable, recoverable, and serviceable even when the W65C816 CPU, FPGA fabric controller, service cards, or accelerator cards have failed or are held in reset.

The supervisor is not a replacement for the CPU or fabric controller. It is a management and recovery authority.

## Design Rule

The supervisor must remain operational when the CPU and fabric are not booting.

It owns recovery, reset sequencing, watchdogs, RTC timekeeping, power-state monitoring, and fault logging.

## Architectural Position

```text
RTC / NVRAM / Fault Log
        |
Supervisor MCU
        |
Management Plane
        |
CPU Card / ULX3S Fabric Controller / Backplane Slots
```

The supervisor participates in the management plane, not the primary data plane.

## Design Goals

1. Provide deterministic reset sequencing.
2. Provide RTC-backed timekeeping.
3. Timestamp fault logs.
4. Monitor CPU, FPGA, and slot heartbeats.
5. Reset hung cards or the full system.
6. Support golden ROM and golden FPGA recovery.
7. Expose diagnostic information over UART or management interface.
8. Remain independent from the main CPU and FPGA fabric.
9. Support NTP-corrected RTC updates when the system network is available.

## Required Functional Blocks

### Supervisor MCU

Candidate MCU families:

```text
RP2350
RP2040
STM32
ESP32-S3
```

Preferred attributes:

- Multiple GPIOs.
- I2C/SPI/UART support.
- Nonvolatile configuration support.
- Watchdog timers.
- Low-power standby capability.
- Good open toolchain support.

### RTC

The supervisor card should include a battery-backed or supercapacitor-backed RTC.

Responsibilities:

- Maintain time when the system is powered off.
- Timestamp watchdog resets.
- Timestamp recovery-mode entries.
- Timestamp slot failures.
- Provide fallback time when NTP is unavailable.

NTP may correct the RTC at boot, but NTP does not replace the RTC.

See:

```text
docs/netboot-ntp-v1.md
```

### Fault Log Storage

The supervisor should include nonvolatile or retained fault-log storage.

Candidate storage:

```text
MCU internal flash
External SPI flash
I2C EEPROM
FRAM
Battery-backed SRAM
```

FRAM is attractive for repeated fault logging because it tolerates many writes.

Fault log entries should include:

```text
Timestamp
Fault source
Fault class
Reset cause
Slot number
Last heartbeat state
Power-good state
Fabric status if available
CPU status if available
```

### Watchdog Timers

The supervisor should monitor heartbeats from:

```text
Primary CPU Card
ULX3S Fabric Controller
Service Cards
Accelerator Cards
Future CPU Cards
```

Watchdog action levels:

```text
Level 0 Log warning
Level 1 Assert slot fault
Level 2 Reset offending slot
Level 3 Reset fabric controller
Level 4 Reset CPU card
Level 5 Enter recovery mode
```

Not every fault should trigger a full system reset.

### Reset Controller

The supervisor should control:

```text
Global Reset
CPU Slot Reset
Fabric Controller Reset
Per-Slot Reset
Recovery Reset
Manual Reset Input
Watchdog Reset Output
```

The reset controller must support holding selected slots in reset while allowing the supervisor to continue operating.

### Power Monitoring

The supervisor should monitor:

```text
+12V
+5V
+3.3V
+3.3V Standby
Optional local rails
```

Power faults should be logged before reset or shutdown if possible.

### Recovery Inputs

Required recovery controls:

```text
Recovery Button
Recovery DIP Switch
Golden ROM Select
Golden FPGA Bitstream Select
Fabric Disable
DMA Disable
Slow Clock / Debug Clock Enable
```

These may be physically located on the supervisor card, backplane, or ULX3S carrier, but the supervisor should be able to read their state.

## Management Plane

The supervisor communicates with system components over a low-speed management interface.

Candidate electrical interfaces:

```text
I2C
SPI
UART multidrop
RS-485 multidrop
CAN
Custom low-speed management bus
```

The management plane should be independent of the main ATARIX data plane.

Management-plane functions:

```text
Heartbeat collection
Slot identity readout
Reset control
Fault reporting
RTC read/write
Configuration read/write
Power-state reporting
Recovery-mode coordination
```

## Heartbeat Model

Each active card should provide a heartbeat.

Heartbeat states:

```text
Absent
Held In Reset
Booting
Alive
Warning
Faulted
Timed Out
```

A missing heartbeat should not immediately cause a reset unless the card is expected to be active.

## RTC and NTP Interaction

The CPU or network service may obtain NTP time during boot.

The supervisor should accept RTC updates only if:

```text
NTP result is valid
Time passes sanity checks
Update is authorized by boot policy
Time is newer than firmware build baseline
```

The supervisor should record whether time is:

```text
Unknown
RTC Only
NTP Synchronized
Manually Set
```

## Golden Recovery

The supervisor should support recovery modes:

```text
Golden ROM Boot
Golden FPGA Bitstream
CPU Held In Reset
Fabric Minimal Mode
Network Recovery Boot
UART Monitor Recovery
```

A failed firmware or FPGA update must not permanently brick the system.

## Diagnostic Interface

The supervisor should expose a diagnostic console.

Candidate interfaces:

```text
UART header
USB serial
Management bus command interface
Fabric mailbox endpoint after fabric startup
```

Minimum diagnostic commands:

```text
STATUS
RESET CPU
RESET FABRIC
RESET SLOT <n>
SHOW FAULTS
CLEAR FAULTS
SHOW RTC
SET RTC
SHOW POWER
SHOW HEARTBEATS
ENTER RECOVERY
```

## Integration With Boot Sequence

Supervisor startup occurs before CPU reset release.

Recommended sequence:

```text
Power stable
Supervisor boots
Power rails verified
Recovery inputs read
FPGA mode selected
Fabric reset controlled
CPU reset held
Fabric ready checked if required
CPU reset released
Boot monitored by heartbeat
```

See:

```text
docs/boot-sequence-v1.md
docs/clock-reset-spec.md
```

## Diagnostic Requirements

Required test points:

```text
Supervisor Clock
Supervisor Reset
RTC I2C/SPI Lines
Power Good Inputs
CPU Reset Output
Fabric Reset Output
Slot Reset Outputs
Watchdog Output
Recovery Input
Fault Log Write Indicator
Management Bus Lines
UART TX/RX
```

Required LEDs:

```text
Supervisor Alive
Power Good
Recovery Mode
CPU Reset Asserted
Fabric Reset Asserted
Fault Logged
Management Activity
```

See:

```text
docs/diagnostic-access-v1.md
```

## Security and Authority

The supervisor is the root management authority.

It may:

- Hold cards in reset.
- Select recovery paths.
- Record faults.
- Clear or preserve configuration.
- Authorize RTC updates.

It should not silently perform normal data-plane work.

The supervisor should not grant unrestricted DMA or accelerator access. Those remain governed by the fabric and capability model.

## Relationship to Fabric Controller

The ULX3S/ECP5 fabric controller owns main fabric services.

The supervisor owns management and recovery.

The supervisor may reset or configure the fabric controller, but it should not replace the fabric controller during normal operation.

See:

```text
docs/ulx3s-backplane-controller.md
docs/fpga-fabric.md
```

## Initial Rev A Supervisor Recommendation

The first supervisor card should include:

```text
MCU
RTC
FRAM or EEPROM fault log
Reset-control outputs
Power-good inputs
Recovery DIP/button inputs
UART/USB diagnostic console
Management bus interface
Status LEDs
Test points
```

## Open Questions

- Final MCU selection.
- RTC part selection.
- Fault-log storage technology.
- Management-bus electrical standard.
- Whether supervisor is a separate card or integrated into the backplane for Rev A.
- Number of independently resettable slots.
- Whether the supervisor directly loads FPGA bitstreams or only selects boot mode.
- Whether NTP updates are pushed by CPU firmware or pulled through a supervisor network path.
- Exact fault-log binary format.

## Design Principle

The supervisor is the system's service processor.

It exists so ATARIX can fail visibly, recover deliberately, and retain enough information to diagnose what happened.

<!-- AEMS-MIGRATED-SOURCE: docs/testing-strategy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/testing-strategy.md -->
### Integrated source: `docs/testing-strategy.md`

# ATARIX Testing Strategy

## Status

Draft project testing specification.

This document defines the testing philosophy, test-suite structure, compatibility goals, and abstraction requirements for ATARIX.

## Purpose

ATARIX is a modular hardware, firmware, FPGA, and operating-system research project. It needs a testing strategy that can survive future CPU cards, fabric revisions, memory-service changes, accelerator cards, and protocol revisions.

Tests should validate behavior, not just one implementation.

## Influences

The `dlworrell/65x02` repository provides a useful testing model for CPU reimplementation work:

- Each test executes only a single instruction.
- Each test provides full processor and memory state before and after execution.
- Tests are generated in large volume.
- Test methodology is documented.
- Test data formats are manually versioned for future breaking changes.

ATARIX should borrow these principles and apply them more broadly across CPU, firmware, FPGA fabric, DMA, mailbox, discovery, supervisor, security, and system tests.

## Design Principles

1. Test behavior, not implementation details.
2. Keep test formats versioned.
3. Make tests reproducible.
4. Preserve before-and-after state where practical.
5. Prefer small targeted tests over vague integration tests.
6. Support large randomized test sets.
7. Make failures diagnosable.
8. Keep tests abstract enough to support future CPU cards and fabric revisions.
9. Avoid hard-coding W65C816 assumptions into tests unless the test is explicitly W65C816-specific.
10. Treat failing tests as engineering data.

## Abstraction Rule

Tests should target stable architectural interfaces.

Examples of stable interfaces:

```text
Mailbox protocol
DMA descriptors
Discovery records
Capability records
Register semantics
Supervisor commands
Boot image format
Memory object model
```

Tests should avoid depending on temporary implementation details unless explicitly marked as implementation-specific.

## Test Categories

### CPU Instruction Tests

CPU instruction tests should follow the 65x02-style model where practical.

Each test should define:

```text
Initial CPU state
Initial memory state
Instruction or operation
Expected CPU state
Expected memory state
Expected cycles or timing, if applicable
Expected exceptions or faults, if applicable
```

W65C816 tests may be CPU-specific.

Future CPU cards should have their own CPU-specific tests while still sharing system-level tests.

### Firmware Tests

Firmware tests should cover:

```text
Reset vector behavior
ROM monitor commands
Memory tests
Discovery parsing
Mailbox commands
DMA submission
Netboot stages
Recovery paths
Error handling
```

### FPGA Fabric Tests

Fabric tests should cover:

```text
Register reads and writes
Mailbox routing
DMA descriptor parsing
Interrupt routing
Discovery response generation
Capability lookup assistance
Window mapping
Fault reporting
Counter behavior
Reset behavior
```

### Protocol Tests

Protocol tests should validate binary structures against the data model.

Required checks:

```text
Little-endian encoding
Field widths
Reserved fields zeroed
Version handling
Unknown optional fields ignored
Unknown mandatory fields rejected
Length and bounds checking
```

Relevant protocols:

```text
Discovery ROM format
Mailbox protocol
DMA descriptors
Capability records
Boot image headers
Supervisor fault logs
```

### Security Tests

Security tests should verify:

```text
Denied access without capability
DMA rejection without valid grants
Capability expiration
Capability revocation
Ring policy enforcement
Device quarantine
Supervisor recovery after violation
Audit event generation
```

### Supervisor Tests

Supervisor tests should verify:

```text
RTC read/write
NTP correction policy
Watchdog escalation
Reset sequencing
Fault logging
Slot quarantine
Recovery mode
Power-good handling
Anomaly scoring
```

### Hardware Bring-Up Tests

Hardware tests should verify:

```text
Power rails
Clock distribution
Reset lines
Bus signal integrity
JTAG/SWD access
UART console
Diagnostic LEDs
Test-point availability
Logic analyzer headers
```

### Integration Tests

Integration tests should combine subsystems carefully.

Examples:

```text
Boot to monitor
Discovery enumeration
Mailbox ping
DMA loopback
Netboot image fetch
Supervisor watchdog recovery
Memory object window mapping
Accelerator command submission
```

## Test Format Versioning

All machine-readable test formats must include:

```text
Format name
Format major version
Format minor version
Target architecture or subsystem
Required feature flags
```

Major version changes may break compatibility.

Minor version changes should be backward-compatible.

## Test Vector Structure

A generic test vector should include:

```text
Test ID
Description
Target subsystem
Required capabilities
Initial state
Operation
Expected final state
Expected faults
Expected logs
Timing expectations, if any
Format version
```

## Randomized Testing

ATARIX should support randomly generated tests where practical.

Examples:

```text
CPU instruction tests
DMA descriptor fuzzing
Mailbox payload fuzzing
Discovery TLV fuzzing
Capability table fuzzing
Register access order tests
```

Randomized tests must record their seed so failures can be reproduced.

## Golden Reference Testing

Where possible, ATARIX should compare behavior against a known-good reference.

Examples:

```text
CPU emulator
Protocol parser reference implementation
Fabric simulation model
Supervisor command simulator
DMA descriptor validator
```

## CI Integration

The repository should eventually run tests through GitHub Actions.

Initial CI targets:

```text
Markdown link checks
Protocol format linting
Schema validation
Unit tests for tools
Firmware build smoke tests
HDL linting
Simulation smoke tests
```

Later CI targets:

```text
Verilator or equivalent simulation
CPU emulator test runs
DMA simulation
Mailbox simulation
Discovery parser tests
Capability validation tests
```

## Hardware-in-the-Loop Testing

Hardware-in-the-loop tests should be added once prototype boards exist.

They should be able to run:

```text
Power-on smoke test
UART monitor test
Supervisor status test
Mailbox loopback
DMA loopback
Reset recovery
Clock scaling
Fault injection
```

## Directory Structure

Recommended structure:

```text
tests/
  README.md
  cpu/
  firmware/
  fabric/
  protocols/
  security/
  supervisor/
  hardware/
  integration/
  vectors/
  schemas/
  tools/
```

## Required Test Documentation

Each test suite should document:

```text
Purpose
Scope
Assumptions
Required hardware or emulator
How to run
Expected output
Failure interpretation
Format version
```

## Compatibility Rule

Tests should assume that ATARIX may eventually include:

```text
W65C816 CPU cards
Future 32-bit CPU cards
Future 64-bit CPU cards
Multiple fabric-controller revisions
Memory-service cards
Storage-service cards
Accelerators
Different supervisor revisions
```

Only CPU-specific tests should assume a specific CPU.

System-level tests should target abstract architectural interfaces.

## Design Principle

Test the architecture as a set of stable contracts.

Implementations may change.

Contracts should remain testable.

<!-- AEMS-MIGRATED-SOURCE: docs/trust-model-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/trust-model-v1.md -->
### Integrated source: `docs/trust-model-v1.md`

# Trust Model v1

Status: Draft v1

## Purpose

This document defines the ATARIX trust model.

Trust answers:

```text
How much confidence should the platform place in this card, provider, service, or resource?
```

Trust does not identify hardware.

Trust does not describe resources.

Trust does not grant capabilities by itself.

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Trust States

Initial trust states:

```text
UNTRUSTED
IDENTIFIED
VALIDATED
TRUSTED
PRIVILEGED
REVOKED
```

## State Definitions

### UNTRUSTED

The platform has insufficient confidence in the entity.

This may occur when:

- identity is missing
- identity validation failed
- discovery validation failed
- policy rejects the entity
- the entity is explicitly blocked

### IDENTIFIED

The entity has a valid identity source.

Identity proves existence only.

IDENTIFIED does not imply operational authority.

### VALIDATED

The entity has passed required validation steps.

Examples:

- Identity EEPROM CRC valid
- Discovery ROM CRC valid
- health checks acceptable
- provisioning records acceptable

### TRUSTED

The entity is trusted by platform policy for normal participation.

TRUSTED does not imply privileged authority.

### PRIVILEGED

The entity is trusted for elevated platform functions.

Examples:

- supervisor-adjacent services
- recovery services
- boot authority
- fabric control
- privileged instrumentation

### REVOKED

The entity is explicitly denied by policy.

Revocation overrides identity, discovery, and prior trust.

## Homebrew Policy

Homebrew cards may be:

```text
Identified
Validated
Enumerated
```

without becoming TRUSTED or PRIVILEGED.

Default homebrew posture:

```text
Identity:      Allowed if valid
Discovery:     Allowed if valid
Validation:    Allowed if safe
Enumeration:   Allowed if policy permits
Capabilities:  Restricted
Supervisor:    Denied
DMA:           Denied unless explicitly authorized
Boot:          Denied unless explicitly authorized
```

## Trust Evaluation Inputs

Trust may consider:

- Identity EEPROM validation state
- Discovery ROM validation state
- board class
- trust class
- provider ID
- provisioning state
- factory test signature
- supervisor policy
- development-mode switch
- ring policy
- future cryptographic signatures
- revocation lists

## Trust and Rings

Trust and rings are separate.

Trust describes platform confidence.

Rings describe authority boundaries.

A trusted service may still be unavailable to Ring 3.

A privileged service may be visible to Ring 0 but controlled only by Ring -2.

## Trust and Capabilities

Capabilities authorize operations.

Trust evaluation determines whether capabilities may be accepted, reduced, or rejected.

A capability from an untrusted or revoked entity shall not be accepted for normal operation.

## Trust Transitions

Typical transition:

```text
UNTRUSTED
    ↓
IDENTIFIED
    ↓
VALIDATED
    ↓
TRUSTED
```

Privileged transition:

```text
TRUSTED
    ↓
PRIVILEGED
```

Revocation:

```text
ANY STATE
    ↓
REVOKED
```

## Recovery Rule

Recovery must not depend on trusted directory services.

Ring -2 supervisor authority must retain enough visibility to diagnose and recover from trust failures.

## Related Documents

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- ADR-RING-SECURITY-MODEL.md
- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- capability-record-format-v1.md


---

# Chapter 6: Human Readability


---

# Chapter 7: Fabric-Centric Computing

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0001-fabric-centric-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0001-fabric-centric-architecture.md -->
### Integrated source: `docs/adr/ADR-0001-fabric-centric-architecture.md`

# ADR-0001 Fabric-Centric Architecture

Status: Accepted

Date: 2026-06-11

## Context

The project must support future processor cards, capability-mediated services, and a workstation-class expansion model.

## Decision

The W65C816 local bus remains local to the CPU card.

All system-wide communication occurs through the Fabric Northbridge.

## Consequences

Benefits:

- Heterogeneous processor support
- Stable service model
- Improved observability
- Easier DMA integration

Costs:

- Additional latency
- More FPGA complexity
- More firmware infrastructure

## Future Revisions

Refine mailbox, DMA, and service contracts.

<!-- AEMS-MIGRATED-SOURCE: docs/fabric-controller-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/fabric-controller-architecture-v1.md -->
### Integrated source: `docs/fabric-controller-architecture-v1.md`

# Fabric Controller Architecture v1

## Purpose

The Fabric Controller is the architectural center of ATARIX.

It provides transport, routing, discovery support, DMA services, interrupt routing, mailbox transport, memory services, and future multi-processor coordination.

The Fabric Controller is implemented initially using the ULX3S ECP5 FPGA platform.

## Design Philosophy

ATARIX is not a shared-bus computer.

The W65C816 local bus remains local to the CPU card.

System-wide communication occurs through the FPGA fabric.

```text
CPU Card
    <-> Fabric Bridge
    <-> Fabric Controller
    <-> Services
```

## Architectural Responsibilities

The Fabric Controller shall provide:

- Endpoint routing
- Service routing
- Mailbox transport
- DMA services
- Interrupt routing
- Memory services
- Resource discovery support
- Multi-processor coordination
- Diagnostic instrumentation

## Architectural Planes

### Management Plane

Implemented by the RP2350 supervisor.

Functions:

- Reset
- Watchdog
- RTC
- Health monitoring
- Recovery
- Fault logging

### Discovery Plane

Implemented through Discovery ROM structures.

Functions:

- Device discovery
- Service publication
- Resource publication
- Capability publication

### Data Plane

Implemented by the Fabric Controller.

Functions:

- DMA
- Mailboxes
- Memory access
- Service communication
- Accelerator access

## Endpoint Model

Every service exposed to the system receives an endpoint identifier.

```text
Endpoint ID
    -> Mailbox
    -> Capabilities
    -> Resources
```

Endpoints may be backed by:

- Physical cards
- FPGA logic
- Supervisor services
- Future CPU cards

## Mailbox Routing

The Fabric Controller routes mailbox traffic.

```text
Request
    -> Fabric Router
    -> Destination Endpoint

Response
    -> Fabric Router
    -> Source Endpoint
```

Mailbox protocol details are defined separately.

## DMA Engine

The Fabric Controller owns DMA execution.

Responsibilities:

- Memory transfers
- Storage transfers
- Network transfers
- Accelerator transfers
- Scatter/gather support

The DMA engine should reduce CPU involvement in large data movement.

## Interrupt Router

Interrupt sources are abstracted from physical hardware.

```text
Interrupt Source
    -> Fabric Router
    -> CPU Target
```

This enables future multi-processor systems.

## Memory Services

The Fabric Controller provides memory service abstractions.

Memory resources may exist:

- On CPU cards
- On memory cards
- Within FPGA-attached resources

The service model allows future expansion without exposing implementation details.

## Service Directory

The Fabric Controller maintains a runtime directory of services.

Example:

```text
Storage Service
DMA Service
Network Service
Framebuffer Service
Memory Service
```

Operating systems query the directory rather than assuming slot locations.

## Multi-Service Devices

One physical device may expose multiple services.

Example:

```text
Storage Card

    Storage Service
    DMA Service
    Compression Service
```

Each service receives a unique endpoint.

## Multi-Processor Direction

Future processors may include:

- W65C816
- 68000-family
- RISC-V

The Fabric Controller serves as the common communication substrate.

## Diagnostic Instrumentation

The Fabric Controller should expose:

- Traffic counters
- DMA statistics
- Mailbox statistics
- Interrupt statistics
- Error counters
- Trace facilities

System observability is a primary design goal.

## Design Goals

1. Fabric-centric architecture.
2. Service-oriented communication.
3. DMA-first transport.
4. Slot-independent discovery.
5. Capability-mediated access.
6. Hardware abstraction through services.
7. Future heterogeneous CPU support.
8. Extensive observability and diagnostics.

<!-- AEMS-MIGRATED-SOURCE: docs/fabric-service-model-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/fabric-service-model-v1.md -->
### Integrated source: `docs/fabric-service-model-v1.md`

# Fabric Service Model v1

## Purpose

This document defines the service-oriented architecture used by ATARIX.

The goal is to decouple operating-system interfaces from physical hardware implementation details.

Applications and operating-system drivers interact with services.

Services may be implemented by:

- Physical cards
- FPGA fabric logic
- Supervisor firmware
- Future processor cards

## Core Principle

```text
Driver
   -> Service

Service
   -> Fabric Endpoint

Fabric Endpoint
   -> Hardware Implementation
```

The operating system should not assume a specific slot number or physical implementation.

## Service Classes

### Core Services

- Console
- Timer
- RTC
- Interrupt Controller
- DMA Engine

### Memory Services

- RAM
- Persistent Memory
- Cache Services

### Storage Services

- Block Storage
- Boot Services
- Filesystem Services

### Network Services

- Ethernet
- Time Services
- Network Boot Services

### Graphics Services

- Framebuffer
- 2D Acceleration
- 3D Acceleration

### Compute Services

- FPGA Compute
- DSP Services
- AI Acceleration

### Management Services

- Supervisor
- Health Monitoring
- Power Control
- Fault Logging

## Service Descriptor

```c
struct atarix_service_descriptor {
    uint16_t service_class;
    uint16_t service_type;

    uint16_t version_major;
    uint16_t version_minor;

    uint32_t endpoint_id;

    uint32_t capabilities;
    uint32_t resource_handle;

    uint32_t mailbox_rx;
    uint32_t mailbox_tx;
};
```

## Discovery Relationship

Discovery ROMs publish services.

```text
Discovery ROM
    -> Service Descriptors

Firmware
    -> Service Directory

Operating System
    -> Service Binding
```

## Service Directory

Firmware builds a service directory during initialization.

The operating system may query:

```text
Find Storage Services
Find Network Services
Find Display Services
Find DMA Services
```

rather than searching for specific slots.

## Mailbox Communication

Services communicate through mailbox interfaces.

```text
Request
    -> Mailbox

Fabric Routing

Response
    -> Mailbox
```

Mailbox protocols are defined separately.

## DMA-Oriented Operation

Services should prefer DMA-based transfers.

The CPU should coordinate transfers rather than moving large volumes of data directly.

## Multi-Service Devices

A single physical card may expose multiple services.

Example:

```text
Storage Card

    Storage Service
    DMA Service
    Compression Service
```

The operating system binds to services individually.

## Fabric-Native Services

Some services may exist entirely inside the FPGA fabric.

Example:

```text
DMA Engine
Interrupt Router
Mailbox Router
Memory Service
```

These services need not correspond to physical cards.

## Future Multi-CPU Support

The service model supports future heterogeneous processor cards.

Examples:

```text
W65C816 CPU Card
68K CPU Card
RISC-V CPU Card
```

All processors communicate with services through the same architectural model.

## Design Goals

1. Hardware independence.
2. Service-oriented discovery.
3. DMA-first operation.
4. Fabric-centric communication.
5. Future multi-processor support.
6. Capability-mediated access control.
7. Clean separation of implementation and interface.

<!-- AEMS-MIGRATED-SOURCE: docs/nubus-upa-influences-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/nubus-upa-influences-v1.md -->
### Integrated source: `docs/nubus-upa-influences-v1.md`

# NuBus and UPA Architectural Influences

## Purpose

This document explains how ATARIX incorporates concepts inspired by Apple NuBus systems and Sun Ultra Port Architecture (UPA) systems while remaining a distinct architecture.

ATARIX is not electrically compatible with either NuBus or UPA.

The project borrows architectural ideas rather than bus implementations.

## Design Summary

```text
NuBus
    -> Discovery

UPA
    -> Transport

Supervisor Plane
    -> Management

ATARIX
    -> Integrated Architecture
```

## NuBus Influence

NuBus contributed the concept of self-describing hardware.

Key concepts adopted by ATARIX:

- Card self-identification
- Device discovery
- Resource publication
- Slot-independent configuration
- Operating-system enumeration

### NuBus Mapping

| NuBus | ATARIX |
|---------|---------|
| Declaration ROM | Discovery ROM |
| Slot Manager | Firmware Discovery Service |
| Board ID | Vendor ID + Device ID |
| Resource Directory | Resource Descriptors |
| Slot Enumeration | Fabric Enumeration |

## UPA Influence

UPA contributed the concept of high-performance transport separated from configuration and discovery.

Key concepts adopted by ATARIX:

- Service-oriented communication
- High-bandwidth transport paths
- DMA-centric operation
- Accelerator-oriented architecture
- Separation of control and data movement

### UPA Mapping

| UPA | ATARIX |
|------|---------|
| UPA Port | Fabric Endpoint |
| Graphics Accelerator | Fabric Service |
| Memory Service | Memory Service |
| DMA Transport | DMA Engine |
| High-Speed Interconnect | ECP5 Fabric |

## Three-Plane Architecture

ATARIX intentionally separates management, discovery, and transport.

### Management Plane

Implemented by the RP2350 supervisor.

Responsibilities:

- Power control
- Reset control
- RTC
- Watchdog
- Health monitoring
- Fault logging
- Recovery

### Discovery Plane

Implemented through Discovery ROM structures and firmware enumeration.

Responsibilities:

- Device identification
- Resource publication
- Capability publication
- Boot-service discovery

### Data Plane

Implemented by the FPGA fabric.

Responsibilities:

- DMA
- Memory services
- Storage services
- Networking
- Video
- Accelerators
- Mailboxes

## Service-Oriented Design

Traditional retrocomputers often expose physical devices directly.

ATARIX instead prefers service exposure.

Example:

```text
Storage Card
    -> Storage Service
    -> DMA Service
    -> Compression Service
```

The operating system interacts with services rather than assuming fixed hardware layouts.

## Discovery ROM Philosophy

The Discovery ROM is intended to combine concepts from:

```text
NuBus Declaration ROM
+
Open Firmware Device Tree
+
PCI Capability Lists
+
Fabric Resource Descriptors
=
ATARIX Discovery ROM
```

## Fabric-Centric Architecture

Traditional systems:

```text
CPU
  <-> Shared Bus
  <-> Devices
```

ATARIX:

```text
CPU Card
    <-> Fabric Bridge
    <-> ECP5 Fabric
    <-> Services
```

The fabric becomes the primary transport mechanism.

## Architectural Goals

1. Self-describing hardware.
2. Slot-independent configuration.
3. Service-oriented communication.
4. DMA-first design.
5. Capability-mediated access.
6. Separation of management, discovery, and transport.
7. Support for heterogeneous future CPU cards.
8. Support for FPGA-defined services and accelerators.

## Historical Lineage

```text
Apple II
    -> Physical expansion slots

NuBus
    -> Self-describing hardware

Sun UPA
    -> Service-oriented high-performance transport

Modern Service Processors
    -> Independent management plane

ATARIX
    -> Combination of all four approaches
```


---

# Chapter 8: Engineering as Architecture


---

---
document: ATX-100-CH09
title: System Layers
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-001
  - ATX-SPEC-002
  - ATX-SPEC-003
  - ATX-SPEC-005
  - ATX-SPEC-015
---

# Chapter 9: System Layers

## Layer Model

Atarix is organized as layered architecture rather than a monolithic bus-oriented machine.

```text
Applications
  -> Compatibility Personalities
  -> Native Services
  -> Object Model
  -> Capability and Policy Enforcement
  -> Mailbox Transport
  -> Fabric Services
  -> Hardware Modules
```

## Architectural Intent

The layer model prevents implementation convenience from becoming architectural authority.

A higher layer may request service from a lower layer. It may not bypass native authority, lifecycle, recovery, or audit requirements.

## Compatibility Personalities

Compatibility personalities translate foreign programming models into native Atarix objects, capabilities, and service requests.

A compatibility personality may emulate POSIX, firmware-style APIs, or other environments. It does not inherit the authority assumptions of those environments.

## Native Services

Native services expose Atarix semantics directly.

They participate in capability enforcement, mailbox routing, audit windows, recovery domains, and version-aware behavior.

## Hardware Services

Hardware services are accessed through explicit fabric-mediated interfaces.

Hardware acceleration may reduce latency or CPU overhead, but accelerated paths must preserve the same authority, audit, lifecycle, and recovery semantics as non-accelerated paths.

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-013-Policy-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-013-Policy-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-013-Policy-Model.md`

# ATX-SPEC-013 Policy Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Policy Model.

The Policy Model defines how Atarix evaluates, approves, denies, constrains, delegates, revokes, audits, and reconciles authority-bearing operations.

Policy is not authority.

Policy governs whether authority may be exercised.

## Core Rule

```text
Authority is explicit.
Policy determines whether authority may be exercised.
```

A valid capability describes what could be done.

Policy decides whether it may be done now.

## Architectural Axioms

```text
Policy is not identity.
Policy is not authority.
Policy is not ownership.
Policy is not a capability.
Policy evaluation must be auditable.
Unknown policy state must be explicit.
Policy failure must fail closed.
```

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model

This specification informs:

- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## What Policy Governs

Policy may govern:

```text
Capabilities
Delegation
Revocation
Directory mutation
Resource allocation
Resource reservation
Service registration
Mailbox communication
Lifecycle transitions
Persistence
Recovery actions
Supervisor requests
Compatibility modes
Version negotiation
```

## Policy Layers

Atarix recognizes policy layers.

### Layer 0: Architectural Policy

Hard architectural invariants.

These cannot be overridden.

Examples:

```text
Identity is not authority.
Lookup is not access.
Observation is not control.
Unknown authority fails closed.
```

### Layer 1: Security Policy

Global security rules.

Examples:

```text
No delegation across rings without explicit approval.
Recovery operations require approved authority.
Supervisor requests require mediated approval.
```

### Layer 2: Administrative Policy

Site-specific administrative rules.

Examples:

```text
Maximum RAM allocation
Audit retention period
Allowed service providers
Persistent storage limits
```

### Layer 3: Object Policy

Object-specific rules.

Examples:

```text
Storage object is read-only.
Directory branch is immutable.
Resource pool has quota.
```

### Layer 4: Session Policy

Temporary or session-scoped rules.

Examples:

```text
Maintenance window
Recovery session
Emergency operation
Temporary override
```

Session policy should expire by default.

## Evaluation Order

Policy evaluation must be deterministic.

Conceptually:

```text
Architectural Policy
  -> Security Policy
  -> Administrative Policy
  -> Object Policy
  -> Session Policy
```

Deny overrides allow.

Unknown policy state fails closed for authority-bearing operations.

## Policy Results

Policy evaluation may return:

```text
ALLOW
DENY
DEFER
REQUIRES_APPROVAL
QUARANTINE
UNKNOWN
```

Policy results are not merely Boolean.

## Policy Object

Policy records are objects or object-owned records.

A policy record has:

```text
Policy ID
Version
Generation
Owner
Scope
Lifecycle state
Audit policy
```

Version describes schema compatibility.

Generation describes policy evolution history.

## Policy Scope

Policy may apply to:

```text
System
Fabric
Node
Service
Directory
Resource
Object
User-associated label
Session
Capability
Supervisor request
Compatibility environment
```

## Policy Inheritance

Policies may inherit or compose by scope.

Example:

```text
System
  -> Storage
      -> Dataset
          -> Object
```

Child policy may restrict further.

Child policy may not weaken architectural policy.

## Policy And Authority

Policy does not create authority by itself.

Policy can allow, deny, restrict, require approval, quarantine, or defer use of existing authority.

Policy cannot turn identity, lookup, observation, connectivity, or execution into authority.

## Policy And Capabilities

A capability may be valid but still denied by policy.

A policy may not allow an operation when the required authority does not exist unless a higher authority explicitly defines that exceptional path.

## Policy And Delegation

Delegation requires policy approval.

Policy may:

```text
Allow delegation
Restrict delegation
Deny delegation
Require approval
Require reduced authority
Require expiration
Require audit
```

Delegation may never exceed the source authority.

## Policy And Resources

Policy may constrain resources such as:

```text
Memory
CPU
Storage
Bandwidth
DMA windows
Mailbox capacity
Audit resources
Compatibility environments
Auxiliary compute providers
```

Resource exhaustion policy must not grant broader authority.

## Policy And Supervisor Requests

Supervisor actions are policy-governed.

Examples:

```text
Reset
Power off
Recovery mode
Firmware update
Clock modification
Watchdog modification
```

The Operational Fabric may request supervisor action only through mediated policy-controlled paths.

Supervisor control isolation remains in effect.

## Policy And Compatibility

Compatibility environments are policy-controlled.

Examples:

```text
POSIX personality
Virtual hardware
Legacy protocol translation
Compatibility VM
```

Policy may disable compatibility, restrict it, require quarantine, require audit, or require explicit approval.

Compatibility must not weaken native Atarix semantics.

## Policy And Versioning

Every policy record must contain:

```text
Policy Version
Policy Generation
```

Unknown policy versions must fail closed for authority-bearing operations.

Policy downgrade, migration, or rollback must be auditable.

## Policy And Audit

Every authority-bearing policy decision should be auditable.

Minimum audit fields:

```text
Policy ID
Policy Version
Policy Generation
Actor identity label
Authority used
Operation
Target object or resource
Result
Reason
Timestamp
```

## Policy And Errors

If policy cannot be evaluated, the result is UNKNOWN.

UNKNOWN fails closed for authority-bearing operations.

Policy errors must distinguish:

```text
Policy missing
Policy malformed
Policy unsupported
Policy version unsupported
Policy unavailable
Policy conflict
Policy evaluation failed
```

## Policy Cleanup Escalation Model

Policy may fail, deny, defer, or become unavailable while cleanup is required.

Cleanup is a security function.

Policy failure must not cause garbage, leaked authority, leaked resources, or unbounded persistence.

Atarix therefore defines a cleanup fallback ladder.

### Cleanup Levels

```text
L1 Normal Cleanup
L2 Owner Cleanup
L3 Parent / Session Cleanup
L4 System Lifecycle Cleanup
L5 Recovery Cleanup
L6 Supervisor Recovery Cleanup
L7 Evidence Preservation
L8 Last-Resort Containment
```

### L1 Normal Cleanup

Normal cleanup under the object's active cleanup policy.

Typical actions:

```text
Release temporary resources
Close mailboxes
Remove temporary directory bindings
Expire leases
Flush ordinary audit records
```

### L2 Owner Cleanup

Cleanup under owner authority or owner cleanup policy when object-local cleanup is denied, incomplete, or unavailable.

Typical actions:

```text
Retry cleanup with owner cleanup capability
Release owner-scoped temporary allocations
Mark unresolved state for audit
```

### L3 Parent / Session Cleanup

Cleanup under parent object, session, or scope authority when the owner is unavailable.

Typical actions:

```text
Expire session leases
Mark objects orphan-pending
Revoke session-external capabilities
Audit owner unavailability
```

### L4 System Lifecycle Cleanup

System lifecycle cleanup when object, owner, and session cleanup cannot safely complete.

Typical actions:

```text
Transition object to cleanup-pending
Revoke outward authority
Release safe resources
Mark unsafe resources for quarantine
```

### L5 Recovery Cleanup

Recovery cleanup when ordinary lifecycle cleanup cannot establish safe state.

Typical actions:

```text
Quarantine object or resource
Deny new authority
Preserve recovery metadata
Require reconciliation
```

### L6 Supervisor Recovery Cleanup

Supervisor-mediated recovery cleanup for failures affecting recovery paths, boot state, power state, watchdogs, or critical system state.

Typical actions:

```text
Local-only recovery action
Recovery-mode transition
Supervisor audit event
No new runtime authority
```

### L7 Evidence Preservation

Evidence-preservation fallback when destructive cleanup may destroy information needed for audit, recovery, or incident reconstruction.

Typical actions:

```text
Stop destructive cleanup
Snapshot or preserve state where possible
Quarantine preserved state
Audit heavily
Require manual or recovery review
```

### L8 Last-Resort Containment

Last-resort containment when cleanup safety is unknown and continued exposure is riskier than isolation.

Typical actions:

```text
Revoke external capabilities
Disconnect service endpoints
Freeze mutation
Preserve recovery path
Keep evidence intact where practical
```

## Cleanup Result Mapping

Policy result maps to cleanup behavior as follows:

```text
ALLOW
  -> Normal cleanup may proceed.

DENY
  -> Destructive cleanup denied. Attempt safe release only and escalate if needed.

DEFER
  -> Hold object, retry policy evaluation, and prevent new authority.

REQUIRES_APPROVAL
  -> Suspend object, preserve state, audit, and alert.

QUARANTINE
  -> Isolate object, revoke external authority, preserve evidence.

UNKNOWN
  -> Fail closed and use recovery-only cleanup path.
```

## Cleanup Escalation Rules

Cleanup escalation follows these rules:

```text
Cleanup must never create broader authority.
Cleanup must never silently delete evidence.
Cleanup must never silently preserve authority.
If cleanup safety is unknown, quarantine beats deletion.
All escalation steps must be auditable.
Escalation may reduce authority.
Escalation may revoke authority.
Escalation may quarantine.
Escalation may release temporary resources when safe.
Escalation may not grant new runtime authority.
```

## Policy Lifecycle

Policies participate in lifecycle management.

Suggested states:

```text
CREATED
ACTIVE
SUSPENDED
DEPRECATED
QUARANTINED
DESTROYED
UNKNOWN
```

Policies are not immortal.

Policy cleanup and replacement must be auditable.

## Policy Language

A future policy language may be defined later.

Rev A should prefer:

```text
Static policies
Structured policies
Table-driven policies
Explicit C-visible policy records
```

before introducing scripting, rules engines, or policy DSLs.

## Initial Policy Sprint Scope

Policy Sprint 1 should implement:

```text
Policy result enum
Policy record metadata
Version and generation fields
Basic allow / deny / unknown evaluation
Cleanup fallback level enum
Audit hooks for policy decisions
Basic tests
```

Policy Sprint 1 should not implement:

```text
Policy language
Distributed policy federation
Complex inheritance
Runtime scripting
Remote administrative policy
```

## Required Tests

Initial tests should verify:

```text
Allow decision
Deny decision
Unknown decision fails closed
Policy version unsupported fails closed
Capability valid but policy denied
Delegation denied by policy
Resource allocation denied by policy
Cleanup escalation from L1 to L5
Evidence-preservation fallback
Last-resort containment fallback
Audit record generated for policy decision
Audit record generated for cleanup escalation
```

## Required Future Work

- Define policy record wire format.
- Define policy result C API.
- Define policy conflict resolution.
- Define policy inheritance rules.
- Define supervisor request policy in ATX-SPEC-016.
- Define bootstrap policy in ATX-SPEC-014.
- Define persistent policy storage in ATX-SPEC-017.
- Define recovery policy reconciliation in ATX-SPEC-018.

## Summary

Capabilities describe what could be done.

Policy decides whether it may be done now.

When policy cannot decide, Atarix fails closed, audits the uncertainty, and escalates cleanup toward quarantine, evidence preservation, or supervisor-mediated recovery rather than permissive deletion or leaked authority.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md`

# ATX-SPEC-015 POSIX Compatibility Model

## Status

Draft v0.1 scaffold

## Purpose

This document defines the Atarix POSIX Compatibility Model.

The POSIX Compatibility Model describes how Unix-style applications and interfaces are represented on top of native Atarix objects, services, capabilities, policies, mailboxes, audit, lifecycle, and recovery semantics.

## Core Rule

```text
POSIX is a compatibility personality.
POSIX is not native authority.
```

POSIX compatibility exists to run software.

POSIX compatibility must not weaken native Atarix authority, policy, audit, lifecycle, recovery, or supervisor isolation rules.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

This specification informs:

- ATX-SPEC-020 Hash Table and Lookup Acceleration Model
- ATX-SPEC-021 Memory and Data Movement Model

## Design Philosophy

POSIX compatibility is a boundary layer.

The POSIX view is a projection.

The Atarix model remains authoritative.

A compatibility environment may expose familiar concepts such as:

```text
Processes
Threads
Files
Directories
Descriptors
Pipes
Sockets
Signals
Environment variables
Working directories
Permission metadata
```

Internally, these must map to:

```text
Objects
Capabilities
Services
Mailbox endpoints
Resource allocations
Policy decisions
Audit records
Lifecycle states
Recovery state
```

The compatibility model should preserve useful Unix application behavior without importing Unix assumptions as native Atarix security semantics.

## Compatibility Environment

A POSIX compatibility environment is an object or service-owned environment.

Suggested service path:

```text
/service/compatibility/posix
```

A compatibility environment has:

```text
Environment identity
Version
Generation
Owner
Policy scope
Lifecycle state
Resource limits
Namespace view
Capability set
Audit policy
Recovery policy
```

The compatibility environment is the container for the POSIX projection.

It is not a privileged native execution mode.

It may be created, started, suspended, degraded, recovered, quarantined, or destroyed according to ordinary Atarix lifecycle and policy rules.

## Compatibility Boundary

The compatibility boundary is the point where POSIX-facing assumptions are translated into Atarix-native operations.

Across this boundary:

```text
POSIX names become namespace lookups.
POSIX descriptors become compatibility handles backed by capabilities.
POSIX process identifiers become compatibility labels for execution objects.
POSIX permission metadata becomes policy input.
POSIX streams become service or mailbox-backed objects.
POSIX sockets become network service endpoints.
```

The translation must be explicit.

The compatibility layer must not allow an application to obtain native authority merely because a POSIX interface normally implies it on another system.

## Native Model Remains Authoritative

Atarix-native state is authoritative when POSIX state and native state disagree.

Examples:

```text
If a descriptor table says a descriptor exists but the backing capability is revoked, the descriptor is unusable.
If a path exists but policy denies access, the operation is denied.
If a compatibility process exists but its native object is quarantined, the process cannot continue normal execution.
If a compatibility namespace view is stale, native directory state governs reconciliation.
```

Compatibility metadata may help explain intent.

It does not override native authority, policy, audit, lifecycle, or recovery state.

## Identity Labels

POSIX user and group concepts are compatibility identity labels.

They are not native authority.

A compatibility user or group label may map to:

```text
Atarix identity label
Policy subject
Compatibility account object
Audit actor label
Namespace view selector
Resource accounting group
```

A label may help policy decide whether an action should be allowed.

The label itself does not grant native authority.

Native capability and policy checks remain required for authority-bearing operations.

## Permission Metadata

POSIX permission bits are compatibility metadata and policy inputs.

They may describe intended access behavior inside the compatibility environment.

They do not replace native Atarix capabilities or policy.

Permission metadata may include:

```text
Read bit
Write bit
Execute bit
Owner category
Group category
Other category
Creation mask
Special compatibility flags
```

The compatibility layer may use this metadata to decide how to present behavior to POSIX-facing software.

The final native decision still depends on Atarix authority, policy, object state, lifecycle state, and recovery state.

## Paths

POSIX paths are compatibility namespace references.

A path may be absolute, relative, symbolic, generated, mounted, or view-specific.

Path resolution maps to Atarix namespace and directory operations.

Core rule:

```text
Path lookup is not access.
```

A successful path lookup may identify an object.

It does not grant permission to read, write, execute, enumerate, modify, delete, signal, mount, or control that object.

A path lookup result should preserve:

```text
Resolved object identity
Directory binding identity
Generation
Namespace view
Lookup policy result
Audit context where required
```

If namespace state and compatibility state disagree, native directory and object state govern reconciliation.

## Descriptors

A POSIX descriptor is a compatibility handle.

Internally it maps to Atarix-native capabilities, service bindings, object references, and resource allocations.

A descriptor may represent:

```text
Storage object access
Directory enumeration state
Stream endpoint
Pipe endpoint
Socket endpoint
Event object
Device-like service endpoint
Compatibility runtime object
```

A descriptor record should include:

```text
Descriptor number
Descriptor generation
Backing object identity
Backing service identity
Capability reference
Open mode
Inheritance policy
Lifecycle state
Audit policy
Resource accounting reference
```

Descriptor possession alone is not native authority.

A descriptor is usable only while its backing native authority remains valid and policy permits the operation.

## Descriptor Duplication And Inheritance

Descriptor duplication, passing, and inheritance are authority-sensitive compatibility operations.

They must be policy-controlled.

The compatibility layer must distinguish:

```text
Duplicate within same environment
Inherit into created execution object
Pass through IPC-like mechanism
Close or revoke descriptor
Mark descriptor non-inheritable
```

A duplicated descriptor must not silently widen authority.

A descriptor inherited by a new compatibility execution object must carry only the authority allowed by policy.

If backing authority is revoked, stale descriptors must become unusable or enter an explicit error state.

## Processes

A POSIX process is a compatibility execution object.

It has both a POSIX-facing identity and a native Atarix object identity.

A process record should include:

```text
Compatibility process identifier
Native object identity
Parent relationship where applicable
Compatibility environment identity
Identity labels
Capability set
Descriptor table reference
Namespace view
Resource allocation
Lifecycle state
Audit context
Recovery state
```

A process identifier is not authority.

Process existence does not imply a right to observe, interrupt, inspect, debug, or control the process.

Those operations remain native authority-bearing operations mediated by capability and policy.

## Threads

POSIX threads are compatibility execution contexts within a process object.

A thread record should include:

```text
Compatibility thread identifier
Native execution context identity
Owning process identity
Resource accounting reference
Scheduling policy reference
Lifecycle state
Audit context where required
```

Thread identity is not authority.

Thread creation, termination, suspension, and scheduling changes are resource- and policy-controlled operations.

## Process Creation

Traditional POSIX process creation must be represented carefully in a capability system.

Atarix may support multiple implementation strategies:

```text
Full compatibility clone
Copy-on-write compatibility clone
Restricted clone
Spawn-based emulation
Denied operation
```

The selected strategy must be declared by the POSIX compatibility profile.

Process creation must not silently duplicate authority beyond policy.

Inherited descriptors, namespace views, identity labels, resource limits, and capabilities must be explicitly copied, filtered, or denied according to policy.

## Program Loading And Replacement

Program loading maps a compatibility executable request to native Atarix object, storage, loader, and execution services.

Program replacement changes the executable image of an existing compatibility process.

Program loading or replacement must validate:

```text
Executable object identity
Executable version or format
Loader or interpreter identity
Execute authority
Policy approval
Descriptor inheritance policy
Environment metadata policy
Resource limits
Audit requirements
```

Program replacement must not preserve authority that policy requires to be dropped.

If loading cannot be verified, the operation must fail explicitly or enter a compatibility error state.

## Process Lifecycle

Suggested process lifecycle states:

```text
CREATED
LOADING
ACTIVE
WAITING
SUSPENDED
EXITING
TERMINATED
DEGRADED
QUARANTINED
FAILED
UNKNOWN
```

Compatibility lifecycle state must reconcile with the native object lifecycle state.

Unknown lifecycle state must be explicit.

## Mailbox-Based IPC Mapping

Atarix uses mailboxes as the native IPC abstraction.

Compatibility communication mechanisms are projections over mailboxes, services, objects, or future memory and data movement facilities.

Core rule:

```text
Native IPC is mailbox-based.
Compatibility IPC is a projection.
```

This keeps compatibility communication aligned with one native authority, audit, lifecycle, recovery, and resource-accounting model.

All compatibility IPC objects shall conform to mailbox lifecycle rules defined by ATX-SPEC-005.

## Compatibility Events

POSIX-facing event delivery should map to mailbox-delivered events associated with a compatibility process, process group, or environment.

An event delivery record should include:

```text
Event identity
Sending compatibility identity
Target compatibility identity
Native actor identity
Target native object identity
Delivery policy result
Lifecycle state
Audit policy
```

Event delivery is not native control authority.

Event delivery requires native authority and policy approval where it affects another object.

## Compatibility Streams

POSIX-facing streams should map to mailbox-backed stream objects.

A stream endpoint should include:

```text
Read endpoint descriptor
Write endpoint descriptor
Backing mailbox identity
Buffer ownership policy
Flow-control policy
Resource accounting
Lifecycle state
Audit policy where required
```

Stream endpoints require capabilities.

Closing one endpoint must update lifecycle state and notify affected consumers according to mailbox semantics.

## Compatibility Queues

POSIX-style queues and notifications should map to mailbox queues or mailbox-delivered event objects.

Compatibility queues must preserve native rules for:

```text
Capability validation
Policy checks
Resource accounting
Backpressure
Audit
Recovery
```

A compatibility queue is not a separate native IPC subsystem unless later architecture explicitly creates one.

## Network Endpoint Projection

POSIX-facing network descriptors are compatibility network endpoints.

They map to Atarix Network Service bindings and mailbox-backed descriptors.

A network descriptor should include:

```text
Network family projection
Network type projection
Protocol projection
Network service identity
Endpoint capability
Mailbox or buffer identity
Resource accounting
Lifecycle state
Audit policy
```

A network descriptor is not direct network authority.

Network operations require service authority, endpoint capability, policy approval, and resource availability.

## Local Endpoint Projection

POSIX local communication endpoints should map to mailbox-backed local service endpoints.

Path-like local endpoint names are compatibility namespace entries.

Path lookup remains not access.

## Service-Bound Compatibility Endpoints

Compatibility endpoint names that look like device entries are bindings to native services.

Examples:

```text
Null-like endpoint -> Null Service
Entropy-like endpoint -> Entropy Service
Terminal-like endpoint -> Terminal Service
Storage-like endpoint -> Storage Service
Network-like endpoint -> Network Service
```

These bindings must be service-mediated, capability-controlled, policy-controlled, auditable where required, and recoverable.

Compatibility endpoint names are service discovery artifacts. They are not service authority.

Observation is not control.

## Shared Memory Projection

POSIX shared memory and memory mapping require special handling.

The native memory and data movement rules belong in ATX-SPEC-021.

This specification treats shared memory as a compatibility projection that must later map to Atarix memory ownership, lifecycle, audit, recovery, and data movement rules.

Shared memory must not become a way to bypass capability checks, policy checks, or audit boundaries.

## Mapping Rules

The following rules apply:

```text
Path lookup is not access.
Descriptor possession is not native authority.
Process identity is not native authority.
Compatibility roles are not native omnipotence.
Signals are compatibility events, not native control.
Device-like endpoints must be service-mediated.
Recovery does not restore compatibility authority without reconciliation.
Native IPC is mailbox-based.
```

## Profiles

Atarix may define compatibility profiles.

Examples:

```text
POSIX_MINIMAL
POSIX_USERLAND
POSIX_NETWORKED
POSIX_DEVELOPMENT
POSIX_LEGACY
```

Each profile defines which compatibility features are supported, restricted, emulated, or denied.

## Audit Requirements

Authority-bearing compatibility operations must be auditable.

Audit should record both:

```text
Compatibility view
Native Atarix authority decision
```

Compatibility audit records shall participate in the native audit chain.

The audit record should preserve native decision context even when the compatibility layer returns a simplified result to the application.

Recommended audit fields include:

```text
Compatibility environment identity
Compatibility process identity
Native object identity
Native service identity where applicable
Descriptor identity where applicable
Operation name
Requested compatibility mode
Native authority result
Policy result
Lifecycle state
Recovery state where relevant
Error mapping result where relevant
```

Compatibility audit should be especially careful around:

```text
Program loading
Descriptor inheritance
Path resolution
Network endpoint creation
Service-bound endpoint access
Policy denial
Recovery reconciliation
```

## Error Mapping

Compatibility errors are projections of native Atarix errors.

The native error remains the authoritative error.

A compatibility error mapping should preserve:

```text
Native error code
Compatibility error code
Object or service identity
Operation
Policy result if applicable
Lifecycle state if applicable
Audit event identity where applicable
```

Lossy error conversion must not erase native error context.

Examples:

```text
Policy denied -> compatibility access denial
Object not found -> compatibility not-found result
Resource exhausted -> compatibility resource failure
Unsupported version -> compatibility unsupported operation
Quarantined object -> compatibility unavailable or restricted result
Unknown state -> compatibility explicit failure
```

Compatibility error mapping should be table-driven and versioned.

Unknown native errors must not be silently converted to success.

Compatibility layers shall preserve native degraded-state semantics. A degraded native state must remain visible through compatibility error context rather than being flattened into an ordinary generic failure.

## Recovery And Reconciliation

Compatibility recovery must reconcile POSIX-facing state against native Atarix state before normal execution resumes.

Core rule:

```text
Compatibility recovery restores execution.
Compatibility recovery does not restore authority.
```

Compatibility recovery is a subordinate recovery domain of native Atarix recovery.

Recovery trusts native Atarix state, not reconstructed compatibility metadata.

Compatibility metadata may be used as evidence during recovery, but it must not grant or restore authority by itself.

## Recovery Phases

Suggested compatibility recovery phases:

```text
DETECT
FREEZE
INSPECT
RECONCILE
RESTORE
VERIFY
RESUME
```

A compatibility environment may not resume normal execution until required reconciliation succeeds or policy explicitly allows degraded operation.

## Descriptor Reconciliation

Descriptor reconciliation validates each compatibility descriptor against native backing state.

For each descriptor, recovery should verify:

```text
Descriptor record exists
Descriptor generation matches expected state
Backing capability exists
Backing capability is valid
Backing object or service exists
Backing object lifecycle permits use
Open mode remains policy-valid
Audit state is preserved where required
```

If reconciliation fails, the descriptor must become invalid, restricted, or quarantined.

Recovery must not silently recreate descriptor authority.

## Path Reconciliation

Path reconciliation validates compatibility namespace state against the native namespace, directory, and object model.

For each path binding, recovery should verify:

```text
Compatibility path record exists
Namespace view is still valid
Directory binding exists
Target object identity matches expected generation where required
Policy permits continued visibility
Target lifecycle permits use
```

If the path cannot be resolved safely, the path becomes unresolved or restricted.

Path recovery must not grant access.

## Process Reconciliation

Process reconciliation validates compatibility process state against native object lifecycle state.

For each process, recovery should verify:

```text
Compatibility process record exists
Native object identity exists
Native lifecycle state permits continuation
Descriptor table has reconciled
Namespace view has reconciled
Resource allocation is valid
Policy permits resumed execution
Audit context is continuous or explicitly degraded
```

A process must not resume merely because a compatibility process identifier exists.

## Network Reconciliation

Network endpoint reconciliation validates compatibility network descriptors against the Network Service.

Recovery should verify:

```text
Network descriptor exists
Endpoint capability remains valid
Network service identity is available
Connection or endpoint lifecycle is known
Resource accounting is valid
Policy permits continued use
```

Broken or ambiguous network endpoints should fail cleanly and require reconnect or explicit recovery policy.

## Audit Continuity

Compatibility recovery must verify audit continuity where required.

Recovery should check:

```text
Last known audit event
Expected next event relationship
Environment audit identity
Process audit identity
Recovery event record
Integrity status where applicable
```

If audit continuity cannot be established, policy decides whether the environment becomes degraded, quarantined, or stopped.

Audit uncertainty must be explicit.

## Quarantine Rules

A compatibility environment should enter quarantine when recovery discovers:

```text
Authority mismatch
Descriptor mismatch
Object identity mismatch
Namespace corruption
Audit discontinuity
Unknown recovery state
Unsupported compatibility version
Untrusted service binding
```

Quarantine blocks normal execution.

Policy may allow limited inspection, export, or repair operations.

Quarantine must not become a path to broader authority.

## Recovery Outcomes

Compatibility recovery may produce:

```text
RECOVERED
RECOVERED_DEGRADED
QUARANTINED
FAILED
MANUAL_REVIEW_REQUIRED
UNKNOWN
```

Unknown recovery outcome must fail closed for authority-bearing operations.

## Compatibility Profile Registry

Compatibility profiles define supported, restricted, emulated, and denied behavior for a POSIX environment.

A profile should specify:

```text
Profile name
Profile version
Supported API families
Descriptor behavior
Process creation strategy
Program loading strategy
IPC mappings
Network support
Service-bound endpoint support
Shared memory policy
Audit policy
Recovery policy
Unsupported operations
```

Profiles are versioned architecture objects.

Unknown profile versions must be explicit and must not silently fall back to broader compatibility.

## POSIX_MINIMAL

`POSIX_MINIMAL` is the smallest useful compatibility environment.

Expected support:

```text
Basic path lookup
Basic descriptors
Basic file-like read/write/close
Basic program loading
Basic environment metadata
Basic error projection
Basic audit hooks
```

Expected restrictions:

```text
No full process cloning requirement
No broad network requirement
No broad device endpoint requirement
No shared memory requirement
No legacy privilege expansion
```

This profile is suitable for early runtime tests, simple utilities, and compatibility scaffolding.

## POSIX_USERLAND

`POSIX_USERLAND` targets ordinary command-line and userland-style software.

Expected support:

```text
Directories
Regular file views
Descriptor inheritance policy
Process creation strategy
Program loading and replacement
Mailbox-backed streams
Compatibility events
Environment variables
Working directory state
```

Expected restrictions:

```text
Network optional
Service-bound endpoints limited
Shared memory profile-gated
Compatibility administrative role limited by native policy
```

## POSIX_NETWORKED

`POSIX_NETWORKED` extends userland compatibility with network endpoint projection.

Expected support:

```text
Network descriptors
Network service binding
Local endpoint projection
Mailbox-backed network buffers
Network audit events
Network recovery checks
```

Expected restrictions:

```text
No direct network authority from descriptor possession
Endpoint capability required
Policy required for bind/connect/listen-like operations
Recovery may require reconnect
```

## POSIX_DEVELOPMENT

`POSIX_DEVELOPMENT` supports build tools, compilers, linkers, shells, interpreters, package tools, and diagnostic utilities.

Expected support:

```text
Large descriptor tables
Process tree metadata
Program loading diagnostics
Filesystem-like workspace views
Expanded audit diagnostics
Development-oriented error detail
```

Expected restrictions:

```text
Debug-style observation requires explicit policy
Build tool authority remains scoped
Compatibility tooling cannot bypass native policy
```

## POSIX_LEGACY

`POSIX_LEGACY` is for software that expects older or broader Unix behavior.

This profile is higher risk and should be opt-in.

Expected support may include broader compatibility emulation.

Required controls:

```text
Explicit policy approval
Stronger audit
Tighter resource limits
Clear unsupported-operation list
Recovery quarantine preference
No native authority expansion
```

Legacy compatibility must not become a reason to weaken native Atarix semantics.

## Compatibility Test Matrix

ATX-SPEC-015 requires tests that prove the compatibility projection does not weaken native authority.

Initial test categories:

```text
Identity mapping
Permission metadata
Path lookup
Descriptor handling
Descriptor inheritance
Program loading
Process lifecycle
Mailbox IPC mapping
Network endpoint projection
Service-bound endpoint projection
Audit mapping
Error mapping
Recovery reconciliation
Quarantine behavior
Profile enforcement
Unsupported operation handling
```

## Required Identity Tests

Tests should verify:

```text
User label does not grant native authority
Group label does not grant native authority
Compatibility role does not bypass native policy
Audit records include compatibility and native identities
```

## Required Path Tests

Tests should verify:

```text
Path lookup identifies objects without granting access
Stale path binding reconciles against native directory state
Denied path access produces native policy result plus compatibility error
Namespace view cannot escape profile policy
```

## Required Descriptor Tests

Tests should verify:

```text
Descriptor maps to backing capability
Descriptor cannot outlive revoked capability
Descriptor duplication obeys policy
Descriptor inheritance obeys policy
Invalid descriptor produces explicit compatibility error
```

## Required Process And Program Tests

Tests should verify:

```text
Process identifier is not authority
Program loading requires execute authority
Program replacement drops denied authority
Process creation strategy matches profile
Quarantined process cannot resume normal execution
```

## Required IPC And Network Tests

Tests should verify:

```text
Compatibility events map to mailbox events
Streams map to mailbox-backed endpoints
Queues preserve mailbox resource limits
Network descriptors require endpoint capability
Local endpoint path lookup does not grant access
Service-bound endpoints are service-mediated
Shared memory remains governed by ATX-SPEC-021
```

## Required Audit And Error Tests

Tests should verify:

```text
Audit records preserve native decision context
Compatibility error mapping preserves native error context
Unknown native error never maps to success
Policy denial is auditable
Recovery actions are auditable
Degraded native state remains visible through compatibility error context
```

## Required Recovery Tests

Tests should verify:

```text
Recovery does not restore authority
Compatibility recovery remains subordinate to native recovery
Descriptor reconciliation fails closed
Path reconciliation does not grant access
Process reconciliation requires native lifecycle validity
Network reconciliation detects broken endpoints
Audit discontinuity becomes explicit
Quarantine blocks normal execution
Unknown recovery outcome fails closed
```

## Initial POSIX Sprint Scope

POSIX Sprint 1 should define:

```text
Compatibility environment metadata
Compatibility profile enum
Identity label mapping
Path lookup mapping
Descriptor table model
Basic open/read/write/close mapping
Basic program replacement mapping
Basic error projection
Audit hooks
Basic tests
```

## Required Future Work

- Define POSIX profile registry object format.
- Define descriptor record format.
- Define compatibility error mapping table.
- Define compatibility audit event additions.
- Define descriptor inheritance policy.
- Define process creation compatibility strategy.
- Define socket compatibility strategy.
- Define minimal libc target.
- Define FOSS application porting guide.
- Define compatibility IPC mappings against ATX-SPEC-005.
- Define shared memory mapping against ATX-SPEC-021.
- Define compatibility recovery test cases against ATX-SPEC-018.
- Define profile conformance test suite.

## Summary

POSIX compatibility is a boundary layer for running existing software.

It is not native Atarix authority.

Its central rules are:

```text
POSIX is a compatibility personality.
Path lookup is not access.
Descriptors are compatibility handles.
Native IPC is mailbox-based.
Compatibility recovery restores execution, not authority.
Compatibility profiles are explicit and versioned.
POSIX compatibility must not weaken native Atarix security semantics.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-019-Service-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-019-Service-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-019-Service-Model.md`

# ATX-SPEC-019 Service Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Service Model.

The Service Model describes how Atarix represents, discovers, authorizes, versions, audits, migrates, replaces, and retires services.

A service is a capability-mediated provider of functionality.

A service is not defined by where it runs.

## Core Rule

```text
Service location is not service identity.
```

A service may be provided by a CPU card, auxiliary compute card, storage card, network card, FPGA fabric service, supervisor-facing bridge, compatibility environment, or future provider.

Consumers should depend on the service identity, interface, version, policy, and capabilities, not the physical provider.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-016 Supervisor Management Fabric

This specification informs:

- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

## Design Philosophy

Atarix is a service-oriented fabric.

The primary CPU card is not required to implement every system capability locally.

Services may be offloaded to dedicated providers.

Examples include:

```text
Directory service
Audit service
Storage service
Network service
Hash service
Compression service
Time service
Netboot service
Recovery service
Compatibility service
```

This allows a W65C816 CPU card to participate in a system that provides larger memory, stronger storage, cryptographic hashing, compression, network boot, audit persistence, and future acceleration through explicit service interfaces.

## Service As Object

A service is an object or object-owned provider interface.

A service has:

```text
Service identity
Service name or names
Provider identity
Interface version
Lifecycle state
Authority requirements
Resource requirements
Policy scope
Audit policy
Error behavior
```

Service identity is independent of provider location.

## Service Namespaces

Recommended service names live under:

```text
/service
```

Examples:

```text
/service/directory
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/time
/service/netboot
/service/recovery
/service/compatibility
```

Directory lookup of a service does not grant authority to use it.

Lookup is not access.

## Service Providers

A service provider may be:

```text
CPU card
Auxiliary compute card
Raspberry Pi-class compute/buffer card
RISC-V service card
GPU or accelerator card
Storage processor card
Network card
FPGA fabric service
Supervisor audit bridge
Compatibility VM
POSIX personality
```

The provider is an implementation location, not the service identity.

## Service Registration

Service registration publishes the existence of a service.

A registration should include:

```text
Service identity
Service name
Provider identity
Interface version
Supported operations
Required capabilities
Resource class requirements
Policy reference
Lifecycle state
Health state
Generation
Audit policy
```

Registration requires authority.

A compromised provider must not be able to register arbitrary trusted services without policy approval.

## Service Discovery

Service discovery returns service metadata, not authority.

A discovery result may include:

```text
Service identity
Service type
Provider identity
Interface version
Available operations
Health state
Policy reference
Binding generation
```

Discovery does not return operational authority.

## Service Capabilities

Using a service requires explicit capability and policy approval.

Service authority may include:

```text
DISCOVER
OBSERVE
CALL
CONFIGURE
ALLOCATE
DELEGATE
REVOKE
MIGRATE
SUSPEND
RETIRE
```

Possession of a service name or provider identity does not imply any of these authorities.

## Service Calls

Service calls are normally transported through mailboxes or fabric message mechanisms.

Mailbox connectivity does not imply service authority.

Every authority-bearing service call must validate:

```text
Caller identity label
Target service identity
Operation
Interface version
Capability
Policy
Lifecycle state
Resource availability
```

## Service Versioning

Every service interface is versioned.

Service versioning follows ATX-SPEC-012.

A service advertises:

```text
Service interface major version
Service interface minor version
Service interface patch version
Feature set
Compatibility policy
```

Unknown major versions fail closed for authority-bearing operations.

Compatibility fallback must be explicit and auditable.

## Service Generation

Service generation is not service version.

Generation describes the evolution of a specific service registration or instance.

Version describes interface compatibility.

## Service Lifecycle

Suggested service lifecycle states:

```text
REGISTERED
INITIALIZING
ACTIVE
DEGRADED
SUSPENDED
MIGRATING
REVOKING
RETIRED
QUARANTINED
FAILED
UNKNOWN
```

Service lifecycle transitions must be auditable.

## Service Health

Services should publish health state where policy permits.

Health state may include:

```text
Available
Degraded
Busy
Resource constrained
Recovering
Quarantined
Failed
Unknown
```

Observation of health does not grant control authority.

## Service Migration

A service may migrate from one provider to another.

Migration must preserve:

```text
Service identity
Authority constraints
Policy constraints
Audit continuity
Version compatibility
Resource accounting
Lifecycle visibility
```

Migration must be auditable.

Migration must not silently create broader authority.

## Service Replacement

A service provider may be replaced.

Replacement must distinguish:

```text
Same service identity, new provider
New service identity, compatible interface
Compatibility adapter
Recovery replacement
```

Clients requiring stability should bind to service identity and compatible version, not physical provider.

## Service Retirement

Services are not immortal.

Retirement requires:

```text
Policy approval
Lifecycle transition
Capability revocation or migration
Resource cleanup
Directory update
Audit record
Compatibility decision where applicable
```

Retired services must not silently continue operating through stale bindings.

## Service Quarantine

A service may be quarantined when:

```text
Provider is compromised
Version is unsupported
Policy cannot be evaluated
Audit continuity is broken
Resource ownership is uncertain
Behavior is unsafe
Recovery requires isolation
```

Quarantine preserves evidence and prevents unsafe authority use.

## Service And Resources

Services consume and may expose resources.

Resource use must be explicit, accounted, policy-controlled, and auditable.

Examples:

```text
Audit service consumes reserved audit buffers.
Storage service consumes persistent storage.
Hash service consumes auxiliary compute cycles.
Network service consumes packet buffers and bandwidth.
Compression service consumes memory and compute time.
```

## Auxiliary Compute Services

Auxiliary compute cards may provide services such as:

```text
Hashing
Compression
Encryption support
Audit staging
Netboot cache
Filesystem service
Storage scrub assistance
Snapshot export
Replication support
```

Auxiliary compute resources are not automatically trusted.

They require service registration, version advertisement, capability mediation, policy approval, audit visibility, and cleanup.

## Supervisor-Facing Services

Supervisor-facing service exports are restricted.

Operational services may observe supervisor events through authorized audit or observation services.

Operational services may request supervisor actions only through mediated supervisor request paths.

A supervisor-facing service must not become direct supervisor control authority.

Observation is not control.

## Compatibility Services

Compatibility services may expose legacy interfaces, POSIX personalities, protocol translators, or virtual hardware.

Compatibility services are objects.

They require explicit capabilities, policy control, resource accounting, audit, lifecycle, and cleanup.

Compatibility must not weaken native Atarix authority semantics.

## Service And Policy

Policy governs:

```text
Who may register a service
Who may discover a service
Who may call a service
Who may configure a service
Who may migrate a service
Who may retire a service
Which versions are allowed
Which providers are trusted
Which compatibility modes are allowed
```

Policy failure must fail closed for authority-bearing service operations.

## Service And Audit

Service events that should be auditable include:

```text
Service registered
Service discovered where policy requires
Service call allowed
Service call denied
Service version negotiated
Service degraded
Service migrated
Service replaced
Service retired
Service quarantined
Service provider failed
Compatibility service activated
```

## Service And Errors

The Error Model must support service errors such as:

```text
SERVICE_NOT_FOUND
SERVICE_UNAVAILABLE
SERVICE_VERSION_UNSUPPORTED
SERVICE_POLICY_DENIED
SERVICE_PROVIDER_FAILED
SERVICE_QUARANTINED
SERVICE_MIGRATION_FAILED
SERVICE_UNKNOWN_STATE
```

Unknown service state must be explicit.

## Bootstrap Services

Bootstrap services are not runtime services.

Netboot, early time acquisition, firmware validation, and recovery image generation belong to the Bootstrap Security Model until runtime handoff.

Bootstrap authority is not runtime authority.

## Initial Service Sprint Scope

Service Sprint 1 should define:

```text
Service identity concept
Service registration metadata
Service version advertisement
Service lifecycle states
Basic service discovery metadata
Capability checks for service calls
Basic tests
```

Service Sprint 1 should not implement:

```text
Service migration
Distributed service federation
Complex service failover
POSIX compatibility
Remote service registry federation
General service scripting
```

## Required Tests

Initial tests should verify:

```text
Service registration requires authority
Service discovery does not grant authority
Service call requires capability
Unsupported service version fails closed
Service lifecycle transition is audited
Service quarantine denies calls
Provider identity is not service identity
Compatibility service requires policy
Auxiliary compute service is not automatically trusted
```

## Required Future Work

- Define service identity wire format.
- Define service registration record format.
- Define service discovery C API.
- Define service call metadata.
- Define service migration semantics.
- Define service retirement semantics.
- Define compatibility service semantics in ATX-SPEC-015.
- Define storage-backed service persistence in ATX-SPEC-017.
- Define service recovery and reconciliation in ATX-SPEC-018.

## Summary

The Service Model defines Atarix as a capability-secured service-oriented fabric.

Its central rules are:

```text
Service location is not service identity.
Discovery is not authority.
A provider is not automatically trusted.
Compatibility belongs at boundaries.
Unknown service state must be explicit.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-021-Memory-and-Data-Movement-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-021-Memory-and-Data-Movement-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-021-Memory-and-Data-Movement-Model.md`

# ATX-SPEC-021 Memory and Data Movement Model

## Status

Draft v0.2

## Purpose

This document defines the Atarix Memory and Data Movement Model.

Memory movement is a system resource.

ATX-SPEC-021 defines how Atarix moves data between CPUs, services, mailboxes, accelerators, storage, and fabric-attached devices without weakening authority, policy, audit, recovery, human readability, or compatibility semantics.

This specification covers:

- Memory ownership
- Data movement authority
- Copy, move, map, and share semantics
- Fabric transfer rules
- Zero-copy constraints
- Mailbox payload movement
- Lookup-accelerator memory access
- POSIX compatibility memory behavior
- Audit-window reconstruction for data movement
- Recovery-safe memory transfer behavior

## Core Rule

Data movement is not authority.

A component may copy, move, map, share, cache, stage, or accelerate access to data only after the relevant Atarix authority checks have succeeded.

Reachability is not permission.

No bus master, fabric node, lookup accelerator, cache, descriptor, or compatibility layer may treat physical addressability as authority.

## Human-Readable Abstraction Rule

Atarix must preserve human-readable architecture even when data movement is optimized.

Every optimized data movement path must provide an explainable projection.

Examples:

```text
copy operation      -> source, destination, length, authority, generation
shared mapping      -> owner, borrower, lifetime, revocation rule
fabric transfer     -> service route, mailbox sequence, audit event
cache promotion     -> original object, generation, validation rule
```

No memory optimization may become opaque authority.

## Audit Window Rule

Privileged or cross-domain data movement must be reconstructable inside a bounded audit window.

An audit window should answer:

- Who requested the movement?
- What object, buffer, page, record, or mailbox was involved?
- Which authority and policy checks applied?
- What source and destination domains participated?
- What generation, version, or lifecycle window applied?
- Was the operation completed, denied, aborted, retried, reconciled, or quarantined?

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model
- ATX-SPEC-020 Hash Table and Lookup Acceleration Model

This specification informs:

- Future fabric controller RTL
- Persistent storage pipelines
- POSIX compatibility memory behavior
- Lookup accelerator memory access
- Service-to-service payload routing
- Zero-copy data-plane design

## Design Goals

Atarix memory and data movement should provide:

- Explicit ownership
- Explicit authority checks
- Bounded transfer semantics
- Auditable movement
- Recovery-safe completion rules
- Low copy overhead where safe
- Clear separation between control-plane and data-plane behavior
- Compatibility with mailbox routing
- Compatibility with POSIX expectations
- Hardware-friendly bus and fabric behavior

Performance is valuable only when it preserves architecture semantics.

## Initial Movement Forms

Atarix initially defines these movement forms.

### Copy

A copy creates a distinct destination representation while preserving the source.

Copies must preserve provenance metadata when crossing authority, policy, audit, or recovery boundaries.

### Move

A move transfers ownership or active responsibility from one domain to another.

Moves must have explicit lifecycle and recovery semantics.

### Map

A map creates an addressable view into data owned by another object, service, or memory domain.

Mappings must be revocable unless explicitly declared permanent by native object state and policy.

### Share

A share allows multiple authorized parties to access the same data under defined rules.

Sharing requires explicit authority, policy, lifetime, and recovery behavior.

### Fabric Transfer

A fabric transfer moves data without ordinary CPU copy loops.

A fabric engine is never authority.

Fabric movement must operate through explicit descriptors validated by capability, policy, lifecycle, generation, and recovery rules.

### Zero-Copy Path

Zero-copy is an optimization, not a permission model.

A zero-copy path may avoid data copying only when ownership, lifetime, authority, policy, audit, and recovery requirements remain explicit.

## Ownership Rule

Every data buffer must have an owner.

Ownership may belong to an object, service, mailbox domain, storage domain, compatibility personality, or fabric endpoint.

Borrowed access must identify:

- Owner
- Borrower
- Access mode
- Lifetime
- Generation
- Revocation rule
- Recovery outcome

## Descriptor Rule

External memory movement must be described by explicit descriptors.

A movement descriptor is not authority.

A descriptor may describe:

- Source identity
- Destination identity
- Source address or object reference
- Destination address or object reference
- Length
- Movement form
- Required access mode
- Expected generation
- Completion rule
- Audit projection rule
- Recovery rule

A descriptor must be validated before execution.

## Canonical Movement Descriptor v1

The initial public descriptor is `atarix_memory_descriptor_v1_t`.

It is intended to be small enough for mailbox-mediated setup and explicit enough for audit, validation, and future fabric execution.

Required fields:

```text
magic              descriptor format marker
version            descriptor version
flags              movement form and validation flags
source_object      source object, mailbox, buffer, or service identifier
source_offset      source-local byte offset
destination_object destination object, mailbox, buffer, or service identifier
destination_offset destination-local byte offset
length             requested byte length
capability_id      explicit capability authorizing the movement
generation         expected source or transfer generation
audit_hint         human-readable projection selector
```

Descriptor validation MUST reject:

- Null descriptors
- Invalid magic
- Unsupported version
- Zero-length transfers
- Missing capability identifiers
- Missing source identifiers
- Missing destination identifiers
- Invalid or reserved movement forms
- Lengths exceeding implementation limits

## Initial Movement Flags

Initial movement forms are encoded as flags:

```text
ATARIX_MEMORY_MOVE_COPY
ATARIX_MEMORY_MOVE_MOVE
ATARIX_MEMORY_MOVE_MAP
ATARIX_MEMORY_MOVE_SHARE
ATARIX_MEMORY_MOVE_FABRIC
ATARIX_MEMORY_MOVE_ZERO_COPY
```

Exactly one movement form should be selected for the initial validation profile.

## Initial Validation Statuses

Initial validation statuses:

```text
ATARIX_MEMORY_STATUS_OK
ATARIX_MEMORY_STATUS_NULL_DESCRIPTOR
ATARIX_MEMORY_STATUS_BAD_MAGIC
ATARIX_MEMORY_STATUS_BAD_VERSION
ATARIX_MEMORY_STATUS_ZERO_LENGTH
ATARIX_MEMORY_STATUS_MISSING_CAPABILITY
ATARIX_MEMORY_STATUS_MISSING_SOURCE
ATARIX_MEMORY_STATUS_MISSING_DESTINATION
ATARIX_MEMORY_STATUS_BAD_FLAGS
ATARIX_MEMORY_STATUS_LENGTH_EXCEEDED
```

## Mailbox Interaction

Mailbox messages may carry small payloads directly or may reference external buffers, descriptors, or data windows.

External references must be:

- Capability-checked
- Policy-checked
- Generation-aware
- Bounded by length
- Auditable
- Recovery-safe
- Non-authoritative by themselves

Mailbox control flow remains separate from bulk data movement.

## Lookup Accelerator Interaction

ATX-SPEC-020 lookup accelerators may fetch, inspect, or compare memory only through ATX-SPEC-021-compliant movement rules.

Lookup accelerators must not infer authority from reachable memory.

Accelerated reads must preserve:

- Registry identity
- Table generation
- Probe bounds
- Capability result
- Policy result
- Audit projection
- Recovery status

## POSIX Compatibility Interaction

POSIX compatibility memory behavior is a personality mapping over native Atarix memory movement.

POSIX read, write, mmap, pipe, socket, and descriptor behavior must not bypass native ownership, authority, audit, or recovery rules.

## Recovery Rule

Every cross-domain data movement operation must have a defined recovery outcome.

Valid outcomes include:

- Completed
- Denied
- Aborted before visibility
- Reconciled
- Retried under a new generation
- Quarantined

Unrecoverable ambiguous movement is forbidden.

## Initial Implementation Plan

The initial implementation should add:

- `include/atarix/memory.h`
- `src/memory.c`
- `tests/memory/test_memory_descriptor.c`
- Automake integration for the memory descriptor test
- AEMS traceability metadata
- Engineering Gate coverage through `make check`

## Initial Test Plan

The initial unit test must validate:

- A valid copy descriptor succeeds.
- Null descriptor fails.
- Bad magic fails.
- Bad version fails.
- Zero length fails.
- Missing capability fails.
- Missing source fails.
- Missing destination fails.
- Unknown flags fail.
- Excessive length fails.

## Open Questions

- Which transfers require pre-commit audit rather than post-commit audit?
- How should zero-copy lifetimes be represented in the object model?
- How should POSIX mmap semantics map onto native Atarix movement forms?
- What minimum hardware signals are required for an ECP5 transfer descriptor gate?

## TODO

- Implement canonical movement descriptor layout.
- Add descriptor validation helper.
- Add tests and wire them into `make check`.
- Add ATX-100 Chapter 14 once this specification stabilizes.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/architecture-backlog.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/architecture-backlog.md -->
### Integrated source: `docs/architecture/architecture-backlog.md`

# Atarix Architecture Backlog

## Status

Living planning document

## Purpose

This document tracks planned architecture specifications and major architecture follow-up work discovered during specification review.

The backlog prevents future work from being hidden inside implementation tickets or ad hoc discussions.

## Current Foundation

The current architecture foundation includes:

```text
ATX-SPEC-001 Security Model
ATX-SPEC-002 Authority Model
ATX-SPEC-003 Capability Model
ATX-SPEC-004 Lifecycle Model
ATX-SPEC-005 Mailbox Model
ATX-SPEC-006 Object Model
ATX-SPEC-007 Namespace Model
ATX-SPEC-008 Directory Service Model
ATX-SPEC-009 Resource Model
ATX-SPEC-010 Audit Model
ATX-SPEC-011 Error Model
```

Architecture Review 001 accepted the current foundation with required follow-up.

## Planned Specifications

### ATX-SPEC-012 Versioning Model

Priority: High

Purpose:

Define versioning for wire formats, object schemas, capability records, directory bindings, resource records, audit records, error records, firmware, services, storage formats, and mixed-version operation.

Key rule:

```text
Version uncertainty must be explicit.
```

### ATX-SPEC-013 Policy Model

Priority: High

Purpose:

Define policy evaluation, policy objects, policy scope, policy inheritance, policy versioning, administrative policy, site policy, and fail-closed behavior when policy cannot be evaluated.

### ATX-SPEC-014 Bootstrap Security Model

Priority: High

Purpose:

Define root of trust, boot-time authority, secure boot, measured boot, pre-OS networking, time synchronization, recovery mode, bootstrap audit, and handoff into runtime security.

Key rule:

```text
Bootstrap authority is not runtime authority.
```

### ATX-SPEC-015 POSIX Compatibility Model

Priority: Deferred

Purpose:

Define how POSIX/FOSS applications may run on top of Atarix without importing ambient-authority assumptions into the native architecture.

Key rule:

```text
POSIX is a compatibility target, not an architectural foundation.
```

### ATX-SPEC-016 Supervisor Management Fabric

Priority: High

Purpose:

Define the isolated Supervisor Management Fabric, supervisor authority, supervisor resources, supervisor command request path, supervisor audit export, physical/logical isolation, and off-board security requirements.

Key rule:

```text
The Supervisor Fabric is control-isolated but audit-visible.
```

### ATX-SPEC-017 Storage and Persistence Model

Priority: Medium-High

Purpose:

Define persistent object storage, checksummed storage, journaling, copy-on-write storage targets, RAM-backed audit buffers, ZFS-inspired recovery architecture, snapshot semantics, and persistence cleanup.

Key rule:

```text
Persistence must be explicit and recoverable.
```

### ATX-SPEC-018 Recovery and Reconciliation Model

Priority: Medium-High

Purpose:

Define crash recovery phases, quarantine, reconciliation of resources, directory bindings, capabilities, audit continuity, storage state, and post-crash authority restoration rules.

Key rule:

```text
Recovery must not silently regrant authority.
```

### ATX-SPEC-019 Service Model

Priority: Medium

Purpose:

Define services as discoverable, versioned, capability-mediated, location-independent providers of system functionality.

Examples:

```text
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/directory
```

Key rule:

```text
Service location is not service identity.
```

## Cross-Cutting Follow-Up Work

Architecture Review 001 identified these required follow-up areas:

```text
Object identity wire format
Path syntax and limits
Authority Model expansion
Lifecycle state machine
Ownership transfer semantics
Directory binding generation representation
Resource identity format
Audit record wire format
Error code namespace
Quarantine transition semantics
Supervisor Audit Bridge semantics
Bootstrap network security
Storage recovery semantics
```

## Architectural Axioms

Current recurring Atarix axioms:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Execution is not authority.
Ownership is not authority.
Allocation is not ownership.
Persistence is not ownership.
Connectivity is not trust.
Bootstrap authority is not runtime authority.
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```

## Recommended Priority Order

```text
1. ATX-SPEC-012 Versioning Model
2. ATX-SPEC-013 Policy Model
3. ATX-SPEC-014 Bootstrap Security Model
4. ATX-SPEC-016 Supervisor Management Fabric
5. ATX-SPEC-017 Storage and Persistence Model
6. ATX-SPEC-018 Recovery and Reconciliation Model
7. ATX-SPEC-019 Service Model
8. ATX-SPEC-015 POSIX Compatibility Model
```

## Implementation Rule

No implementation sprint should silently define behavior that belongs in a planned architecture specification.

If implementation requires unresolved architecture, update this backlog and write the specification before hardening the behavior into public interfaces.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/architecture-review-001.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/architecture-review-001.md -->
### Integrated source: `docs/reviews/architecture-review-001.md`

# Architecture Review 001

## Status

Draft review v0.1

## Scope

This review covers the current Atarix architecture foundation:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

## Purpose

This review checks whether the current specifications agree on terminology, authority, identity, ownership, lifecycle, observability, cleanup, failure, and recovery.

The goal is to catch contradictions before implementation hardens them into public APIs.

## Architectural Axioms Under Review

The following axioms are currently treated as controlling architectural rules:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Execution is not authority.
Ownership is not authority.
Allocation is not ownership.
Persistence is not ownership.
Connectivity is not trust.
Bootstrap authority is not runtime authority.
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```

## Review Summary

Overall status:

```text
ACCEPTED WITH REQUIRED FOLLOW-UP
```

The current specification set is coherent enough to continue architectural development, but several cross-cutting topics require future specifications or deeper expansion.

## Consistency Findings

### Identity And Names

Status: PASS

The Object, Namespace, and Directory specifications consistently separate object identity from names and bindings.

Confirmed rules:

```text
Names are bindings.
Object identities are opaque.
Lookup does not grant authority.
Directory entries are not capabilities.
```

Required follow-up:

- Define concrete object identity wire format.
- Define path syntax and limits.
- Define whether kernel-visible paths support Unicode or restricted ASCII.

### Authority

Status: PASS WITH FOLLOW-UP

The Security, Authority, Capability, Directory, Resource, and Error specifications consistently reject ambient authority.

Confirmed rules:

```text
Execution does not imply authority.
Identity does not imply authority.
Connectivity does not imply trust.
Lookup does not imply access.
Observation does not imply control.
```

Required follow-up:

- Expand ATX-SPEC-002 Authority Model beyond stub status.
- Define bootstrap authority separately from runtime authority.
- Define supervisor authority separately from operational fabric authority.

### Ownership

Status: PASS WITH FOLLOW-UP

The current specifications consistently treat ownership as accountability rather than unrestricted control.

Confirmed rules:

```text
Ownership is not authority.
Binding ownership and object ownership may differ.
Resource ownership and resource authority may differ.
```

Required follow-up:

- Define ownership transfer semantics.
- Define shared ownership or administrative sponsorship.
- Define owner disappearance behavior more rigorously in Lifecycle Model.

### Lifecycle And Cleanup

Status: PASS WITH FOLLOW-UP

Lifecycle, Resource, Directory, Audit, and Error specifications consistently treat cleanup as a security requirement.

Confirmed rules:

```text
No object is immortal by default.
No resource may silently leak.
Stale bindings are observable.
Cleanup failures are auditable.
Unknown state is explicit.
```

Required follow-up:

- Expand ATX-SPEC-004 Lifecycle Model beyond stub status.
- Define object lifecycle state machine.
- Define lease expiration semantics.
- Define uninstall and owner-disappearance behavior.

### Directory And Lookup

Status: PASS

ATX-SPEC-008 correctly separates discoverability from authority.

Confirmed rules:

```text
Lookup is not access.
Enumeration is not access.
Aliases do not grant authority.
Rebinding requires authority.
```

Required follow-up:

- Define Directory Sprint 1 public C interfaces.
- Define binding ID and generation representation.
- Define directory failure error codes after Error Model matures.

### Resource Management

Status: PASS WITH FOLLOW-UP

ATX-SPEC-009 correctly treats resources as first-class architectural entities with ownership, accounting, leases, quota, cleanup, and audit.

Confirmed rules:

```text
Resource use is authority-bearing.
Resource visibility is not control.
Resource exhaustion must fail safely.
Supervisor resources are control-isolated.
```

Required follow-up:

- Define minimum Rev A resource classes.
- Define resource identity format.
- Define reserved resources for audit, recovery, and supervisor paths.
- Define RAM-card-backed audit buffers once fabric memory services mature.

### Audit

Status: PASS WITH FOLLOW-UP

ATX-SPEC-010 establishes audit as evidence rather than debugging output.

Confirmed rules:

```text
Audit is evidence.
Observation is not control.
Audit suppression must be observable.
Supervisor events are auditable.
Cleanup failures are security events.
```

Required follow-up:

- Define audit record wire format.
- Define audit buffer hierarchy.
- Define RAM-backed audit ingest buffers.
- Define asynchronous audit persistence.
- Define ZFS-inspired or copy-on-write persistent audit storage in future Storage Model.

### Error Handling

Status: PASS WITH FOLLOW-UP

ATX-SPEC-011 correctly treats failure and unknown state as first-class architectural concepts.

Confirmed rules:

```text
Unknown state must be explicit.
Fail closed for authority-bearing operations.
Prefer quarantine over assumption.
Degraded operation must be observable.
Recovery must be auditable.
```

Required follow-up:

- Define concrete error code namespace.
- Define structured error object format.
- Define quarantine state transitions.
- Define recovery workflow semantics.

### Supervisor And Operational Fabric

Status: NEEDS SPECIFICATION

The current documents identify Supervisor Management Fabric and Supervisor Audit Bridge concepts, but a dedicated specification is required.

Confirmed direction:

```text
Operational Fabric contains discovery, control, service, and data planes.
Supervisor Management Fabric is control-isolated.
Operational Fabric may observe supervisor state through audit paths.
Operational Fabric may not directly control supervisor authority.
Observation is not control.
```

Required follow-up:

- Create ATX-SPEC-016 Supervisor Management Fabric.
- Define Supervisor Audit Bridge semantics.
- Define supervisor command request path.
- Define physical vs logical isolation requirements.
- Define cryptographic requirements for off-board supervisor access.

### Bootstrap Security

Status: NEEDS SPECIFICATION

The current Security, Audit, Error, and Resource specifications all reference bootstrap behavior, but bootstrap authority remains undefined.

Confirmed direction:

```text
Bootstrap authority is not runtime authority.
Pre-OS networking is not runtime networking.
Boot audit imports into runtime audit where practical.
Failure before Supervisor enters recovery-only mode.
```

Required follow-up:

- Create ATX-SPEC-014 Bootstrap Security Model.
- Define root of trust.
- Define secure boot and measured boot requirements.
- Define pre-OS network services for netboot and time sync.
- Define recovery mode restrictions.

### Storage And Persistence

Status: NEEDS SPECIFICATION

The current specifications repeatedly reference persistence, crash recovery, journal replay, RAM-backed buffers, and copy-on-write storage, but no Storage and Persistence Model exists yet.

Confirmed direction:

```text
Persistence must be explicit.
Persistent state must be recoverable.
Unknown persistent state must be quarantined.
Audit persistence should be checksummed and crash-recoverable.
ZFS-style semantics are a long-term target, not a Rev A dependency.
```

Required follow-up:

- Create ATX-SPEC-017 Storage and Persistence Model.
- Define persistent object storage.
- Define audit persistence hierarchy.
- Define snapshot and rollback semantics.
- Define recovery after partial writes.

### Recovery And Reconciliation

Status: NEEDS SPECIFICATION

Recovery and reconciliation appear in Lifecycle, Resource, Audit, and Error specifications, but remain cross-cutting.

Confirmed direction:

```text
Recovery must not silently regrant authority.
Unknown state remains visible.
Unreconciled resources are quarantined.
Recovery is auditable.
```

Required follow-up:

- Create ATX-SPEC-018 Recovery and Reconciliation Model.
- Define crash recovery phases.
- Define resource reconciliation.
- Define directory reconciliation.
- Define capability and authority reconciliation.
- Define audit continuity verification.

## Required Future Specifications

The review identifies the following future specifications as required:

```text
ATX-SPEC-012 Versioning Model
ATX-SPEC-013 Policy Model
ATX-SPEC-014 Bootstrap Security Model
ATX-SPEC-015 POSIX Compatibility Model
ATX-SPEC-016 Supervisor Management Fabric
ATX-SPEC-017 Storage and Persistence Model
ATX-SPEC-018 Recovery and Reconciliation Model
```

## Recommended Priority Order

Recommended next specification order:

```text
1. ATX-SPEC-012 Versioning Model
2. ATX-SPEC-013 Policy Model
3. ATX-SPEC-014 Bootstrap Security Model
4. ATX-SPEC-016 Supervisor Management Fabric
5. ATX-SPEC-017 Storage and Persistence Model
6. ATX-SPEC-018 Recovery and Reconciliation Model
7. ATX-SPEC-015 POSIX Compatibility Model
```

Rationale:

- Versioning affects every wire format and persistent record.
- Policy determines conditional authority.
- Bootstrap Security and Supervisor Fabric govern early trust and recovery.
- Storage and Recovery define durable state.
- POSIX compatibility should remain deferred until native semantics are stable.

## Implementation Guidance

No major subsystem implementation should proceed without checking:

```text
Does this expose identity as authority?
Does this turn lookup into access?
Does this turn observation into control?
Does this create ambient authority?
Does this create persistence without policy?
Does this create resources without cleanup?
Does this hide unknown state?
Does this fail open when authority is uncertain?
```

## Review Decision

```text
Decision: ACCEPTED WITH REQUIRED FOLLOW-UP
Reviewer: Architecture Review 001
Date: 2026-06-14
```

## Closing Note

The current architecture is coherent enough to continue specification work, but the next phase must focus on cross-cutting models: versioning, policy, bootstrap trust, supervisor isolation, storage persistence, and recovery.

The strongest recurring Atarix principle is separation of concepts that legacy systems often conflate:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Unknown is not valid.
```


---

# Chapter 10: Object Model

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/address-space-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/address-space-architecture.md -->
### Integrated source: `docs/address-space-architecture.md`

# ATARIX Address Space Architecture

## Status

Draft architectural specification.

This document defines the long-term organization of CPU-local address spaces, fabric resources, memory-service objects, DMA windows, discovery services, mailboxes, future accelerators, and future 32-bit / 64-bit CPU cards.

## Purpose

The earlier memory-map document focuses on Rev A W65C816 bring-up.

This document defines the larger architectural model that future revisions should converge toward.

## Data Model

Address-space definitions follow:

```text
docs/data-model-and-endianness.md
```

All architecture-defined multi-byte fields are little-endian.

The W65C816's u24 address space is a CPU-local address space, not the universal ATARIX address space.

## Design Rules

CPU width is not system width.

CPU-local address width is not fabric address width.

A CPU card may expose its own local address model while the ATARIX fabric uses resource IDs, object IDs, offsets, windows, and capabilities.

## Design Goals

1. Preserve a simple Rev A bring-up path.
2. Maintain compatibility with the W65C816 24-bit address space.
3. Avoid constraining future CPU cards to the W65C816 address model.
4. Support modular hardware.
5. Support DMA mediation.
6. Support capability-based ownership.
7. Permit future MMU experimentation.
8. Permit accelerator integration.
9. Support large memory and storage objects through fabric services.

## Address Classes

ATARIX distinguishes address classes rather than assuming one global CPU address width.

```text
W65C816 native address      u24
Future 32-bit CPU address   u32
Future 64-bit CPU address   u64
Fabric register offset      u32
Fabric resource ID          u64
Memory object ID            u64
Memory object offset        u64
Storage object ID           u64
Storage object offset       u64
Capability ID               u64
```

A u24 address is valid for W65C816 CPU-local access.

It is not a universal system address.

## W65C816 Address Space

The W65C816 provides:

```text
24-bit addressing
256 banks
64 KiB per bank
16 MiB total native address space
```

ATARIX treats the W65C816 address space as the local address space of a CPU card.

Future CPU cards may expose different local address widths without changing the fabric object model.

## System Address Model

ATARIX should be modeled as a fabric of resources rather than as a single flat W65C816 memory map.

A general fabric reference should be represented conceptually as:

```text
Resource ID: u64
Offset:      u64
Length:      u32 or u64, depending on resource class
Capability:  u64
```

A CPU card may map part of a resource into a local CPU-visible aperture.

For the W65C816, that aperture may be a 16 KiB, 32 KiB, or 64 KiB window inside the 24-bit local address space.

For a future 32-bit or 64-bit CPU card, the aperture may be much larger.

## W65C816 Architectural Regions

The following regions describe the W65C816 CPU-card local view, not the entire ATARIX system.

```text
Region 0  Boot and Core Services
Region 1  General CPU-Local System Memory
Region 2  Device and I/O Space
Region 3  Fabric Services Aperture
Region 4  DMA Windows
Region 5  Discovery Space
Region 6  Accelerator Windows
Region 7  Reserved and Experimental
```

## Region 0 - Boot and Core Services

Primary bank:

```text
Bank $00
```

Contains:

```text
Direct Page
Stack
Monitor RAM
Boot Services
I/O Aperture
ROM
Interrupt Vectors
```

This region must remain usable even when advanced fabric services are unavailable.

## Region 1 - General CPU-Local System Memory

Candidate range:

```text
$010000-$7FFFFF
```

Purpose:

```text
Kernel Memory
Application Memory
Buffers
File Cache
Shared Regions
```

This region is the primary W65C816-local memory pool for software execution.

It is not the maximum possible ATARIX system memory.

Large memory is exposed through memory-service objects and windows.

## Region 2 - Device and I/O Space

Candidate range:

```text
$800000-$8FFFFF
```

Purpose:

```text
UART
Network Controllers
Storage Controllers
GPIO
Legacy Device Registers
```

Device register windows should be explicitly discoverable.

## Region 3 - Fabric Services Aperture

Candidate range:

```text
$900000-$9FFFFF
```

Purpose:

```text
Interrupt Controller
Mailbox Engine
DMA Engine
Timer Services
Fabric Diagnostics
Capability Services
```

This region becomes the primary W65C816 software interface to FPGA-mediated system services.

The fabric itself may manage resources larger than this aperture.

## Region 4 - DMA Windows

Candidate range:

```text
$A00000-$BFFFFF
```

Purpose:

```text
DMA Buffers
High-Speed Transfers
Network Buffers
Storage Buffers
Accelerator Buffers
Mapped Memory-Service Windows
```

Access should be governed by capability grants.

DMA visibility does not imply unrestricted ownership.

## Region 5 - Discovery Space

Candidate range:

```text
$C00000-$CFFFFF
```

Purpose:

```text
Discovery ROMs
Slot Discovery Records
Device Identity Tables
Supervisor Discovery Records
```

Discovery information should be readable without enabling advanced services.

See:

```text
docs/discovery-rom-format.md
```

## Region 6 - Accelerator Windows

Candidate range:

```text
$D00000-$EFFFFF
```

Purpose:

```text
ARM Compute Modules
FPGA Accelerators
Future Heterogeneous Devices
Mapped Accelerator Command Windows
```

Accelerator access should be mediated through mailbox and capability mechanisms.

Accelerators should not receive unrestricted memory visibility.

## Region 7 - Reserved and Experimental

Candidate range:

```text
$F00000-$FFFFFF
```

Potential uses:

```text
Debug Services
Supervisor Diagnostics
Experimental MMU Research
Alternate Firmware Images
Future Expansion
```

## Memory Service Objects

Memory services expose memory as objects, not as mandatory extensions of the W65C816 native address space.

A memory object should use:

```text
Memory Object ID: u64
Object Size:      u64
Object Offset:    u64
Capability ID:    u64
```

A W65C816 CPU card accesses a memory object through a mapped window.

Example:

```text
Memory Object #42
Offset: 0x0000000200000000
Window: $A00000-$A0FFFF
Length: 64 KiB
```

A future 64-bit CPU card may map a larger portion of the same object without changing the object itself.

## Storage Service Objects

Storage services should use the same large-object concept.

```text
Storage Object ID: u64
Extent Offset:     u64
Extent Length:     u64
Capability ID:     u64
```

Storage objects should not be constrained by CPU-local address width.

## Window Mapping

Window mapping is the process of exposing a portion of a fabric resource or memory object into a CPU-local aperture.

A mapping requires:

```text
Resource or Object ID
Offset
Length
Target CPU-local aperture
Capability grant
Access rights
```

The target aperture width depends on the CPU card.

For W65C816:

```text
Target CPU address: u24
Window length:      typically u16 or u32
```

For future 32-bit or 64-bit processors:

```text
Target CPU address: u32 or u64
Window length:      u32 or u64
```

## Ownership Model

Every region, object, or window may have:

```text
Owner
Permissions
Visibility Rules
Capability Requirements
```

Ownership is a logical concept and may be enforced through software, FPGA services, or future hardware assistance.

## Shared Memory

Shared memory regions should be explicitly declared.

Properties:

```text
Owner
Participants
Access Rights
Lifetime
Capability IDs
```

Shared memory should not automatically become globally visible.

## Mailbox Memory

Mailbox payload storage may reside in:

```text
Dedicated Mailbox RAM
Shared System RAM
Fabric-Owned Memory
Memory-Service Object
```

Mailbox ownership remains governed by mailbox and capability rules.

See:

```text
docs/mailbox-protocol-v1.md
```

## DMA Visibility

DMA-capable devices must possess:

```text
DMA Capability
Source Resource Capability
Destination Resource Capability
Valid Ownership Chain
```

DMA may operate on CPU-local addresses or fabric resources depending on descriptor class.

See:

```text
docs/dma-engine-v1.md
```

## Future MMU Experiments

Future revisions may introduce:

```text
Address Translation
Process Isolation
Window Mapping
Capability-Backed Address Spaces
Per-CPU Local Address Spaces
Fabric Resource Translation
```

This specification intentionally avoids requiring an MMU.

## Discovery Integration

Discovery records may describe:

```text
CPU-Local Memory Windows
Fabric Resource Windows
I/O Windows
DMA Regions
Mailbox Endpoints
Accelerator Regions
Memory Service Objects
Storage Service Objects
```

See:

```text
docs/discovery-rom-format.md
```

## Design Rules Summary

1. Region ownership is explicit.
2. DMA visibility does not imply authority.
3. Discovery remains available early in boot.
4. Boot services remain independent of advanced fabric functions.
5. Future MMU work must preserve compatibility with existing firmware where practical.
6. W65C816 u24 addresses describe CPU-local access, not universal system addressing.
7. Fabric resources use resource IDs, object IDs, offsets, windows, and capabilities.
8. Future 32-bit and 64-bit CPU cards must not be forced into the W65C816 address model.

## Open Questions

- Final W65C816-local region boundaries.
- Whether regions are fixed or configurable.
- Supervisor visibility rules.
- Shared-memory implementation details.
- Future MMU architecture.
- Accelerator memory-coherency strategy.
- Whether capability tables reside in dedicated fabric memory.
- Whether memory-service objects require 128-bit identifiers in future distributed systems.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0012-abstraction-and-compatibility.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0012-abstraction-and-compatibility.md -->
### Integrated source: `docs/adr/ADR-0012-abstraction-and-compatibility.md`

# ADR-0012: Abstraction and Compatibility

Status: Accepted

Date: 2026-06-10

## Context

ATARIX is expected to evolve across multiple CPU cards, FPGA revisions, memory technologies, storage technologies, supervisor implementations, and accelerator architectures.

If higher-level software depends on implementation details, the system will become difficult to extend or replace.

## Decision

ATARIX shall define stable architectural contracts and keep implementations replaceable.

Architectural contracts include:

- Mailbox semantics.
- DMA semantics.
- Discovery records.
- Capability records.
- Memory object model.
- Supervisor service model.
- Register access semantics.
- Boot image formats.
- Test formats.

Specific implementations of those contracts may change over time.

## Evidence

The project has already separated CPU-local addressing from fabric resources, native DMA descriptors from fabric DMA descriptors, identity EEPROMs from discovery records, and discovery from authority.

Those separations reduce coupling and preserve future compatibility.

## Alternatives Considered

- Optimize every interface for the first W65C816 implementation.
- Expose FPGA internals as architectural requirements.
- Treat fixed CPU addresses as the primary system abstraction.
- Delay abstraction until a later revision.

## Consequences

Pros:

- Future CPU cards can be added.
- Future FPGA families can be used.
- Memory and storage technologies can change.
- System-level tests remain valid across implementations.
- Software can target architectural behavior rather than board-specific details.

Cons:

- More documentation is required.
- Some implementation-specific optimizations may need wrapper interfaces.
- Initial designs may be slightly more verbose.

## Future Revisions

Future ADRs may refine individual contracts, but the project should continue to prefer stable interfaces over exposed implementation details.

## Design Rule

Implementations may change.

Contracts should remain stable.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/object-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/object-model.md -->
### Integrated source: `docs/architecture/object-model.md`

# Atarix Object Model

## Purpose

The Object Model defines the fundamental unit of identity, authority, naming, ownership, communication, and resource management within Atarix.

All externally visible entities within Atarix are represented as objects.

## Definition

An object is a uniquely identifiable entity that participates in the Atarix namespace and may possess ownership, authority, capabilities, resources, and communication endpoints.

## Object Identity

An object's identity:

- Exists independently of its name.
- Exists independently of its location.
- Exists independently of its capabilities.
- Is opaque.
- Is immutable.
- Is globally unique within the Atarix fabric.
- Is never reused.

Identity shall never be inferred from physical location, ring number, table slot, local handle, memory address, or implementation-specific layout.

## Object Naming

Objects may possess one or more human-readable names. Names are bindings, not identities.

Examples:

```text
system.directory
system.object-manager
service.discovery
device.serial0
storage.boot
fabric.node.7
```

## Required Properties

Every object has:

- Identity
- Type
- Owner
- Ring association
- Lifecycle state
- Resource accounting
- Audit visibility

## Invariants

1. Every object possesses exactly one identity.
2. Identity is opaque and immutable.
3. Identity is not a name.
4. Identity is not a mailbox.
5. Identity is not a capability.
6. Identity is not a memory address.
7. Identity is not a table slot, index, or local handle.
8. Every object possesses exactly one owner.
9. Every object belongs to exactly one ring.
10. Authority must be traceable.

<!-- AEMS-MIGRATED-SOURCE: docs/toolchain.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/toolchain.md -->
### Integrated source: `docs/toolchain.md`

# ATARIX Toolchain Strategy

## Goals

Use open and maintainable tools wherever practical.

The toolchain should support firmware, operating-system development, FPGA development, simulation, and automated testing.

## Assembly Tools

Candidate tools:

- 64TASS
- WDC tools

## FPGA Tools

Primary path:

- Yosys
- nextpnr
- Project Trellis

Target platform:

- Lattice ECP5
- ULX3S

## Editors and IDEs

Candidate tools:

- Vim
- VS Code
- Foenix IDE

## Build System

Candidate tools:

- Make
- CMake research path
- CI automation

## Simulation

Candidate tools:

- Verilator
- Icarus Verilog
- Custom W65C816 emulation

## Firmware Development

Stages:

- ROM monitor
- Diagnostics
- Loader
- Kernel

## Long-Term Objectives

- Fully reproducible builds
- Automated regression testing
- FPGA simulation workflows
- Cross-platform development support


---

# Chapter 11: Capability Model

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0013-resource-oriented-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0013-resource-oriented-architecture.md -->
### Integrated source: `docs/adr/ADR-0013-resource-oriented-architecture.md`

# ADR-0013: Resource-Oriented Architecture

Status: Accepted

Date: 2026-06-10

## Context

Early revisions of ATARIX are based on a W65C816 CPU card, but the long-term architecture includes memory services, storage services, accelerators, multiple processor types, DMA services, and capability-mediated access.

A fixed-address model centered on a single CPU address space does not scale well to those goals.

## Decision

The primary ATARIX abstraction is a resource.

Resources are represented through identifiers, offsets, windows, services, and capabilities.

CPU-local addresses remain valid within a CPU card but are not the universal architectural abstraction.

## Resource Examples

- Memory objects.
- Storage objects.
- Mailbox endpoints.
- DMA channels.
- Interrupt sources.
- Device windows.
- Accelerator queues.
- Discovery records.
- Supervisor services.

## Evidence

The architecture already uses:

- u64 resource identifiers.
- u64 object identifiers.
- Capability-mediated access.
- Window mapping.
- Fabric descriptors.

## Alternatives Considered

- Flat memory-map-centric architecture.
- CPU-address-centric architecture.
- Per-device ad hoc addressing schemes.

## Consequences

Pros:

- Easier support for future CPU cards.
- Easier support for large memory systems.
- Cleaner DMA architecture.
- Better compatibility with capability-based security.
- Supports distributed and heterogeneous resources.

Cons:

- Additional indirection.
- More metadata and discovery infrastructure.

## Design Rule

Resources, objects, services, and capabilities are primary.

Fixed CPU memory addresses are implementation details of a particular processor card.

<!-- AEMS-MIGRATED-SOURCE: docs/backplane-spec.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/backplane-spec.md -->
### Integrated source: `docs/backplane-spec.md`

# ATARIX Backplane Specification

## Purpose

This document defines the first-pass active-backplane architecture for ATARIX.

The backplane is intended to support modular CPU cards, service modules, FPGA-mediated routing, and future multiprocessor experimentation.

## Design Goals

1. Support a single CPU system initially.
2. Permit future multi-CPU experiments.
3. Avoid requiring every slot to observe every signal.
4. Allow FPGA-assisted arbitration.
5. Support recovery and fault isolation.

## Conceptual Slot Types

### CPU Slot

Hosts:

- W65C816 CPU card
- Future experimental processor modules

### Service Slot

Hosts:

- Networking
- Storage
- Supervisor functions
- Debug hardware

### Fabric Slot

Hosts:

- FPGA fabric modules
- DMA services
- Discovery services

### Accelerator Slot

Hosts:

- ARM modules
- FPGA accelerators
- Future heterogeneous compute devices

## Logical Signal Groups

### Power

Candidate rails:

- +3.3V
- +5V
- Ground
- Auxiliary standby power

### Clocking

Signals:

- System clock
- Reference clock
- Debug clock
- Clock-enable support

### Reset

Signals:

- Global reset
- Slot reset
- Recovery reset

### Interrupts

Candidate structure:

- Interrupt request lines
- Fabric-mediated interrupt routing
- Prioritized delivery

### Mailboxes

Mailbox services support:

- CPU-to-CPU communication
- CPU-to-device communication
- Supervisor messaging

### Discovery

Each slot should expose:

- Vendor identifier
- Device identifier
- Capability description
- Firmware version

## Arbitration Model

Early revisions may use centralized arbitration.

Future revisions may support:

- Bus ownership tracking
- Mailbox ownership
- DMA ownership
- Capability-controlled access

## DMA Considerations

DMA should eventually be mediated through the FPGA fabric.

The fabric should be capable of:

- Granting DMA rights
- Revoking DMA rights
- Detecting violations
- Logging faults

## Recovery Requirements

A failed module should not require complete system replacement.

Desired capabilities:

- Slot isolation
- Supervisor diagnostics
- Independent reset control
- Recovery firmware support

## Open Questions

- Connector family
- Pin count
- Mechanical card dimensions
- Maximum slot count
- Voltage distribution strategy
- Whether CPU slots are electrically identical to accelerator slots
- Fabric topology for Rev C and beyond

<!-- AEMS-MIGRATED-SOURCE: docs/data-model-and-endianness-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/data-model-and-endianness-v1.md -->
### Integrated source: `docs/data-model-and-endianness-v1.md`

# Data Model and Endianness v1

## Purpose

This document defines the canonical ATARIX data model used by firmware, Discovery ROMs, Service Directory records, mailbox packets, DMA descriptors, Boot Information Blocks, Fabric CSRs, and future processor cards.

This document exists to prevent accidental coupling between the W65C816 CPU-local view of the machine and the wider Fabric Northbridge view of the system.

## Core Rules

```text
All ATARIX-defined binary structures are little-endian.

CPU Width != Fabric Width.
Address != Handle.
CPU-local SRAM is for execution.
CPU-local DMA memory is for exchange.
```

These rules apply unless a later specification explicitly states otherwise.

## Endianness

ATARIX structures are little-endian.

This includes:

- Discovery ROM records
- Identity EEPROM records
- capability records
- Service Directory records
- Boot Information Blocks
- mailbox headers
- mailbox payload descriptors
- DMA descriptors
- Fabric CSR fields
- kernel image headers
- monitor records
- provisioning records

The W65C816 is little-endian, and keeping the full architecture little-endian avoids unnecessary conversion in firmware and early bring-up code.

Future processor cards that use a different native byte order must adapt at the CPU-card boundary and present ATARIX-defined structures in little-endian form.

## CPU-Local Address Model

The W65C816 CPU-local address model is 24-bit.

Use type name:

```text
atarix_u24_cpu_addr
```

A CPU-local address refers only to memory visible in the local W65C816 CPU-card address space.

CPU-local addresses must not be used as Fabric addresses, service handles, DMA handles, or global resource identifiers.

## Fabric Address Model

Fabric addresses are 64-bit.

Use type name:

```text
atarix_u64_fabric_addr
```

A Fabric address refers to a fabric-visible memory or resource address, not a W65C816 CPU-local address.

The W65C816 does not directly expose or directly address the complete fabric address space.

## Resource Handles

Resource handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_resource_handle
```

A resource handle is not an address.

Handles may identify:

- services
- devices
- DMA buffers
- mailboxes
- capabilities
- boot resources
- provisioning records
- diagnostic streams

Software must not infer physical address layout from a handle value.

## Service Handles

Service handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_service_handle
```

The Service Directory maps discovered services to service handles. Operating systems and firmware components should bind to service handles rather than physical slots or fixed addresses.

## DMA Handles

DMA handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_dma_handle
```

A DMA handle represents an authorized DMA buffer, window, or transaction context.

DMA handles should be capability-checked and should not be interpreted as raw addresses.

## CPU-Local DMA Boundary

CPU-local DMA memory is the translation boundary between the W65C816 CPU-local world and the Fabric Northbridge world.

```text
W65C816 CPU-local address
    -> CPU-local DMA buffer
    -> CPU-card DMA shim
    -> Fabric address / service handle / DMA handle
```

The Fabric Northbridge should not DMA directly into arbitrary CPU-local SRAM.

## Alignment

Fabric-visible structures should be naturally aligned for 64-bit transport unless a compact ROM format explicitly requires otherwise.

Recommended defaults:

```text
16-bit fields  aligned to 2 bytes
32-bit fields  aligned to 4 bytes
64-bit fields  aligned to 8 bytes
records        aligned to 8 bytes where practical
```

Discovery ROM and Identity EEPROM formats may use compact layouts, but must define alignment explicitly.

## Integer Type Names

Preferred architectural type names:

```text
atarix_u8
atarix_u16
atarix_u24_cpu_addr
atarix_u32
atarix_u64
atarix_u64_fabric_addr
atarix_u64_resource_handle
atarix_u64_service_handle
atarix_u64_dma_handle
```

These names should be reflected in future C headers under `include/atarix/`.

## Binary Format Policy

New binary specifications should explicitly state:

- endianness
- field width
- alignment
- record version
- checksum or integrity mechanism
- whether a value is an address or a handle
- whether an address is CPU-local or fabric-visible

No binary format should use an unqualified word such as `address` when `cpu_local_address`, `fabric_address`, or `handle` is meant.

## Future Processor Cards

ATARIX must support future 32-bit and 64-bit processor cards without changing resource interfaces.

Therefore:

- service handles remain 64-bit
- DMA handles remain 64-bit
- fabric addresses remain 64-bit
- binary structures remain little-endian
- CPU-specific address sizes remain local to each CPU card

## Design Consequence

The W65C816 is a first-class ATARIX CPU node, but it does not define the width of the whole machine.

The Fabric Northbridge defines the system-level transport and resource model.

<!-- AEMS-MIGRATED-SOURCE: docs/design-rationale-history-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design-rationale-history-v1.md -->
### Integrated source: `docs/design-rationale-history-v1.md`

# ATARIX Design Rationale History v1

## Purpose

This document preserves the discussion and reasoning behind the current ATARIX architectural direction.

It is not an implementation specification. Its job is to explain why the project moved from the early seed design toward the current Fabric Northbridge, service-directory, discovery-ROM, hybrid-firmware, and CPU-local DMA architecture.

## Source Material Accounted For

This rationale accounts for:

- the seed design documentation archive uploaded as `atarix_seed_design_docs.zip`
- the June 2026 discussion-output summary uploaded as `atarix_discussion_outputs.zip`
- the current repository documentation inspected during June 2026 updates
- the later design discussions around toolchain, firmware handoff, Discovery ROMs, Service Directory, mailbox transport, and CPU-local DMA memory

The uploaded seed archive contained early design notes for:

- architecture
- backplane
- FPGA fabric
- hardware roadmap
- references
- security model
- Rev A discrete W65C816 board
- Rev B backplane
- Rev C FPGA fabric
- M548 chassis concept
- BIOS / ROM monitor
- kernel notes
- BB816, Vega816, and Flipper One references

The uploaded discussion-output summary recorded discussion results for:

- boot firmware architecture
- NuBus and UPA influences
- fabric service model
- Fabric Northbridge architecture
- README updates emphasizing the Fabric Northbridge
- documentation validation tooling

## Seed Architecture

The early ATARIX seed design described a modular W65C816 experimental workstation or cyberdeck platform.

The initial revision strategy was staged:

```text
Rev A  Discrete W65C816 bring-up board
Rev B  Modular CPU-card/backplane architecture
Rev C  FPGA added as programmable chipset / smart glue
Rev D  FPGA becomes Ring 0 fabric
Rev E  Secure heterogeneous accelerator expansion
```

The key early rule was:

```text
Plugged in does not mean trusted.
```

This rule drove the later emphasis on declared, enumerated, permissioned, and sandboxed hardware.

## Why the Design Moved Beyond a Raw Backplane

The original staged design could have remained a conventional shared-bus 65C816 backplane.

That approach was rejected as the long-term architecture because it would make several goals harder:

- heterogeneous CPU cards
- capability-mediated resource access
- secure DMA
- isolated accelerators
- fabric memory services
- deterministic CPU-local execution
- service-oriented drivers
- reliable discovery and recovery

The current direction keeps the W65C816 local bus local to the CPU card and moves system integration into the Fabric Northbridge.

## Fabric Northbridge Rationale

The Fabric Northbridge became the architectural center because it gives ATARIX a clean separation between CPU-local behavior and system-wide resources.

The W65C816 remains a valid processor node, but it is not forced to behave like a modern shared-memory bus master across the whole machine.

The Fabric Northbridge is responsible for:

- service discovery
- mailbox transport
- DMA coordination
- interrupt routing
- memory services
- resource management
- future multi-processor coordination

This preserves retro-computing compatibility at the CPU-card level while allowing the larger machine to behave more like a modern modular workstation.

## NuBus and UPA Influence

NuBus influenced the self-describing card model.

The key lesson taken from NuBus is not electrical compatibility. It is the architectural idea that expansion cards should describe themselves and their resources so software does not bind blindly to slot numbers.

Sun UPA influenced the fabric/service direction.

The key lesson taken from UPA is not direct protocol cloning. It is the idea that a workstation-class machine benefits from a high-level interconnect model that can mediate traffic between processors, memory, I/O, and graphics-class devices.

The resulting ATARIX principle is:

```text
Bind to services, not slots.
```

## Three-Plane Model

The discussion-output summary recorded a three-plane architecture:

```text
Management Plane   RP2350 supervisor
Discovery Plane    enumeration, declaration, service registration
Data Plane         mailbox, DMA, memory, I/O transport
```

This separation prevents the supervisor MCU, discovery machinery, and data movement fabric from becoming one confused mechanism.

The supervisor manages reset, recovery, health monitoring, fault logging, watchdogs, and board-management functions.

The discovery plane enumerates cards and constructs system knowledge.

The data plane moves commands and payloads between services.

## Service-Oriented Architecture

ATARIX deliberately moved away from slot-oriented drivers.

A slot-oriented model would require software to know where hardware lives.

A service-oriented model lets firmware discover hardware, build a Service Directory, and let the kernel bind to capabilities.

This is important because ATARIX is intended to support:

- memory cards
- network cards
- storage services
- accelerators
- framebuffer services
- future CPU cards
- supervisor services

The Service Directory becomes the handoff object that turns discovered hardware into usable software-facing services.

## Discovery ROM Rationale

Discovery ROMs exist to preserve self-describing hardware.

The system should not depend on a hard-coded assumption that slot 3 is a network card or that a fixed address always contains a storage controller.

Each intelligent card should describe:

- vendor identity
- device identity
- class
- revision
- capabilities
- resources
- service endpoints
- DMA requirements
- interrupt requirements
- diagnostics

This supports both Rev A bring-up and later heterogeneous expansion.

## Hybrid Firmware Model Rationale

Three firmware models were considered.

### Traditional 65xx BIOS

In this model, software continues to call firmware services after boot.

Benefits:

- easy bring-up
- familiar 8-bit/16-bit monitor style
- convenient debugging

Costs:

- firmware can become an operating-system dependency
- harder to build a clean kernel ownership model
- can trap the system in a retro BIOS design

### Open Firmware Style

In this model, firmware discovers hardware and hands the machine completely to the kernel.

Benefits:

- cleaner workstation-style ownership
- kernel controls runtime policy
- better long-term architecture

Costs:

- harder early bring-up
- less convenient recovery and debugging
- more work before the machine becomes usable

### Hybrid Model

ATARIX chooses the hybrid model.

Firmware discovers hardware, builds the Service Directory and Boot Information Block, then hands ownership to the kernel.

A small emergency BIOS remains available for:

- console
- diagnostics
- recovery boot
- firmware update
- supervisor access

This keeps early bring-up practical without making ordinary runtime I/O dependent on BIOS calls.

## Toolchain Rationale

The project should not require Linux on the W65C816 target.

Linux, BSD, macOS, or another modern system may be used as the host development environment, but the target should remain an ATARIX machine.

64TASS is the practical near-term assembler path for W65C816 firmware and monitor work.

WDC tools remain useful as a vendor reference and compatibility check.

A freestanding C subset should come before any full libc.

This avoids blocking firmware bring-up on a complete operating system or hosted C runtime.

## Emulator-First Rationale

The emulator should model the architectural contract before all hardware exists.

It does not need to be cycle-accurate at first.

It should model:

- reset
- ROM loading
- CPU-local SRAM
- CPU-local DMA memory
- mailbox transport
- Fabric Northbridge stubs
- Service Directory
- Boot Information Block

The reason is simple: the firmware, kernel, and FPGA contracts need a test oracle before board bring-up becomes expensive.

## CPU-Local SRAM Rationale

The W65C816 CPU card should contain maximum directly addressable local SRAM, with a 16 MiB target.

CPU-local SRAM is for:

- kernel execution
- interrupt handlers
- direct page
- stack
- local deterministic buffers

This keeps the CPU card useful and deterministic even though larger system memory lives behind the Fabric Northbridge.

## Fabric Memory Rationale

Large system memory belongs behind the Fabric Northbridge.

Fabric memory is for:

- file cache
- framebuffers
- network buffers
- shared memory
- large datasets
- future processor cards

This avoids pretending that the W65C816 local bus is the same thing as the entire workstation memory system.

## CPU-Local DMA Memory Rationale

CPU-local DMA memory was added as an explicit staging and isolation layer.

The Fabric Northbridge should not DMA directly into arbitrary W65C816 execution memory.

Instead:

```text
W65C816 CPU
   -> CPU-local SRAM / ROM
   -> CPU-local DMA buffer memory
   -> CPU-card DMA shim
   -> Fabric Northbridge mailbox and DMA interface
   -> system services
```

The governing rule is:

```text
CPU-local SRAM is for execution.
CPU-local DMA memory is for exchange.
```

This provides:

- bounce buffering
- bus-width adaptation
- timing decoupling
- protection boundary
- deterministic CPU-local SRAM behavior
- a clean service boundary for the Fabric Northbridge

The CPU card advertises CPU-local DMA capability through the Service Directory, and mailbox messages should carry DMA descriptors rather than bulk payloads where practical.

## Security Rationale

The early seed security model treated optional hardware as untrusted until declared, enumerated, permissioned, and sandboxed.

That principle remains central.

The current architecture supports it by combining:

- supervisor management
- discovery ROMs
- service registration
- capability-mediated access
- DMA descriptors
- CPU-local DMA staging
- future fabric-enforced memory windows

The long-term security model still points toward the Fabric acting as a hardware enforcement layer.

## What Changed From the Seed Design

The seed design staged the FPGA from optional smart glue into Ring 0 fabric over several revisions.

The current design moves the Fabric Northbridge concept forward as the central organizing model earlier in the architecture documents.

This does not reject the seed design. It refines it.

The project still preserves:

- staged bring-up
- observability
- discrete W65C816 learning path
- FPGA-mediated long-term security
- modular backplane goals
- secure expansion goals

The main change is that the documents now describe the long-term fabric-centered design more explicitly.

## Current Canonical Direction

As of this rationale document, the canonical ATARIX direction is:

```text
W65C816 CPU card
    -> CPU-local SRAM
    -> CPU-local DMA memory
    -> Fabric Northbridge interface
    -> modular backplane
    -> ECP5 Fabric Northbridge
    -> Service Directory
    -> fabric memory, storage, networking, accelerators, and future CPU cards
```

The supervisor management plane remains separate.

The firmware model is hybrid.

The toolchain model is cross-development first.

The software model is service-oriented.

The security model remains capability-mediated and distrusts optional hardware by default.

## Practical Next Step

The next important work is to convert rationale into implementation contracts:

- Discovery ROM binary format
- Service Directory binary format
- Boot Information Block
- Mailbox packet format
- Fabric register map
- Kernel image format
- CPU-local DMA descriptor format

After those contracts exist, the project should move to emulator and firmware bring-up rather than continuing to add high-level architecture prose.

<!-- AEMS-MIGRATED-SOURCE: docs/design.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design.md -->
### Integrated source: `docs/design.md`

# ATARIX Architecture Design Document

## Purpose

ATARIX is a modular cyberdeck-oriented experimental workstation platform built around the W65C816, a staged FPGA system fabric, a supervisor microcontroller, and a secure heterogeneous accelerator model.

The project is intended to support long-lived experimentation in operating systems, FPGA-based system architecture, capability-oriented security, heterogeneous computing, and retro-modern workstation design.

## Design Philosophy

The system is intentionally staged. Each revision must produce a useful, testable, and debuggable computer rather than a dependency on future hardware.

Guiding principles:

1. Observable hardware.
2. Incremental complexity.
3. Recovery before optimization.
4. Capability-based expansion.
5. Explicit trust boundaries.
6. Long service life.

## System Architecture

The long-term architecture consists of:

- W65C816 CPU subsystem
- Supervisor microcontroller
- FPGA-mediated system fabric
- Active backplane
- Device discovery framework
- Capability broker
- Accelerator framework

The architecture evolves from a simple single-board computer into a modular workstation platform.

## Revision Roadmap

### Rev A

Single W65C816 bring-up platform.

Features:

- W65C816
- ROM
- SRAM
- UART
- GPIO
- Discrete address decode
- Monitor firmware

### Rev B

Modularization phase.

Features:

- CPU card preparation
- Shared services
- W5500 networking
- Golden ROM recovery
- Active backplane planning

### Rev C

FPGA chipset phase.

Features:

- Address decoding
- Interrupt routing
- Mailboxes
- DMA foundations
- Device windows

### Rev D

Protected multiprocessing phase.

Features:

- Dual CPU operation
- MMU concepts
- DMA mediation
- Security rings
- Fault containment

### Rev E

Accelerator phase.

Features:

- ARM compute modules
- FPGA accelerators
- Capability broker
- Secure command queues
- Mediated DMA

## Security Model

Devices are not trusted by default.

Capabilities describe:

- Resource ownership
- Allowed operations
- Address windows
- DMA permissions
- Revocation rules

The security architecture becomes increasingly important as multiprocessing and accelerators are introduced.

## Supervisor Controller

Responsibilities include:

- Reset sequencing
- Power monitoring
- Clock management
- Recovery functions
- FPGA configuration support
- Fault logging
- Board identity management

## FPGA Fabric

Early FPGA responsibilities:

- Address decode
- Interrupt routing
- Mailboxes
- Timers

Later responsibilities:

- DMA mediation
- Bus arbitration
- Device discovery
- Accelerator attachment
- Capability enforcement assistance

## Software Implications

Early software:

- ROM monitor
- Memory test
- Serial loader
- Device diagnostics

Later software:

- Capability broker
- Device registry
- DMA manager
- Multiprocessor services
- Accelerator runtime

## Current Status

Architectural design and planning phase.

<!-- AEMS-MIGRATED-SOURCE: docs/discovery-rom-and-slot-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/discovery-rom-and-slot-architecture-v1.md -->
### Integrated source: `docs/discovery-rom-and-slot-architecture-v1.md`

# Discovery ROM and Slot Architecture v1

## Purpose

This document expands the card-discovery model used by ATARIX and records the current architectural direction discussed during design reviews.

The goal is that software binds to capabilities and services rather than physical slot numbers.

## Design Inspiration

Architectural influences include:

- Apple II slot discovery concepts
- NuBus declaration ROMs
- Open Firmware device descriptions
- PCI capability discovery
- Modern service-oriented fabrics

## Core Rule

Applications and operating systems should not depend on:

- Slot numbers
- Backplane position
- Fixed interrupt assignments
- Fixed memory windows

Instead they should bind to discovered services.

## Discovery ROM

Every intelligent card should expose a Discovery ROM.

Conceptually:

```text
Card Inserted
    -> Discovery ROM Read
    -> Capability Enumeration
    -> Resource Enumeration
    -> Service Registration
    -> Runtime Binding
```

## Discovery Information

Recommended fields:

- Vendor identifier
- Device identifier
- Card class
- Card revision
- Firmware revision
- Capability list
- Resource descriptors
- Service endpoints
- DMA requirements
- Interrupt requirements
- Diagnostic information

## Example Classes

Examples include:

- CPU card
- Memory card
- Storage card
- Network card
- Framebuffer card
- Accelerator card
- Supervisor card
- Development card

## Service Registration

A discovered card registers services with the Fabric Northbridge.

Example:

```text
Network Card
    -> Network Service
    -> DMA Capability
    -> Interrupt Endpoint
```

The operating system consumes the resulting service directory.

## Resource Descriptors

Resources should be described rather than hard-coded.

Examples:

- Mailbox endpoints
- DMA channels
- Interrupt vectors
- Memory windows
- MMIO regions
- Shared buffers
- Boot services

## Boot-Time Enumeration

Firmware should:

1. Enumerate every card.
2. Validate Discovery ROM structures.
3. Register capabilities.
4. Build the service directory.
5. Publish a Boot Information Block.
6. Transfer control to the loader or kernel.

## Hot-Plug Considerations

Hot-plug is not required for Rev A.

However, the discovery model should not prevent future support for:

- card insertion
- card removal
- service re-registration
- capability updates

## Supervisor Integration

The supervisor processor maintains an independent view of hardware health.

Examples:

- slot power status
- thermal alarms
- watchdog events
- fault history
- recovery operations

The supervisor may assist discovery and diagnostics but should not become the sole source of hardware truth.

## Long-Term Goal

The long-term goal is a self-describing workstation fabric in which hardware announces capabilities, firmware constructs a service directory, and operating systems bind to services rather than buses, addresses, or slots.

<!-- AEMS-MIGRATED-SOURCE: docs/discovery-rom-format.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/discovery-rom-format.md -->
### Integrated source: `docs/discovery-rom-format.md`

# ATARIX Discovery ROM Format

## Status

Draft specification for the ATARIX hybrid discovery format.

This design uses a fixed header followed by typed length-value records. It is intended to be simple enough for W65C816 monitor firmware to parse while remaining extensible enough for later CPU cards, FPGA services, backplane modules, and accelerators.

## Design Choice

ATARIX uses a hybrid discovery model:

- Fixed header for fast identification.
- Vendor, device, class, and revision fields for stable driver matching.
- TLV records for capabilities, memory windows, interrupts, DMA, mailboxes, human-readable names, and future extensions.

This is inspired by NuBus-style self-description, PCI-style identity fields, and device-tree-style extensibility, but it is deliberately simpler than any of those systems.

## Goals

1. Easy parsing from ROM monitor firmware.
2. Stable driver binding.
3. Extensible capability declaration.
4. Human-readable diagnostics.
5. Compatibility with future capability-based security.
6. No mandatory dynamic allocation during early boot.

## Byte Order

All multi-byte fields are little-endian.

## Alignment

Discovery records should be aligned to 2-byte boundaries.

Padding bytes must be zero.

## Fixed Header

```text
Offset  Size  Field
+00     4     Magic: "ATDX"
+04     1     Format Major Version
+05     1     Format Minor Version
+06     2     Header Length
+08     2     Total Length
+0A     2     Header Checksum
+0C     2     Vendor ID
+0E     2     Device ID
+10     2     Device Class
+12     2     Device Subclass
+14     2     Device Revision
+16     2     Flags
+18     4     TLV Offset
+1C     4     Reserved
```

Minimum header length:

```text
32 bytes
```

## Header Fields

### Magic

The magic field must contain:

```text
ATDX
```

Meaning:

```text
ATARIX Discovery
```

### Format Version

Major version changes indicate incompatible format changes.

Minor version changes indicate backward-compatible extensions.

### Header Length

Length of the fixed header in bytes.

### Total Length

Total discovery structure length in bytes, including header, TLV records, and padding.

### Header Checksum

A simple 16-bit checksum over the fixed header with the checksum field treated as zero.

This is not a security feature. It is only a corruption-detection feature.

### Vendor ID

Vendor or project identifier.

Reserved values:

```text
$0000 Invalid
$0001 ATARIX Project
$FFFF Experimental / Local
```

### Device ID

Vendor-defined device identifier.

### Device Class

Primary device class.

```text
$0000 Unknown
$0001 CPU Card
$0002 Memory Device
$0003 Serial Device
$0004 Network Device
$0005 Storage Device
$0006 FPGA Fabric Function
$0007 Supervisor Controller
$0008 Timer Device
$0009 DMA Engine
$000A Accelerator
$000B Backplane Service
$8000-$FFFF Experimental
```

### Device Subclass

Class-specific subtype.

### Device Revision

Hardware or firmware revision.

### Flags

```text
Bit 0 Boot Critical
Bit 1 Requires Driver
Bit 2 Has Interrupts
Bit 3 DMA Capable
Bit 4 Mailbox Capable
Bit 5 Capability Restricted
Bit 6 Hot Reset Safe
Bit 7 Supervisor Visible
Bits 8-15 Reserved
```

## TLV Record Format

Each TLV record begins with:

```text
Offset  Size  Field
+00     1     Type
+01     1     Length
+02     N     Value
```

Length is the size of the value field only.

A type of `$00` marks the end of records.

## Standard TLV Types

```text
$00 End
$01 Human-Readable Name
$02 Description
$03 Memory Window
$04 I/O Register Window
$05 Interrupt Source
$06 DMA Capability
$07 Mailbox Endpoint
$08 Firmware Version
$09 Driver Binding
$0A Capability Requirement
$0B Power Requirement
$0C Clock Requirement
$0D Reset Behavior
$0E Slot Information
$0F Diagnostic Entry Point
$10 Vendor-Specific Data
$80-$FF Experimental
```

## Name Record

Type:

```text
$01
```

Value:

```text
ASCII string, not necessarily null-terminated
```

Example:

```text
ATARIX Rev B Supervisor
```

## Memory Window Record

Type:

```text
$03
```

Value:

```text
Offset  Size  Field
+00     3     Base Address, 24-bit
+03     3     Length, 24-bit
+06     1     Attributes
```

Attributes:

```text
Bit 0 Readable
Bit 1 Writable
Bit 2 Executable
Bit 3 DMA Visible
Bit 4 Cacheable, future use
Bits 5-7 Reserved
```

## I/O Register Window Record

Type:

```text
$04
```

Value:

```text
Offset  Size  Field
+00     3     Base Address, 24-bit
+03     2     Length, 16-bit
+05     1     Access Width
```

Access width:

```text
$01 8-bit
$02 16-bit
$03 Mixed
```

## Interrupt Source Record

Type:

```text
$05
```

Value:

```text
Offset  Size  Field
+00     1     Interrupt Source ID
+01     1     Default Priority
+02     1     Flags
```

Flags:

```text
Bit 0 Level Triggered
Bit 1 Edge Triggered
Bit 2 Maskable
Bit 3 Wake Capable
```

## DMA Capability Record

Type:

```text
$06
```

Value:

```text
Offset  Size  Field
+00     1     DMA Channels
+01     1     Address Width
+02     1     Flags
```

Address width is measured in bits.

Flags:

```text
Bit 0 Read From Memory
Bit 1 Write To Memory
Bit 2 Scatter/Gather Capable
Bit 3 Requires Capability Grant
```

## Mailbox Endpoint Record

Type:

```text
$07
```

Value:

```text
Offset  Size  Field
+00     1     Endpoint ID
+01     1     Max Message Size Low
+02     1     Max Message Size High
+03     1     Flags
```

Flags:

```text
Bit 0 Interrupt On Receive
Bit 1 Polling Supported
Bit 2 Responses Supported
Bit 3 Capability Required
```

## Driver Binding Record

Type:

```text
$09
```

Value:

```text
ASCII string
```

Example:

```text
atarix,w5500
```

## Capability Requirement Record

Type:

```text
$0A
```

Value:

```text
Offset  Size  Field
+00     2     Required Capability Class
+02     2     Required Capability Flags
```

This record allows a device to declare that it requires mediated access before normal use.

## End Record

Type:

```text
$00
```

Length:

```text
$00
```

## Parser Requirements

A minimal parser must:

1. Verify magic.
2. Check major version.
3. Verify header length.
4. Verify total length is reasonable.
5. Optionally verify header checksum.
6. Iterate TLV records until end marker or total length.
7. Ignore unknown TLV types.

## Monitor Commands

Future ROM monitor commands should support:

```text
DISCOVER
DISCOVER LIST
DISCOVER SHOW <device>
DISCOVER RAW <device>
```

## Open Questions

- Final checksum algorithm.
- Whether total length should be 16-bit or 24-bit.
- Whether device identity should include serial number fields.
- Whether discovery records live in ROM, EEPROM, FPGA block RAM, or supervisor-provided memory.
- Whether human-readable strings should be ASCII-only or UTF-8.

## Versioning Rule

Version 1 discovery structures must remain parseable by future firmware.

Unknown TLV records must be ignored unless marked as mandatory by a future extension.

<!-- AEMS-MIGRATED-SOURCE: docs/fpga-fabric.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/fpga-fabric.md -->
### Integrated source: `docs/fpga-fabric.md`

# ATARIX FPGA Fabric Architecture

## Purpose

The FPGA fabric serves as the evolutionary bridge between a simple W65C816 computer and a modular workstation architecture.

The FPGA begins as programmable glue logic and gradually becomes a mediated system fabric.

## Initial Platform

Development target:

- Lattice ECP5
- ULX3S development board

Reasons:

- Open-source tooling
- Strong community support
- Adequate logic resources
- Long-term availability

## Rev C Responsibilities

### Address Decode

The FPGA replaces portions of discrete decode logic.

Responsibilities:

- Chip selects
- Address windows
- Device routing

### Interrupt Router

The FPGA centralizes interrupt management.

Capabilities:

- Interrupt prioritization
- Routing tables
- Masking
- Status reporting

### Mailboxes

Mailbox services support:

- CPU-to-device messaging
- CPU-to-CPU messaging
- Supervisor communication

### Timers

Fabric timers provide:

- System tick generation
- Benchmark timing
- Watchdog support

## DMA Engine

Future DMA support includes:

- Transfer descriptors
- Completion notification
- Access validation
- Fault reporting

DMA operations should eventually be capability mediated.

## Discovery Services

Each attached module should expose:

- Vendor ID
- Device ID
- Version information
- Capability records

The design is influenced by NuBus-style discovery concepts.

## Fabric Registers

Candidate blocks:

- System control
- Interrupt controller
- Mailboxes
- DMA control
- Discovery services
- Performance counters

## Future Responsibilities

- Bus arbitration
- Device isolation
- Capability enforcement assistance
- Accelerator attachment
- Fabric-level diagnostics

## Design Rule

The FPGA must remain documented.

Every register, mailbox, interrupt route, and DMA mechanism should be described by a stable specification.

<!-- AEMS-MIGRATED-SOURCE: docs/os-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/os-architecture.md -->
### Integrated source: `docs/os-architecture.md`

# ATARIX Operating System Architecture

## Goals

The operating system should evolve with the hardware rather than being designed in isolation.

The system begins as monitor firmware and grows into a workstation operating system.

## Boot Flow

### Stage 0

- Hardware reset
- Supervisor initialization
- ROM selection

### Stage 1

- ROM monitor startup
- Memory test
- Console initialization

### Stage 2

- Program loader
- Device discovery
- Service initialization

### Stage 3

- Kernel startup
- Driver registration
- Scheduler initialization

## Kernel Services

Candidate services:

- Memory management
- Interrupt management
- Mailboxes
- Device registry
- Timer services

## Capability Broker

Responsibilities:

- Resource grants
- DMA authorization
- Device permissions
- Revocation support

## Driver Model

Drivers should register through a discovery framework.

Each driver should describe:

- Device identity
- Capabilities
- Resource requirements

## Multiprocessor Services

Future support includes:

- Message passing
- Shared-memory coordination
- Synchronization primitives

## Accelerator Runtime

Future support includes:

- Accelerator discovery
- Command queues
- Memory grants
- Completion notification

## Long-Term Goal

A modular workstation operating system capable of supporting secure experimentation across CPUs, FPGA services, and heterogeneous accelerators.

<!-- AEMS-MIGRATED-SOURCE: docs/rev-a-hardware-baseline.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/rev-a-hardware-baseline.md -->
### Integrated source: `docs/rev-a-hardware-baseline.md`

# ATARIX Rev A Hardware Baseline

## Status

Draft baseline hardware-selection record.

This document records concrete Rev A hardware choices that should guide the CPU card, supervisor, fabric-controller, and backplane specifications.

## Fabric Controller

Selected development platform:

```text
ULX3S with Lattice LFE5U-85F ECP5 FPGA
```

Rationale:

- Largest commonly available ULX3S ECP5 configuration.
- Adequate headroom for fabric-controller experiments.
- Open-source FPGA toolchain support.
- Suitable for mailbox, interrupt, DMA, discovery, debug, and health-counter logic.
- Practical bridge between prototype and custom fabric-controller card.

The ULX3S LFE5U-85F should be treated as the Rev A/B fabric-controller target unless superseded by a custom card.

## Fabric-Controller Local Memory

The ULX3S / ECP5 fabric controller should use its available SDRAM as fabric infrastructure memory.

Baseline:

```text
64 MB SDRAM on the ULX3S / ECP5 fabric-controller side
```

Primary uses:

- Mailbox storage.
- DMA queues.
- Discovery cache.
- Capability cache.
- Object-window cache.
- Trace buffers.
- Netboot staging.
- Fault logs and diagnostic buffers.

Design rule:

```text
Fabric memory is infrastructure memory.

Memory-service memory is system memory.

CPU-local memory is execution memory.
```

Rev A should not require the fabric controller to carry the burden of large system RAM. Large memory belongs on future memory-service cards.

## Supervisor MCU

Selected supervisor MCU family:

```text
RP2350
```

Responsibilities:

- RTC coordination.
- Watchdog supervision.
- Fault logging.
- Reset sequencing.
- Power-good monitoring.
- Recovery-mode control.
- Management-plane communication.
- DIP-switch and recovery-input handling.
- I2C management-bus coordination.
- Per-slot identity and health inventory.

The RP2350 should be treated as the primary supervisor-controller target.

## Per-Card Management and I2C Baseline

Rev A cards should expose a minimum independent management cluster reachable by the supervisor and/or fabric-controller management path.

Required per-card management objects:

```text
Identity EEPROM / FRU EEPROM
Temperature Sensor
Voltage Monitor
Current Monitor
Status / Control Register
```

Recommended per-card management objects:

```text
Fan Tachometer
Fan Controller
Additional ADC Channels
Environmental Sensors
```

Optional / high-reliability objects:

```text
Secure Element
Calibration EEPROM
Management MCU
```

These objects should function independently from the card's main processing logic whenever practical.

See:

```text
docs/card-identity-eeprom.md
```

## CPU-Card Memory Target

The W65C816 CPU card should provide the maximum native RAM the CPU can directly address, where practical.

Target CPU-local RAM:

```text
16 MiB CPU-local addressable RAM target
```

This corresponds to the W65C816 native 24-bit physical address space.

The CPU card should still preserve a reliable boot and recovery layout for:

- Direct page.
- Stack.
- ROM monitor workspace.
- Diagnostics.
- Netboot staging.
- Recovery operation.

Memory planning principle:

```text
Populate the CPU card with the maximum practical W65C816-addressable local RAM while preserving deterministic boot, recovery, and observability.
```

Large SDRAM or future DDR memory should be introduced through the fabric-controller path or through a controlled memory-service window, not as a reason to eliminate CPU-local execution RAM.

## Network Service Baseline

Rev A must include a network service path sufficient for netboot, NTP, and remote diagnostics.

Acceptable Rev A implementation:

```text
W5500 Ethernet on the CPU card
```

Alternative Rev A implementation:

```text
Dedicated network service card available at boot
```

Requirement:

```text
The base Rev A system must not depend on a missing optional network card in order to perform netboot.
```

Networking may move to a service card in later revisions, but Rev A must define a concrete network service path.

See:

```text
docs/netboot-ntp-v1.md
```

## Flash Device

Flash device selection remains open.

Candidate roles:

```text
CPU boot ROM / flash
Golden ROM image
FPGA bitstream storage
Supervisor firmware storage
Netboot fallback image
Configuration storage
```

Selection criteria:

- Availability.
- Programming simplicity.
- In-circuit update support.
- Recovery safety.
- Compatibility with supervisor-controlled golden image selection.
- Electrical compatibility with selected CPU, FPGA, and supervisor domains.

## CPU Clock Target

Rev A CPU card should be designed to run the W65C816 as fast as practical while preserving a deterministic debug path.

Required clock modes:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Conservative Operation
14 MHz Target Operation
Fastest Stable Experimental Mode
Single-step or gated-clock debug mode
```

The goal is not to guarantee maximum frequency on the first board.

The goal is to design the card so frequency limits are set by measurable timing margins rather than by poor observability, weak clocking, or avoidable signal-integrity problems.

## Design Implications

These selections imply:

- The fabric-controller specification should assume the LFE5U-85F resource class.
- The supervisor specification should assume RP2350 GPIO, timer, and management capabilities.
- The backplane should reserve enough signals for ULX3S-based fabric experiments.
- The CPU card should include strong diagnostic visibility around clock, RDY, chip select, interrupt, and wait-state behavior.
- The CPU card should target the full W65C816 native 16 MiB addressable RAM footprint where practical.
- SDRAM should be treated as a fabric-visible infrastructure resource with explicit ownership and test strategy.
- Flash selection should be tied to recovery architecture, not selected only by capacity.
- A Rev A network service path is mandatory for netboot and NTP.
- Per-card I2C management objects should be reachable independently of the card data plane.

## Related Documents

```text
docs/ulx3s-backplane-controller.md
docs/supervisor-card-v1.md
docs/cpu-card-architecture-v1.md
docs/clock-reset-spec.md
docs/netboot-ntp-v1.md
docs/diagnostic-access-v1.md
docs/card-identity-eeprom.md
```

## Open Questions

- Exact ULX3S connector strategy.
- Exact SDRAM topology and maximum practical capacity.
- Exact 16 MiB CPU-local RAM implementation strategy.
- Whether large memory is attached through ULX3S, CPU card, or a memory service card.
- Flash part family and package.
- Whether boot flash and FPGA bitstream flash are separate.
- CPU-card oscillator strategy.
- Maximum verified W65C816 frequency target after timing tests.
- Whether the first Rev A board should include selectable voltage-domain measurement jumpers.
- Whether W5500 is placed on the CPU card or implemented as a boot-critical network service card.

## Design Principle

Rev A should be ambitious in capability but conservative in recoverability.

The machine should be able to run fast, but it must always be possible to slow it down, observe it, recover it, and explain what failed.

<!-- AEMS-MIGRATED-SOURCE: docs/specification-roadmap.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/specification-roadmap.md -->
### Integrated source: `docs/specification-roadmap.md`

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


---

# Chapter 12: Mailbox Transport


---

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


---

# Chapter 14: Memory and Data Movement


---

---
document: ATX-100-CH15
title: Lookup Acceleration
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-020
implements:
  - include/atarix/index.h
  - rtl/atarix/atx_spec_020_accelerator.v
verified_by:
  - atx_spec_020_accelerator_ci_tb
  - atx_spec_020_modules_tb
---

# Chapter 15: Lookup Acceleration

## Principle

Lookup acceleration is not authority.

Atarix uses accelerated lookup structures to reduce latency, memory movement, cache misses, synchronization cost, and search cost. These structures may accelerate discovery, filtering, routing, indexing, or selection.

They do not grant permission.

## Canonical Rule

A lookup result identifies a candidate target. It does not authorize access to that target.

Authority still derives from:

- Capability state.
- Policy state.
- Native object state.
- Lifecycle state.
- Recovery state.
- Audit rules.

## Human-Readable Projection

Every accelerated structure must provide an explainable projection suitable for audit and debugging.

Examples:

```text
Runtime hash table -> registry view
Compressed index -> ordered generation window
Bitmap query -> explicit result set
Membership filter -> precheck explanation
```

## Audit Window Requirement

Every accelerated lookup must be reconstructable inside a bounded audit window.

An audit window should answer:

- Who requested the lookup?
- What key was used?
- Which structure was used?
- What version and generation were involved?
- What candidate was returned?
- What authority check followed?
- What policy decision followed?

## Hardware Acceleration

ATX-SPEC-020 defines an initial RTL-backed lookup acceleration path.

The implementation includes SIMD-style control byte probing, scalar comparison reference behavior, Krapivin-style stepping, Elias-Fano-style decoding, and hardware audit-window support.

The RTL simulations validate both correctness and a scalar-vs-SIMD gain proxy.

## Relationship To ATX-SPEC-020

This chapter owns the stable architectural explanation of lookup acceleration.

ATX-SPEC-020 remains responsible for detailed payload definitions, RTL module interfaces, simulation behavior, and verification evidence.

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md -->
### Integrated source: `docs/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md`

# ATX-SPEC-020 Hardware Mailbox Integration Notes

## Status

Draft v0.1

## Purpose

This note records the hardware and mailbox integration decisions for ATX-SPEC-020 Hash Table and Lookup Acceleration Model.

It captures the W65C816 plus ECP5 Fabric Northbridge integration model, corrected message payload direction, RTL correction notes, and implementation TODOs.

## Core Architectural Decision

The lookup accelerator is a service behind the Atarix mailbox framework.

```text
Client
  -> Mailbox frame
  -> Ring authorization
  -> Capability context validation
  -> Lookup service
  -> Audit window commit
  -> Reply
```

The accelerator is not directly exposed as an ambient hardware primitive.

```text
Lookup acceleration is not authority.
```

## Mailbox-First Rule

All hardware lookup acceleration requests must enter through an Atarix mailbox message.

No lookup accelerator may execute a table probe, bitmap query, membership-filter query, Elias-Fano decode, or other accelerated search operation before mailbox authorization succeeds.

## Capability Context Rule

Capability authority should be derived from mailbox context and native capability state.

The caller should not be trusted merely because it supplies a capability-like scalar in the payload.

Recommended direction:

```text
Mailbox header/source/ring context
  -> native capability lookup
  -> policy check
  -> accelerator execution permission
```

Payloads may carry a capability reference or descriptor only if the receiving service validates it against native capability state.

## Registry Identifier Rule

Accelerator requests should not expose raw physical table addresses as caller-controlled authority.

Prefer:

```text
registry_id
table_id
index_id
view_id
```

instead of:

```text
table_base
raw physical pointer
fabric-local memory address
```

The service resolves the identifier to physical placement after authorization.

## CRC Rule

CRC validation is an integrity check, not an authority check.

CRC may detect bus corruption, framing errors, or malformed transfers.

CRC must not be used for authentication, policy approval, authority, or anti-tamper guarantees.

## Deterministic Probe Limit Rule

Every probe-capable lookup structure must define a bounded forward-progress limit.

For hardware acceleration, this limit may map to a fabric down-counter.

If the probe sequence exceeds the limit, the operation must terminate with an explicit sequence error rather than hanging the memory engine.

## Audit Window Rule

The accelerator must emit an audit-window record before reporting successful completion to the CPU when the operation is authority-bearing or policy-relevant.

The audit window should include:

```text
Message sequence
Message type
Source identity
Target service identity
Registry or table identifier
Capability context result
Policy result
Structure type
Generation
Probe count or query window
Result status
Returned object or row identifier where applicable
```

Human-readable text output may be used for early development, but the final format should be a structured audit event that can also project to text.

## Corrected Message Direction

Initial sketch used raw fields such as:

```text
table_base
cap_token
fingerprint
probe_limit
```

Corrected architecture should move toward declarative service requests:

```text
message_type: INDEX_QUERY
registry_id or table_id
query_type
key or key hash
expected generation
probe budget
projection flags
```

The accelerator service chooses the physical implementation.

Clients request results.

Services choose implementations.

## Proposed Message Types

Initial proposed registry additions:

```text
ATARIX_MAILBOX_MESSAGE_INDEX_QUERY
ATARIX_MAILBOX_MESSAGE_INDEX_MUTATE
ATARIX_MAILBOX_MESSAGE_INDEX_RECOVER
ATARIX_MAILBOX_MESSAGE_INDEX_STATUS
```

Legacy or lower-level probe-specific messages may exist inside the service boundary, but should not be the public compatibility interface unless explicitly approved by architecture review.

## Corrected Payload Sketch

The following C sketch is not yet committed to `include/atarix/index.h` because that path was not found in the current repository search results.

It records the corrected direction for a future implementation issue.

```c
#ifndef ATARIX_INDEX_H
#define ATARIX_INDEX_H

#include <stdint.h>
#include "atarix/mailbox.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_MAILBOX_MESSAGE_INDEX_QUERY   0x0020u
#define ATARIX_MAILBOX_MESSAGE_INDEX_MUTATE  0x0021u
#define ATARIX_MAILBOX_MESSAGE_INDEX_RECOVER 0x0022u
#define ATARIX_MAILBOX_MESSAGE_INDEX_STATUS  0x0023u

typedef uint32_t atarix_index_registry_id_t;
typedef uint32_t atarix_index_generation_t;

typedef enum atarix_index_query_type {
    ATARIX_INDEX_QUERY_EXACT      = 0x00u,
    ATARIX_INDEX_QUERY_RANGE      = 0x01u,
    ATARIX_INDEX_QUERY_FILTER     = 0x02u,
    ATARIX_INDEX_QUERY_BITMAP     = 0x03u,
    ATARIX_INDEX_QUERY_ORDERED    = 0x04u
} atarix_index_query_type_t;

#pragma pack(push, 1)

typedef struct atarix_payload_index_query {
    uint32_t registry_id;
    uint32_t key_hash;
    uint32_t expected_generation;
    uint8_t  query_type;
    uint8_t  probe_limit;
    uint8_t  projection_flags;
    uint8_t  reserved;
} atarix_payload_index_query_t;

typedef struct atarix_payload_index_reply {
    uint32_t resolved_id;
    uint32_t resolved_generation;
    uint8_t  status;
    uint8_t  match_offset;
    uint16_t audit_window_id;
} atarix_payload_index_reply_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_INDEX_H */
```

## RTL Correction Notes

The RTL sketches discussed so far are not yet a verified implementation.

Known corrections before implementation:

```text
1. Replace any typo such as landmark_offset with resolved_lane_offset.
2. Do not expose raw table_base as public authority.
3. Treat caller-provided capability tokens as references requiring native validation, not authority.
4. CRC32 is integrity only, not security.
5. Confirm CRC polynomial, bit reflection, seed, and final XOR against the mailbox framing contract.
6. Ensure the priority encoder has deterministic default behavior.
7. Ensure Elias-Fano select/rank logic is implemented as a verified primitive, not an accidental behavioral loop.
8. Audit commit must occur before successful completion is signaled for authority-bearing operations.
9. Probe-limit exhaustion must return an explicit sequence error.
10. Filter positive results must always be confirmed by authoritative lookup.
```

## ECP5 Northbridge Pipeline Requirement

The hardware pipeline should enforce this order:

```text
Frame receive
  -> Magic/header validation
  -> CRC integrity validation
  -> mailbox ring authorization
  -> capability context validation
  -> policy validation
  -> optional membership precheck
  -> authoritative table/index lookup
  -> ordered/bitmap decode if required
  -> audit-window commit
  -> reply
```

No implementation may move membership probing ahead of authorization when that would leak presence information.

## Elias-Fano Hardware Direction

Elias-Fano acceleration should expose verified rank/select style primitives.

The implementation must produce a human-readable ordered projection for audit windows.

The architecture should not depend on an unbounded bit-by-bit walk.

## Roaring Bitmap Direction

Roaring bitmap routing should be developed as a separate implementation task.

Required properties:

```text
Container type visibility
Query plan auditability
Explicit result window projection
Policy-preserving intersection
Recovery-checkable container metadata
```

## Prime Catalog Direction

Hardware and software table sizing should use signed offline prime catalogs where appropriate.

The catalog is a sizing aid only.

It is not authority.

## Summary

The hardware integration is architecturally sound if implemented as a mailbox-authorized lookup service rather than as a raw probe primitive.

Required corrections are:

```text
Use registry/table identifiers instead of raw addresses.
Validate capability through native mailbox context.
Treat CRC as integrity only.
Make probe limits mandatory.
Commit audit windows before successful completion.
Keep filters non-authoritative.
Expose human-readable projections.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md`

# ATX-SPEC-020 Hash Table and Lookup Acceleration Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Hash Table and Lookup Acceleration Model.

Lookup cost is a system resource.

ATX-SPEC-020 defines how Atarix minimizes lookup latency, memory footprint, cache misses, synchronization overhead, and search cost without weakening authority, policy, audit, recovery, human readability, or compatibility semantics.

This specification covers lookup and indexing for:

```text
Objects
Capabilities
Mailboxes
Services
Namespaces
Directories
Resources
Policies
Audit events
Storage metadata
POSIX compatibility state
Research-driven acceleration structures
```

## Core Rule

```text
Lookup acceleration is not authority.
```

A lookup structure may accelerate discovery, indexing, filtering, routing, caching, or search.

It must not become the source of authority.

Authority still comes from:

```text
Capabilities
Policy
Native object state
Lifecycle state
Recovery state
Audit rules
```

## Human-Readable Abstraction Rule

Atarix must preserve human-readable architecture even when machine-optimized data structures are used.

Every accelerated structure must provide an explainable projection.

```text
Machine-optimized structure
    +
Human-readable audit projection
```

Examples:

```text
Runtime hash table -> registry view
Roaring bitmap -> explicit result set
Elias-Fano index -> ordered event or generation window
Membership filter -> precheck explanation, never authority
```

No compressed, probabilistic, cached, or accelerated structure may become opaque authority.

## Audit Window Rule

Every accelerated lookup must be reconstructable inside a bounded audit window.

An audit window should be able to answer:

```text
Who requested the lookup?
What key was looked up?
Which structure was used?
What version and generation were involved?
What object, service, mailbox, or capability was returned?
What authority check followed?
What policy decision followed?
What time, generation, or version window applied?
```

Lookup acceleration must preserve audit explainability.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

This specification informs:

- ATX-SPEC-021 Memory and Data Movement Model

Related research:

- ATX-RESEARCH-003 Advanced Lookup and Index Structures
- ATX-RESEARCH-004 Memory Locality and Allocation Systems
- ATX-RESEARCH-005 Zero-Copy and Data Movement Architectures
- ATX-RESEARCH-006 Prime Lookup and Sieve Acceleration

## Design Goals

Atarix lookup structures should provide:

```text
Low average lookup latency
Predictable tail latency
Cache-efficient memory layout
NUMA-friendly placement where applicable
Shard-local operation where applicable
Human-readable projections
Audit-window reconstruction
Recovery-safe rebuild
Version-aware behavior
Generation-aware behavior
Capability-safe lookup
Policy-safe lookup
```

Performance is valuable only when it preserves architecture semantics.

## Lookup Hierarchy

Atarix defines a lookup hierarchy.

```text
Tier 0: Direct identity lookup
Tier 1: Runtime hash tables
Tier 2: Ordered compressed indexes
Tier 3: Membership filters
Tier 4: Bitmap and search structures
Tier 5: Persistent indexes
```

Not every lookup problem is a hash table problem.

Each subsystem should use the lowest-complexity structure that satisfies correctness, performance, auditability, and recovery requirements.

## Tier 0: Direct Identity Lookup

Direct identity lookup is the fastest lookup class.

It applies when an Atarix identifier already carries enough information to directly locate or validate the target through native object state.

Examples:

```text
ObjectID
CapabilityID
MailboxID
ServiceID
GenerationID
VersionID
DescriptorID
AuditEventID
```

Direct identity lookup must still validate lifecycle, generation, authority, and policy where required.

Direct identity lookup is not authority by itself.

## Tier 1: Runtime Hash Tables

Runtime hash tables are used for high-frequency dynamic registries.

Atarix defines the initial canonical runtime table family:

```text
ATARIX_TABLE_V1
```

`ATARIX_TABLE_V1` should be based on open addressing without element reordering, informed by Krapivin-style optimal bounds for open addressing without reordering.

Required properties:

```text
Open addressing
No element reordering during ordinary insertion
Cache-friendly probe sequences
Generation-aware entries
Version-aware table format
Recovery-checkable metadata
Deterministic rebuild behavior
High-load-factor support
Human-readable registry projection
```

Candidate users:

```text
Object Registry
Capability Registry
Mailbox Registry
Service Registry
Descriptor Registry
Policy Registry
Resource Registry
POSIX process table
POSIX descriptor table
```

## Tier 2: Ordered Compressed Indexes

Ordered compressed indexes are used for monotonic identifiers, generation windows, event windows, and version chains.

The initial approved structure family is:

```text
Elias-Fano-style monotonic indexes
```

Used for:

```text
Audit event IDs
Object generation sequences
Storage version chains
Namespace generation lists
Recovery checkpoints
Time-ordered service events
```

Required capabilities:

```text
Ordered traversal
Range lookup
Predecessor/successor lookup where applicable
Compact storage
Human-readable ordered projection
Audit window reconstruction
```

## Tier 3: Membership Filters

Membership filters are precheck structures.

They may reduce expensive lookups.

They are never authoritative.

Approved initial families:

```text
Quotient filters
XOR filters
Cuckoo filters
```

Rules:

```text
A positive filter result is not authority.
A positive filter result must be confirmed by authoritative lookup.
A negative filter result may avoid work only when the filter type and policy permit that use.
Filter version and generation must be auditable.
```

Recommended use:

```text
Quotient filter -> dynamic namespace and directory presence checks
XOR filter -> read-mostly catalogs and static repositories
Cuckoo filter -> dynamic cache membership with deletion support
```

## Tier 4: Bitmap And Search Structures

Bitmap and search structures accelerate query-style lookups.

The initial approved bitmap family is:

```text
Roaring bitmap-style indexes
```

Used for:

```text
Policy matching
Capability search
Mailbox ownership search
Service tag search
Audit event search
Object attribute search
Namespace attribute search
```

Bitmap query results must project to human-readable result sets.

Bitmap intersections must be auditable as query plans and result windows.

## Tier 5: Persistent Indexes

Persistent indexes belong to storage and persistence layers.

They may use structures such as:

```text
B+ trees
LSM trees
Fractal trees
Append-only indexes
Persistent hash indexes
```

Persistent indexes must conform to ATX-SPEC-017 and ATX-SPEC-018.

Persistent lookup structures are not runtime authority.

Storage metadata must reconcile with native object and authority state before use.

## Prime And Sizing Strategy

Atarix may use prime-sized tables, shard sizes, bucket counts, and partition sizes where mathematically justified.

Prime lookup and sizing should use signed offline-generated catalogs rather than repeated runtime prime searches.

This requirement is informed by ATX-RESEARCH-006 and the existing code-noodling prime/sieve work.

Prime catalogs may include:

```text
Prime values
Prime gaps
Recommended table capacities
Recommended resize thresholds
Shard sizing metadata
Filter sizing metadata
Generation and signature metadata
```

Rules:

```text
No authority derives from prime catalogs.
Prime catalogs are sizing aids.
Prime catalogs must be versioned.
Prime catalogs used by trusted components must be signed or otherwise integrity-checked.
Runtime fallback sizing must be deterministic.
```

## Optimization Budget

Atarix permits implementation optimizations only when architecture semantics are preserved.

Required baseline families:

```text
ATARIX_TABLE_V1 runtime hash tables
Roaring bitmap-style query indexes
Elias-Fano-style ordered indexes
Membership filters
Human-readable audit windows
Recovery-safe rebuild
```

Permitted optimization families:

```text
SIMD or vectorized probing
Control-byte probing
Prefetch-aware probe scheduling
NUMA-aware table placement
Per-core or shard-local tables
Hot/cold key separation
Epoch-based reclamation
Perfect hashing for immutable registries
Learned indexes for large read-mostly datasets
GPU or FPGA acceleration for bulk audit/search workloads
Offline signed sizing catalogs
```

Forbidden optimization outcomes:

```text
Opaque lookup authority
Unexplainable cache hits
Probabilistic authority
Unrecoverable indexes
Indexes that bypass policy
Indexes that bypass capability checks
Indexes that bypass audit
Indexes that cannot project human-readable state
```

## Namespace Acceleration

Namespace and directory lookup should use layered acceleration.

Recommended stack:

```text
Membership filter
Runtime hash table
Ordered generation index
```

Example:

```text
Quotient filter
    -> ATARIX_TABLE_V1
        -> Elias-Fano generation index
```

Rules:

```text
Path lookup is not access.
Namespace lookup is not authority.
Directory visibility is policy-controlled.
Accelerated namespace state must reconcile with native directory state.
```

## Mailbox Acceleration

Mailbox lookup and routing should use:

```text
ATARIX_TABLE_V1
Roaring bitmap-style ownership and routing indexes
```

Used for:

```text
Mailbox identity lookup
Mailbox ownership lookup
Subscription lookup
Group routing
Broadcast targeting
Queue accounting
```

All accelerated mailbox state must conform to ATX-SPEC-005 lifecycle rules.

## Service Acceleration

Service discovery and service lookup should use:

```text
ATARIX_TABLE_V1 for service identity lookup
Membership filters for presence checks
Roaring bitmap-style tag indexes
Ordered indexes for service generations
```

Rules:

```text
Service discovery is not service authority.
Service location is not service identity.
Service endpoint names are discovery artifacts.
```

## Policy Acceleration

Policy evaluation may use accelerated structures.

Recommended stack:

```text
ATARIX_TABLE_V1
Roaring bitmap-style policy match sets
Elias-Fano-style generation windows
```

Policy acceleration must preserve the distinction between possible authority and exercised authority.

A cached policy result must remain version-aware, generation-aware, and audit-visible.

## Audit Acceleration

Audit search and audit window construction should use:

```text
Roaring bitmap-style indexes
Elias-Fano-style ordered event indexes
Persistent indexes for long-term storage
```

Audit acceleration must preserve:

```text
Event order
Actor identity
Object identity
Policy decision context
Authority decision context
Generation window
Version window
Integrity status
```

Audit acceleration must never erase native decision context.

## POSIX Compatibility Acceleration

ATX-SPEC-015 compatibility state may use ATX-SPEC-020 structures.

Candidate structures:

```text
POSIX descriptor tables -> ATARIX_TABLE_V1
POSIX process tables -> ATARIX_TABLE_V1
Path caches -> membership filter + table
Network endpoint caches -> table + service binding index
Compatibility audit windows -> bitmap + ordered index
```

Compatibility acceleration must preserve ATX-SPEC-015 rules:

```text
POSIX is a compatibility personality.
Path lookup is not access.
Descriptors are compatibility handles.
Native IPC is mailbox-based.
Compatibility recovery restores execution, not authority.
```

## Recovery Requirements

Every acceleration structure must support recovery behavior.

Required recovery operations:

```text
Snapshot
Verify
Rebuild
Reconcile
Quarantine
Human-readable projection
```

A corrupt index must not become authority.

If an index cannot be verified, the system must use an authoritative fallback path or quarantine the affected lookup domain.

```text
Lookup state is not authority.
Index state is not authority.
Cache state is not authority.
```

## Versioning And Generation Requirements

All acceleration structures must have explicit version and generation metadata.

Required metadata:

```text
Structure type
Structure version
Generation
Owner subsystem
Authority domain reference
Recovery domain reference
Audit projection format
Human-readable projection format
Integrity metadata where required
```

Unknown versions must be explicit.

Unknown versions must not silently downgrade into unsafe behavior.

## Human-Readable Projection Requirements

Every accelerated structure must define a projection format suitable for diagnostics and audit review.

Projection examples:

```text
Registry table -> list of entries by key, object, generation, lifecycle
Bitmap query -> explicit result set plus query terms
Membership filter -> filter metadata and confirmation path
Ordered index -> bounded ordered event/window listing
Persistent index -> storage object mapping plus generation range
```

A human reviewer must be able to inspect what the structure claims and how it relates to native Atarix state.

## Mathematical Foundations Registry

Initial approved mathematical and data-structure foundations:

```text
Krapivin-style open addressing without reordering
Elias-Fano monotonic compressed indexes
Roaring bitmap-style compressed bitmaps
Quotient filters
XOR filters
Cuckoo filters
Prime sizing catalogs
Cache-aware segmented lookup structures
```

Research items do not become architecture merely by appearing in this registry.

A structure becomes normative only when this specification or a later Architecture Review assigns it to a subsystem or table class.

## Required Tests

Initial tests should verify:

```text
Accelerated lookup does not grant authority
Membership filter positives require authoritative confirmation
Membership filter negatives obey policy and filter semantics
Table lookup preserves generation checks
Table lookup preserves lifecycle checks
Bitmap query produces human-readable result set
Audit window can reconstruct accelerated lookup
Corrupt index fails closed
Index rebuild preserves authority boundaries
Version mismatch is explicit
Unknown structure type fails safely
POSIX descriptor lookup preserves ATX-SPEC-015 semantics
Mailbox lookup preserves ATX-SPEC-005 lifecycle semantics
Service lookup preserves service identity vs location distinction
```

## Required Future Work

- Define ATARIX_TABLE_V1 record format.
- Define table class registry.
- Define prime catalog format.
- Define hash function policy.
- Define probe sequence policy.
- Define bitmap query plan format.
- Define ordered index projection format.
- Define membership filter confirmation rules.
- Define benchmark suite.
- Define adversarial lookup tests.
- Define NUMA and shard-local placement rules.
- Define GPU/FPGA bulk search eligibility.
- Reconcile with ATX-SPEC-021 Memory and Data Movement Model.

## Summary

ATX-SPEC-020 defines lookup acceleration for Atarix.

Its central rules are:

```text
Lookup acceleration is not authority.
Optimized lookup must remain human-readable.
Every accelerated lookup must fit inside an audit window.
Probabilistic filters are never authority.
A corrupt index must fail closed or rebuild.
Performance must not weaken architecture semantics.
```

<!-- AEMS-MIGRATED-SOURCE: docs/research/RESEARCH-BACKLOG.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/research/RESEARCH-BACKLOG.md -->
### Integrated source: `docs/research/RESEARCH-BACKLOG.md`

# ATARIX Research Backlog

## Purpose

This document tracks research topics under investigation.

Architecture specifications represent decisions.
Research items represent investigations.

## Active Research Areas

### ATX-RESEARCH-002 Data-Oriented Systems Architecture

Focus:
- Ownership-centric design
- Arena allocation
- Cache-friendly layouts
- Zero-copy transport
- Authority movement vs data movement
- Persistent memory integration
- Recovery-aware design

### ATX-RESEARCH-003 Advanced Lookup and Index Structures

Focus:
- Krapivin open addressing
- Bubble-up d-ary cuckoo hashing
- Modern cuckoo filters
- Roaring bitmaps
- SPIDER rank/select structures
- Elias-Fano indexes
- Succinct data structures

Target Subsystems:
- Directory
- Services
- Capabilities
- Resources
- Audit

### ATX-RESEARCH-004 Memory Locality and Allocation Systems

Focus:
- Arena allocators
- Region allocators
- Generational allocation
- NUMA placement
- Fabric-aware memory placement
- Persistent memory
- Recovery-aware allocation

### ATX-RESEARCH-005 Zero-Copy and Data Movement Architectures

Focus:
- Zero-copy networking
- Scatter/gather I/O
- DMA capability models
- Buffer ownership systems
- Content-addressed transfer
- Fabric transport optimization

### ATX-RESEARCH-006 Prime Lookup and Sieve Acceleration

Focus:
- Existing OSE.c segmented 6-wheel CPU sieve from dlworrell/code-noodling
- Existing OSE_CUDA.cc multi-GPU segmented odd-only CUDA sieve from dlworrell/code-noodling
- JSON prime table export
- Cache-aware segmented sieves
- Bit-packed sieve representations
- Wheel factorization
- Incremental sieves
- Prime-counting algorithms
- Distributed sieve partitioning
- FPGA sieve acceleration
- Persistent prime databases
- Prime lookup services

Target Subsystems:
- Hash and lookup acceleration
- Memory locality research
- Data movement research
- Service-oriented computational fabrics
- Future math service experiments

## Guiding Principle

Research informs architecture.

Research does not become architecture until benchmarked, reviewed, documented, and accepted.

<!-- AEMS-MIGRATED-SOURCE: docs/research/hashing/krapivin-2025-open-addressing-summary.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/research/hashing/krapivin-2025-open-addressing-summary.md -->
### Integrated source: `docs/research/hashing/krapivin-2025-open-addressing-summary.md`

# Research Summary: Optimal Bounds for Open Addressing Without Reordering

## Status

Research note

## Paper

```text
Title: Optimal Bounds for Open Addressing Without Reordering
Authors: Martin Farach-Colton, Andrew Krapivin, William Kuszmaul
Date: 2025-01-04
arXiv: 2501.02305
```

## Repository Use

This note records why the paper is relevant to Atarix and how it may influence future lookup-table architecture.

The paper itself is not vendored into this repository. The repository should track architectural implications, implementation hypotheses, and benchmark requirements rather than storing a static PDF artifact.

## High-Level Relevance

Atarix depends on fast lookup structures in multiple subsystems:

```text
Directory Service
Service Registry
Object Manager
Capability Engine
Resource Manager
Audit Indexing
Error and Metadata Tables
```

Lookup performance affects both runtime speed and security enforcement latency.

A slow lookup path can become a denial-of-service surface or force excessive caching that complicates lifecycle, revocation, and audit semantics.

## Paper Claim To Investigate

The paper revisits open-addressed hash tables and studies insertion without reordering existing elements.

The authors show that open addressing without reordering can achieve stronger expected search complexities than previously believed, with matching lower bounds, and they revisit Yao's earlier optimality conjecture.

## Atarix Architectural Interpretation

The immediate Atarix conclusion is not to adopt a specific algorithm blindly.

The architectural conclusion is:

```text
Lookup structures are architectural performance infrastructure,
not ad hoc implementation detail.
```

Future lookup tables should be specified, versioned, benchmarked, and tested as part of the system architecture.

## Candidate Atarix Applications

### Directory Service

Path and object identity lookup may use hash tables for binding lookup, alias resolution caches, and generation-aware binding indexes.

### Service Registry

Service discovery may require fast lookup by service name, service identity, interface version, provider identity, and policy scope.

### Capability Engine

Capability validation may require fast lookup by capability identity, target object identity, operation, revocation state, and generation.

### Resource Manager

Resource allocation and accounting may require fast lookup by resource identity, owner, allocation ID, quota group, and lifecycle state.

### Audit Index

Audit queries may require fast secondary indexes over event type, object identity, actor label, service identity, and sequence ranges.

## Design Constraints

Any Atarix hash-table design must preserve:

```text
Authority checks
Capability revocation
Policy enforcement
Lifecycle cleanup
Audit visibility
Versioned record formats
Recovery and reconciliation
Resource accounting
Deterministic failure behavior
```

Performance must not override security semantics.

## Candidate Design Direction

Future ATX-SPEC-020 should investigate:

```text
Open-addressed hash tables
No-reordering insertion models
Cache-friendly probing
Versioned table headers
Generation-aware entries
Explicit tombstone policy
Explicit rebuild policy
Transient vs persistent table formats
Crash recovery behavior
Benchmark methodology
```

## Versioning Requirements

Lookup table formats should carry explicit versions.

At minimum:

```text
Table format version
Hash policy version
Probe policy version
Entry schema version
Generation or epoch metadata
```

Unknown table versions must fail closed for authority-bearing lookups.

## Recovery Requirements

Persistent or recovery-relevant tables must define:

```text
Integrity checks
Rebuild-from-source policy
Journal or replay relationship
Tombstone handling
Partial-write behavior
Quarantine behavior
```

A corrupted lookup accelerator must not become corrupted authority.

If a lookup table cannot be trusted, the system should rebuild it, quarantine it, or fall back to a slower authoritative structure.

## Benchmark Requirements

A future benchmark suite should test:

```text
Successful lookup probe counts
Failed lookup probe counts
Insertion cost
Deletion and tombstone behavior
High-load-factor behavior
Cache behavior
W65C816-local feasibility
Fabric-memory feasibility
Auxiliary-compute-card feasibility
Recovery rebuild time
Adversarial key distributions
```

Benchmarks should compare candidate designs against simple baseline tables before adopting complexity.

## Open Questions

```text
What load factors are acceptable for each subsystem?
Which tables are transient and which are persistent?
Which tables must be deterministic across reboot?
Which tables can be rebuilt from authoritative records?
How are tombstones audited or compacted?
Can lookup acceleration live on auxiliary compute cards?
What is the minimum viable Rev A implementation?
```

## Related Architecture Work

```text
ATX-SPEC-008 Directory Service Model
ATX-SPEC-009 Resource Model
ATX-SPEC-010 Audit Model
ATX-SPEC-012 Versioning Model
ATX-SPEC-019 Service Model
ATX-SPEC-020 Hash Table and Lookup Acceleration Model
```

## Summary

The paper should inform Atarix's lookup architecture, but the system must prove the performance benefit under Atarix-specific constraints before relying on it.

The target is not merely a faster hash table.

The target is a secure, auditable, recoverable, versioned lookup infrastructure that can support fast authority and object resolution without weakening the architecture.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/Architecture-Review-002.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/Architecture-Review-002.md -->
### Integrated source: `docs/reviews/Architecture-Review-002.md`

# Architecture Review 002

## Status

Review complete

## Scope

This review covers the Atarix architecture baseline developed through the current architecture and research pass.

Architecture scope:

```text
ATX-SPEC-001 Security Model
ATX-SPEC-002 Authority Model
ATX-SPEC-003 Capability Model
ATX-SPEC-004 Lifecycle Model
ATX-SPEC-005 Mailbox Model
ATX-SPEC-006 Object Model
ATX-SPEC-007 Namespace Model
ATX-SPEC-008 Directory Service Model
ATX-SPEC-009 Resource Model
ATX-SPEC-010 Audit Model
ATX-SPEC-011 Error Model
ATX-SPEC-012 Versioning Model
ATX-SPEC-013 Policy Model
ATX-SPEC-014 Bootstrap Security Model
ATX-SPEC-016 Supervisor Management Fabric
ATX-SPEC-017 Storage and Persistence Model
ATX-SPEC-018 Recovery and Reconciliation Model
ATX-SPEC-019 Service Model
```

Backlog scope:

```text
ATX-SPEC-015 POSIX Compatibility Model
ATX-SPEC-020 Hash Table and Lookup Acceleration Model
ATX-SPEC-021 Memory and Data Movement Model
```

Research scope:

```text
ATX-RESEARCH-002 Data-Oriented Systems Architecture
ATX-RESEARCH-003 Advanced Lookup and Index Structures
ATX-RESEARCH-004 Memory Locality and Allocation Systems
ATX-RESEARCH-005 Zero-Copy and Data Movement Architectures
ATX-RESEARCH-006 Prime Lookup and Sieve Acceleration
```

## Executive Summary

The architecture has reached a coherent baseline.

The central theme is consistent:

```text
Discovery is not authority.
Observation is not control.
Persistence is not authority.
Recovery is not administrative access.
Service location is not service identity.
Visibility is not ownership.
Compatibility is not authority.
```

The architecture is strongest where it separates visibility, identity, authority, policy, lifecycle, and audit.

The largest remaining risks are not inside individual specifications. They are cross-document consistency risks.

## Review Result

```text
Architecture Coherence:        Strong
Security Model Alignment:      Strong
Authority Separation:          Strong
Audit Coverage:                Strong
Recovery Coverage:             Good
Versioning Coverage:           Good
Lifecycle Coverage:            Good
Implementation Specificity:    Intentionally early
Research Capture:              Good and improving
```

## Major Strengths

### 1. Authority Is Consistently Separated

The architecture repeatedly distinguishes identity, lookup, discovery, observation, persistence, recovery, compatibility, and service location from authority.

This is the strongest architectural property of the current system.

### 2. Policy Is Correctly Positioned

The Policy Model places policy between possible authority and exercised authority.

Capabilities describe what could be done.

Policy decides whether it may be done now.

This prevents capabilities from becoming unconditional access grants.

### 3. Recovery Does Not Become Administrative Bypass

The Recovery and Reconciliation Model explicitly prevents recovery from becoming hidden administrative access.

Recovery restores trust.

Recovery does not restore authority.

### 4. Supervisor Isolation Is Coherent

The Supervisor Management Fabric is control-isolated but audit-visible.

This aligns with the broader rule that observation is not control.

### 5. Versioning And Generation Are Properly Separated

The Versioning Model establishes a necessary distinction:

```text
Version    = compatibility
Generation = instance history
```

This distinction should remain mandatory in all future specifications.

### 6. Research Is Now Properly Separated From Architecture

The research backlog explicitly states that architecture specifications are decisions while research items are investigations.

This separation helps prevent promising algorithms or experiments from becoming undocumented architectural requirements.

## Findings

### Finding 001: ATX-SPEC-015 Is Now The Highest Priority Functional Gap

POSIX compatibility is the next major semantic risk.

POSIX concepts such as users, groups, processes, file descriptors, sockets, signals, fork, exec, permissions, and paths must be mapped onto Atarix objects, capabilities, mailboxes, services, policy, audit, and lifecycle.

Recommendation:

```text
Draft ATX-SPEC-015 POSIX Compatibility Model next.
```

### Finding 002: ATX-SPEC-020 And ATX-SPEC-021 Should Remain Research-Informed

Hash table acceleration, lookup structures, memory locality, zero-copy transfer, prime lookup, and succinct indexing are valuable research directions.

They should not harden into architecture until benchmarked and reconciled with authority, audit, recovery, and lifecycle constraints.

Recommendation:

```text
Keep ATX-SPEC-020 and ATX-SPEC-021 in backlog until after POSIX compatibility and at least one benchmark/research synthesis pass.
```

### Finding 003: Lifecycle State Names Need A Reconciliation Pass

Many specifications define lifecycle-like state sets, but their names differ by subsystem.

Examples include:

```text
ACTIVE
DEGRADED
QUARANTINED
FAILED
UNKNOWN
RECOVERY_PENDING
RECOVERY_ACTIVE
REGISTERED
MIGRATING
RETIRED
DESTROYED
```

The variation is reasonable, but common states should be standardized where possible.

Recommendation:

```text
Create a lifecycle state vocabulary appendix or reconciliation table.
```

### Finding 004: Error Codes Need A Cross-Spec Registry

The Error Model defines the concept of structured errors, and later specs introduce domain-specific errors.

These should be consolidated into a registry before implementation begins.

Recommendation:

```text
Create docs/architecture/error-code-registry.md or an appendix to ATX-SPEC-011.
```

### Finding 005: Audit Event Types Need A Cross-Spec Registry

Audit requirements are strong, but audit event names are distributed across many documents.

Bootstrap, supervisor, policy, service, storage, recovery, and versioning each introduce events.

Recommendation:

```text
Create docs/architecture/audit-event-registry.md or an appendix to ATX-SPEC-010.
```

### Finding 006: Authority Classes Need A Compact Registry

The architecture now references several authority classes.

Examples:

```text
Runtime authority
Bootstrap authority
Supervisor authority
Recovery authority
Recovery signing authority
Audit export authority
Observation authority
Service authority
Cleanup authority
```

Recommendation:

```text
Create docs/architecture/authority-class-registry.md or an appendix to ATX-SPEC-002.
```

### Finding 007: Research Mapping Is Now Good Enough For Forward Work

The research backlog now captures:

```text
Data-oriented architecture
Advanced lookup and indexing
Memory locality and allocation
Zero-copy/data movement
Prime lookup and sieve acceleration
```

The backlog correctly maps research to future specifications and target subsystems.

Recommendation:

```text
Keep research separate and require benchmark evidence before adopting research-driven mechanisms into specifications.
```

## Dependency Review

The current high-level dependency structure is coherent.

### Foundation Layer

```text
Security
  -> Authority
      -> Capability
          -> Policy
```

### Object And Naming Layer

```text
Object
  -> Namespace
      -> Directory
```

### Operational Layer

```text
Mailbox
Resource
Audit
Error
Versioning
Lifecycle
```

### Control And Trust Layer

```text
Bootstrap Security
Supervisor Management Fabric
Recovery and Reconciliation
```

### Runtime Layer

```text
Service Model
Storage and Persistence
```

### Future Layer

```text
POSIX Compatibility
Hash Table and Lookup Acceleration
Memory and Data Movement
```

## Cross-Cutting Principles Confirmed

The following principles should be treated as architecture-wide rules:

```text
Discovery is not authority.
Lookup is not access.
Observation is not control.
Persistence is not authority.
Recovery is not administrative access.
Compatibility is not authority.
Service location is not service identity.
Visibility is not ownership.
Unknown authority fails closed.
Unknown compatibility is explicit.
Quarantine beats unsafe execution.
```

## Risks

### Risk 001: POSIX Semantics May Conflict With Native Atarix Semantics

POSIX assumes ambient process identity, file descriptors, permissions, signals, fork, and path-based access.

Atarix is object, service, capability, and policy oriented.

Mitigation:

```text
Define POSIX as a compatibility personality, not native system truth.
```

### Risk 002: Research Optimizations Could Accidentally Weaken Security Semantics

Lookup acceleration, zero-copy movement, and persistent prime/index tables may bypass ordinary authority paths if adopted carelessly.

Mitigation:

```text
Require capability mediation, policy checks, audit hooks, and recovery behavior for optimized paths.
```

### Risk 003: Registry Debt

As specifications grow, distributed lists of states, errors, events, and authority classes may drift.

Mitigation:

```text
Create registries before implementation begins.
```

## Required Follow-Up Work

```text
1. Draft ATX-SPEC-015 POSIX Compatibility Model.
2. Create lifecycle state reconciliation table.
3. Create error code registry.
4. Create audit event registry.
5. Create authority class registry.
6. Later draft ATX-SPEC-020 after research synthesis.
7. Later draft ATX-SPEC-021 after storage, recovery, and POSIX interactions are clearer.
```

## Architecture Review Decision

Architecture Review 002 accepts the current architecture baseline as coherent enough to proceed.

The next specification should be:

```text
ATX-SPEC-015 POSIX Compatibility Model
```

The next reconciliation work should focus on:

```text
Lifecycle states
Error codes
Audit events
Authority classes
```

## Summary

The Atarix architecture is now internally coherent at the conceptual level.

The strongest property of the architecture is the repeated separation of identity, visibility, discovery, persistence, compatibility, recovery, and authority.

The next major challenge is POSIX compatibility, because POSIX will introduce legacy assumptions that must not weaken Atarix-native authority, policy, audit, and recovery semantics.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/Architecture-Review-003.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/Architecture-Review-003.md -->
### Integrated source: `docs/reviews/Architecture-Review-003.md`

# Architecture Review 003

## Status

Complete

## Tracking Issue

- Issue #16: Architecture Review 003: ATX-SPEC-015 POSIX Compatibility Reconciliation

## Scope

This review reconciles ATX-SPEC-015 POSIX Compatibility Model against:

- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Result

PASS

ATX-SPEC-015 is architecture-complete for Revision 1.

## Review Findings

Architecture Review 003 identified five clarification-level findings:

```text
1. Mailbox lifecycle conformance reference
2. Native audit chain participation
3. Degraded-state propagation semantics
4. Subordinate recovery domain statement
5. Service-discovery artifact clarification
```

## Resolution

All five findings were incorporated into ATX-SPEC-015.

Resolved by ATX-SPEC-015 cleanup commit:

```text
5f75de430f6b48082ae0e7305eb693c20502c5d3
```

## Final Status

```text
ATX-SPEC-015 POSIX Compatibility Model
Status: Reconciled
Status: Architecture Complete for Revision 1
Blocking Issues: None
```

## Ready For

```text
ATX-SPEC-020 Hash Table and Lookup Acceleration Model
ATX-SPEC-021 Memory and Data Movement Model
```


---

# Chapter 16: Processor Modules

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-005-Evolution-Strategy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-005-Evolution-Strategy.md -->
### Integrated source: `docs/design/ATX-DESIGN-005-Evolution-Strategy.md`

# ATX-DESIGN-005: Evolution Strategy

## Purpose

This document defines how Atarix evolves without losing architectural coherence.

## Principle

Processor modules, implementation technologies, and performance mechanisms may change. Architectural invariants require explicit review before change.

## Initial Evolution Rules

- New CPU cards should consume fabric services rather than redefine them.
- New services require specifications and review.
- Stable architectural concepts migrate into ATX-100.
- Superseded working documents are archived for historical reference.
- AEMS should record lifecycle transitions.

<!-- AEMS-MIGRATED-SOURCE: docs/hardware.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/hardware.md -->
### Integrated source: `docs/hardware.md`

# ATARIX Hardware Architecture

## Overview

ATARIX is intended to evolve from a simple W65C816 single-board computer into a modular workstation architecture built around CPU cards, an active backplane, FPGA-mediated services, and heterogeneous compute modules.

## Processor Strategy

### Initial Processor

- W65C816S
- Native mode operation
- Banked memory model
- Observable external bus

### Future Processor Modules

- Single CPU card
- Dual CPU experimentation
- SMP research configurations
- FPGA-assisted interprocessor communication

## Backplane Architecture

The long-term goal is an active backplane rather than a passive bus.

Responsibilities:

- Slot identification
- Arbitration
- Interrupt distribution
- Discovery services
- DMA mediation
- Recovery isolation

## FPGA Strategy

Initial target:

- Lattice ECP5 family
- ULX3S development platform during early development

Early responsibilities:

- Address decode
- Interrupt routing
- Mailboxes
- Timers

Later responsibilities:

- DMA control
- Fabric services
- Accelerator attachment
- Capability enforcement assistance

## Supervisor Controller

Responsibilities:

- Power sequencing
- Reset management
- Recovery support
- Configuration storage
- FPGA loading
- Fault logging

## Memory Architecture

Early revisions:

- ROM
- SRAM

Future revisions:

- Expanded memory windows
- DMA-visible regions
- Capability-controlled mappings

## Networking

Initial networking:

- W5500 Ethernet controller

Future networking:

- Fabric-aware services
- Remote management
- Distributed experimentation

## Design Objectives

- Serviceability
- Observability
- Recoverability
- Incremental complexity
- Long service life


---

# Chapter 17: Fabric Northbridge


---

# Chapter 18: Supervisor Controller

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/clock-reset-spec.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/clock-reset-spec.md -->
### Integrated source: `docs/clock-reset-spec.md`

# ATARIX Clock and Reset Specification

## Status

Draft hardware specification for clock generation, clock distribution, reset sequencing, supervisor control, and FPGA startup.

## Purpose

Clocking and reset behavior form the foundation of the ATARIX platform.

This specification establishes deterministic startup behavior for:

- W65C816 CPU cards
- Supervisor controller
- FPGA fabric
- Service modules
- Future accelerator cards

## Design Goals

1. Deterministic power-on behavior.
2. Recoverable startup sequence.
3. Debug-friendly operation.
4. Supervisor-controlled reset.
5. FPGA configuration before CPU release when required.
6. Future multi-CPU support.

## Clock Domains

Initial clock domains:

```text
Supervisor Clock Domain
CPU Clock Domain
FPGA Fabric Clock Domain
Peripheral Clock Domain
```

Future revisions may introduce accelerator-specific clock domains.

## Primary Oscillator

Recommended system reference:

```text
25 MHz
```

Reasons:

- Common FPGA reference frequency.
- Suitable for Ethernet timing derivation.
- Easy PLL generation.
- Good long-term availability.

## Supervisor Clock

The supervisor controller should start immediately after power stabilization.

Supervisor clock requirements:

```text
Independent of W65C816 startup
Independent of FPGA configuration status
Available during recovery mode
```

The supervisor must remain operational even if the CPU or FPGA fails.

## CPU Clock Generation

Initial supported CPU clocks:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Normal
14 MHz Experimental
```

Clock selection may be controlled by:

- Supervisor firmware
- DIP switches
- Configuration EEPROM
- Recovery mode settings

## Debug Clock Mode

Debug mode should support:

```text
Single Step
Slow Clock
Continuous Clock
```

The supervisor may optionally gate CPU clock generation during debugging.

## FPGA Clocking

FPGA clock sources:

```text
Reference Oscillator
PLL-Derived Clocks
Optional External Debug Clock
```

Fabric services should not assume CPU and FPGA clocks are identical.

Clock-domain crossing must be explicit.

## Reset Sources

Possible reset sources:

```text
Power-On Reset
Supervisor Reset
Manual Reset Button
Recovery Button
Watchdog Timeout
Software Reset Request
Fabric Fault Reset
```

## Reset Tree

Logical hierarchy:

```text
Power Good
    |
Supervisor Controller
    |
    +-- FPGA Reset
    |
    +-- CPU Reset
    |
    +-- Slot Reset
```

The supervisor is the root reset authority.

## Reset Sequencing

### Phase 1

Power rails stabilize.

### Phase 2

Supervisor exits reset.

### Phase 3

Supervisor validates power-good status.

### Phase 4

FPGA configured if required.

### Phase 5

Fabric services initialized.

### Phase 6

CPU released from reset.

### Phase 7

Boot monitor executes.

## Recovery Mode

Recovery mode may hold:

```text
CPU in Reset
DMA Disabled
Accelerators Disabled
```

while leaving:

```text
Supervisor Active
UART Active
FPGA Configuration Access Active
```

## Watchdog Architecture

Supervisor watchdog responsibilities:

- Detect stalled CPU.
- Detect failed boot sequence.
- Trigger recovery.
- Record reset cause.

Watchdog expiration should never immediately erase diagnostic information.

## Reset Cause Register

Recommended retained causes:

```text
Power-On Reset
Manual Reset
Recovery Request
Watchdog Reset
Software Reset
Fabric Fault
Unknown
```

The ROM monitor should display reset cause information during startup.

## Multi-CPU Considerations

Future systems may support:

```text
Global Reset
Per-CPU Reset
Per-Slot Reset
```

The supervisor should be capable of resetting a single CPU card without affecting the remainder of the system.

## Design Rules

1. The supervisor always starts first.
2. CPU release is intentional, never automatic.
3. DMA remains disabled during early boot.
4. Accelerators remain disabled during early boot.
5. Recovery mode must remain available after failed firmware updates.
6. Clock generation must remain observable during debugging.

## Open Questions

- Final oscillator frequency.
- Supervisor MCU selection.
- External clock input support.
- FPGA configuration storage location.
- Multi-CPU synchronization strategy.
- Watchdog timeout values.
- Whether clock gating occurs in supervisor hardware or FPGA fabric.

<!-- AEMS-MIGRATED-SOURCE: docs/memory-map.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/memory-map.md -->
### Integrated source: `docs/memory-map.md`

# ATARIX Memory Map

## Purpose

This document captures the first-pass ATARIX memory-map plan. It is intentionally conservative for Rev A and leaves room for FPGA-mediated banking, device windows, DMA control, and capability enforcement in later revisions.

## Design Rules

1. Rev A must be simple enough to debug with ordinary tools.
2. ROM, RAM, and UART must be easy to verify independently.
3. Later FPGA windows must not invalidate the Rev A bring-up model.
4. Device registers must be explicitly documented.
5. DMA-visible memory must eventually be mediated by the fabric.

## W65C816 Addressing Context

The W65C816 presents a 24-bit address space organized as 256 banks of 64 KiB each.

For early bring-up, ATARIX should treat bank `$00` as the most important bank because it contains reset vectors, interrupt vectors, direct-page assumptions, stack behavior, and most monitor activity.

## Rev A Draft Map

```text
Bank $00

$000000-$0000FF  Direct page / zero-page-compatible region
$000100-$0001FF  Hardware stack default region
$000200-$007FFF  Low RAM / monitor work area
$008000-$00BFFF  Expansion or device window candidate
$00C000-$00DFFF  I/O window candidate
$00E000-$00FFFF  ROM window, vectors at top of bank
```

This map favors easy decoding and easy monitor development over density.

## Rev A Required Regions

### RAM

Minimum useful RAM:

- Direct page
- Stack page
- Monitor variables
- Upload buffer
- Test area

### ROM

ROM must contain:

- Reset vector
- Native interrupt vectors
- Emulation-mode vectors if used during bring-up
- Monitor firmware
- Memory-test entry points
- Serial loader entry points

### UART

The UART should be placed in the I/O window and decoded simply.

Candidate location:

```text
$00C000-$00C00F  UART register block
```

### GPIO

Candidate location:

```text
$00C100-$00C1FF  GPIO register block
```

## Rev B Extensions

Rev B may add:

- W5500 Ethernet register window
- Supervisor mailbox
- Board identity registers
- Recovery control registers

Candidate locations:

```text
$00C200-$00C2FF  Supervisor mailbox
$00C300-$00C3FF  Board identity / configuration
$00C400-$00C7FF  W5500 window
```

## Rev C FPGA Register Space

Rev C introduces FPGA-mediated system registers.

Candidate locations:

```text
$00D000-$00D0FF  FPGA system control
$00D100-$00D1FF  Interrupt controller
$00D200-$00D2FF  Mailbox registers
$00D300-$00D3FF  DMA control
$00D400-$00D4FF  Timer block
$00D500-$00D5FF  Device discovery block
```

## Future Banked Memory

Banks above `$00` may eventually be used for expanded RAM, memory windows, accelerator buffers, or DMA-visible regions.

Candidate organization:

```text
$010000-$7FFFFF  General RAM / banked system memory
$800000-$BFFFFF  Device and accelerator windows
$C00000-$EFFFFF  Fabric-controlled mappings
$F00000-$FFFFFF  Reserved / ROM shadow / debug / recovery
```

This is provisional and should not be treated as a final hardware contract.

## DMA Visibility

DMA-capable devices must not receive unrestricted access to the full address space.

Future DMA regions should include:

- Owner CPU or supervisor
- Start address
- Length
- Direction
- Expiration or revocation rule
- Fault behavior

## Open Questions

- Final Rev A ROM size
- Final Rev A SRAM size
- Exact UART part
- Whether Rev A uses a 16 KiB or 8 KiB I/O aperture
- Whether ROM is mirrored for simple decoding
- Whether FPGA registers enter at Rev C only or appear in limited form in Rev B

<!-- AEMS-MIGRATED-SOURCE: docs/register-map-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/register-map-v1.md -->
### Integrated source: `docs/register-map-v1.md`

# ATARIX Register Map v1

## Status

Draft register specification for Rev B and Rev C development.

This document defines the first stable hardware/software contract between the W65C816, supervisor controller, FPGA fabric, diagnostics, and future operating-system services.

## Address Allocation

All registers reside within the primary I/O aperture.

```text
$00C000-$00C0FF UART
$00C100-$00C1FF GPIO
$00C200-$00C2FF Supervisor Mailbox
$00C300-$00C3FF Board Identity
$00D000-$00D0FF Fabric Control
$00D100-$00D1FF Interrupt Controller
$00D200-$00D2FF Mailbox Engine
$00D300-$00D3FF DMA Engine
$00D400-$00D4FF Timers
$00D500-$00D5FF Discovery Services
$00D600-$00D6FF Performance Counters
```

---

## UART Block

Base Address:

```text
$00C000
```

Registers:

```text
+00 DATA
+01 STATUS
+02 CONTROL
+03 BAUD
```

STATUS bits:

```text
Bit 0 RX Ready
Bit 1 TX Ready
Bit 2 Framing Error
Bit 3 Overrun
```

---

## Supervisor Mailbox

Base:

```text
$00C200
```

Registers:

```text
+00 COMMAND
+01 STATUS
+02 ARG0
+03 ARG1
+04 ARG2
+05 ARG3
```

Example commands:

```text
01 Reset Request
02 Recovery Mode
03 Read Board ID
04 Read Fault Log
05 FPGA Reload
```

---

## Board Identity Block

Base:

```text
$00C300
```

Fields:

```text
+00 Vendor ID Low
+01 Vendor ID High
+02 Board ID Low
+03 Board ID High
+04 Revision
+05 Capability Flags
```

---

## Fabric Control Block

Base:

```text
$00D000
```

Registers:

```text
+00 Fabric Version
+01 Fabric Status
+02 Fabric Control
+03 Fault Status
```

Fabric Control bits:

```text
Bit 0 Enable Fabric Services
Bit 1 Enable Discovery
Bit 2 Enable DMA
Bit 3 Enable Diagnostics
```

---

## Interrupt Controller

Base:

```text
$00D100
```

Registers:

```text
+00 Pending
+01 Mask
+02 Priority
+03 Acknowledge
```

Interrupt Sources:

```text
0 UART
1 Supervisor
2 Timer
3 Mailbox
4 DMA
5 Network
6 Accelerator
7 Reserved
```

---

## Mailbox Engine

Base:

```text
$00D200
```

Registers:

```text
+00 Mailbox Status
+01 Mailbox Control
+02 Message Length
+03 Source ID
+04 Destination ID
+05 Message Type
```

Mailbox payload storage is implementation dependent.

---

## DMA Engine

Base:

```text
$00D300
```

Registers:

```text
+00 Control
+01 Status
+02 Source Low
+03 Source High
+04 Destination Low
+05 Destination High
+06 Length Low
+07 Length High
```

Future revisions may expand to descriptor-based operation.

---

## Timer Block

Base:

```text
$00D400
```

Registers:

```text
+00 Counter Low
+01 Counter High
+02 Compare Low
+03 Compare High
+04 Control
```

---

## Discovery Services

Base:

```text
$00D500
```

Registers:

```text
+00 Device Count
+01 Selected Device
+02 Vendor ID Low
+03 Vendor ID High
+04 Device ID Low
+05 Device ID High
+06 Capability Pointer
```

Inspired by NuBus-style discovery concepts.

---

## Performance Counters

Base:

```text
$00D600
```

Registers:

```text
+00 Cycle Counter Low
+01 Cycle Counter High
+02 Interrupt Count
+03 DMA Count
+04 Mailbox Count
```

---

## Versioning Rule

New revisions may add registers.

Existing register meanings must not change without incrementing the major specification version.

<!-- AEMS-MIGRATED-SOURCE: docs/system-spec-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/system-spec-v1.md -->
### Integrated source: `docs/system-spec-v1.md`

# ATARIX System Specification v1

## Status

Draft engineering specification for Rev A and Rev B development.

This document serves as the authoritative reference until superseded by later revisions.

---

# 1. System Objectives

Rev A objectives:

- Boot a W65C816 successfully.
- Execute monitor firmware.
- Verify RAM.
- Verify ROM.
- Verify UART communications.
- Establish development workflow.

Rev B objectives:

- Introduce modularization.
- Add supervisor services.
- Add networking.
- Prepare for FPGA integration.

---

# 2. Processor

Primary CPU:

- Western Design Center W65C816S.

Operating modes:

- Native mode preferred.
- Emulation mode available for diagnostics.

Clock targets:

- Initial: 1 MHz debug clock.
- Normal bring-up: 4 MHz.
- Target operational range: 8–14 MHz depending on board implementation.

---

# 3. Memory Architecture

## Rev A Memory Map

```text
$000000-$0000FF Direct page
$000100-$0001FF Stack
$000200-$007FFF General RAM
$008000-$00BFFF Expansion window
$00C000-$00DFFF I/O window
$00E000-$00FFFF ROM
```

Vector table remains in ROM.

---

# 4. UART

Requirements:

- Console access
- Firmware upload
- Diagnostics

Candidate devices:

- 65C51-compatible UART
- FPGA UART implementation

Default rate:

- 115200 baud

---

# 5. Supervisor Controller

Responsibilities:

- Power sequencing
- Reset sequencing
- Recovery support
- Configuration storage
- Fault logging

Candidate families:

- RP2350
- STM32
- ESP32-S3

---

# 6. FPGA Fabric

Development target:

- ULX3S
- Lattice ECP5

Initial responsibilities:

- Address decode
- Interrupt routing
- Timers
- Mailboxes

Future responsibilities:

- DMA
- Discovery
- Capability enforcement assistance

---

# 7. Backplane

Initial slot types:

- CPU
- Service
- Fabric
- Accelerator

Design assumptions:

- Active backplane
- Supervisor visibility
- Slot isolation capability

---

# 8. Networking

Rev B networking:

- W5500 Ethernet

Functions:

- Firmware transfer
- Remote diagnostics
- Development support

---

# 9. Security Foundations

Principle:

Devices are not trusted by default.

Future systems should use:

- Capability grants
- DMA mediation
- Revocation mechanisms

---

# 10. Software Stack

Stage 1:

- ROM monitor
- Diagnostics

Stage 2:

- Loader
- Networking

Stage 3:

- Kernel
- Driver framework

Stage 4:

- Capability broker
- Multiprocessor services

Stage 5:

- Accelerator runtime

---

# 11. Deliverables

Rev A:

- Working CPU board
- ROM monitor
- Memory diagnostics
- UART console

Rev B:

- Supervisor services
- Ethernet support
- Modular architecture foundation

---

# 12. Open Engineering Decisions

- CPU card connector
- Backplane connector
- Final UART implementation
- Supervisor controller selection
- FPGA deployment strategy
- Register map finalization
- DMA architecture
- Discovery ROM format
- Capability encoding

This document establishes the baseline implementation target for ATARIX Rev A and Rev B.

<!-- AEMS-MIGRATED-SOURCE: docs/ulx3s-backplane-controller.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ulx3s-backplane-controller.md -->
### Integrated source: `docs/ulx3s-backplane-controller.md`

# ULX3S Backplane Fabric Controller

## Status

Draft hardware design note.

This document records the decision that the first ATARIX backplane fabric controller should be implemented using a ULX3S-class Lattice ECP5 FPGA board mounted on, or connected directly to, the ATARIX backplane.

## Purpose

The ULX3S board provides a practical early FPGA platform for implementing and testing ATARIX fabric concepts before designing a custom fabric-controller card.

The ULX3S should be treated as the Rev A/B fabric-controller development platform rather than a temporary afterthought.

## Design Decision

The ATARIX backplane should include support for a ULX3S FPGA module with DIP-switch configuration.

The ULX3S fabric controller may provide:

- Address decode experiments.
- Interrupt routing.
- Mailbox services.
- Discovery-service exposure.
- DMA-controller experiments.
- Slot identification.
- Reset and clock coordination assistance.
- Debug and diagnostic logic.

## Backplane Role

The ULX3S-connected FPGA acts as the first implementation of the ATARIX fabric controller.

Conceptual topology:

```text
DIN41612 Backplane
        |
ULX3S / ECP5 Fabric Controller
        |
Fabric Control Logic
        |
CPU Cards / Service Cards / Accelerator Cards
```

## DIP-Switch Purpose

The backplane should expose a DIP-switch block associated with the fabric controller.

Candidate DIP-switch functions:

```text
Switch 0 Recovery Mode
Switch 1 Golden Bitstream / Normal Bitstream
Switch 2 Debug Clock Enable
Switch 3 Fabric Services Enable
Switch 4 DMA Enable
Switch 5 Discovery Override
Switch 6 Slot Test Mode
Switch 7 Reserved
```

The exact switch mapping may change, but the design should reserve manual configuration inputs from the beginning.

## Recovery Requirement

The system must support a human-controlled recovery path.

Recovery mode should be able to:

- Hold CPU cards in reset.
- Disable DMA.
- Disable accelerators.
- Select a known-good FPGA bitstream.
- Permit supervisor or UART diagnostics.
- Bring up the fabric in a minimal safe state.

## FPGA Development Advantages

Using ULX3S early provides:

- Lattice ECP5 toolchain compatibility.
- Open-source FPGA flow support.
- Existing board availability.
- Faster HDL experimentation.
- Reduced risk before custom PCB design.

## Integration With Existing Architecture

This design note connects to:

```text
docs/fpga-fabric.md
docs/bus-architecture.md
docs/backplane-spec.md
docs/clock-reset-spec.md
docs/interrupt-architecture-v1.md
docs/mailbox-protocol-v1.md
docs/dma-engine-v1.md
```

## Design Rule

The ULX3S fabric controller should be part of the backplane design from the beginning.

Do not design the backplane first and then try to attach the FPGA later.

The FPGA, DIP switches, recovery path, and slot-control signals are first-class backplane requirements.

## Open Questions

- Whether the ULX3S mounts directly to the backplane or through an adapter card.
- Which ULX3S connectors are used for fabric signals.
- Whether the DIP switch is on the backplane, adapter, or ULX3S carrier.
- How many FPGA I/O lines are reserved for slot control.
- Whether the supervisor MCU or ULX3S owns reset sequencing in Rev B.
- Whether FPGA configuration storage lives on the ULX3S or the backplane.


---

# Chapter 19: Storage and Networking

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/cpu-local-dma-memory-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/cpu-local-dma-memory-v1.md -->
### Integrated source: `docs/cpu-local-dma-memory-v1.md`

# CPU-Local DMA Memory Architecture v1

## Purpose

This document defines the role of CPU-local DMA memory on the ATARIX W65C816 CPU card.

CPU-local DMA memory is a staging and isolation layer between the W65C816 local bus and the Fabric Northbridge. It allows the CPU card to exchange data with fabric services without exposing arbitrary CPU-local memory to system-wide DMA.

## Architectural Position

```text
W65C816 CPU
   |
Local CPU Bus
   |
CPU-local SRAM / ROM
   |
CPU-local DMA Buffer Memory
   |
CPU Card DMA Engine / Shim
   |
Fabric Northbridge Mailbox + DMA Interface
   |
System Services
```

The Fabric Northbridge should not DMA directly into arbitrary W65C816 memory.

Instead:

1. The CPU allocates or exposes local DMA buffers.
2. The CPU-card DMA shim owns transfers between those buffers and the Fabric Northbridge.
3. The Fabric Northbridge sees a controlled endpoint rather than the raw W65C816 local bus.
4. Firmware or the kernel copies, maps, or promotes data from local DMA memory into normal execution data structures.

## Design Rule

CPU-local SRAM is for execution.

CPU-local DMA memory is for exchange.

## Responsibilities

CPU-local SRAM is intended for:

- kernel execution
- interrupt handlers
- direct page
- stack
- normal local buffers
- deterministic execution

CPU-local DMA memory is intended for:

- fabric ingress buffers
- fabric egress buffers
- mailbox payload staging
- block I/O staging
- network packet buffers
- framebuffer command staging
- service replies
- diagnostic capture buffers

## Why Local DMA Memory Exists

CPU-local DMA memory provides:

- bounce buffering
- bus-width adaptation
- timing decoupling
- a protection boundary
- deterministic CPU-local SRAM behavior
- a clean interface to the Fabric Northbridge
- a way to avoid forcing the W65C816 into a modern shared-memory bus model

## DMA Ownership

The CPU card owns its local DMA memory.

The Fabric Northbridge may request transfers to or from CPU-local DMA buffers, but should not directly master the W65C816 execution memory space.

The CPU-card DMA shim mediates:

- buffer readiness
- transfer direction
- transfer size
- alignment
- completion status
- error reporting
- interrupt or mailbox completion notification

## Service Directory Exposure

The CPU card should advertise its DMA capability through the Service Directory.

A CPU-card DMA service descriptor should include:

```text
service_class          CPU_LOCAL_DMA
buffer_base            CPU-local address or handle
buffer_size            bytes
alignment              bytes
max_transfer_size      bytes
supported_directions   fabric-to-cpu, cpu-to-fabric, bidirectional
coherency_rules        explicit flush/invalidate unless otherwise stated
completion_model       interrupt, mailbox, polling, or mixed
```

The operating system should bind to this service rather than assuming a fixed memory address.

## Mailbox Integration

Mailbox messages should carry DMA descriptors rather than large payloads whenever practical.

Example DMA request:

```text
DMA_REQUEST
    source_service
    destination_service
    local_buffer_handle
    length
    direction
    flags
    transaction_id
```

Small control messages may remain inline in mailbox payloads, but bulk data should use DMA descriptors.

## Boot Information Block Integration

Firmware should publish CPU-local DMA information in the Boot Information Block.

Draft structure:

```c
struct atarix_dma_local_info {
    uint32_t dma_buffer_base;
    uint32_t dma_buffer_size;
    uint16_t dma_alignment;
    uint16_t dma_flags;
};
```

The kernel must validate this information before using CPU-local DMA buffers.

## Memory Hierarchy

The CPU-local DMA layer extends the ATARIX memory hierarchy:

```text
L0  CPU Registers
L1  Direct Page / Stack
L2  CPU-local SRAM
L3  CPU-local DMA Buffer Memory
L4  Fabric Memory Services
L5  Persistent Storage, Network, and Accelerators
```

## Rev A Policy

For Rev A, CPU-local DMA memory should be simple, explicit, and observable.

Recommended constraints:

- fixed-size DMA buffer region
- firmware-visible descriptor table
- explicit ownership bits
- explicit completion status
- logic-analyzer-friendly control signals where possible
- no implicit coherency
- no direct DMA into arbitrary kernel structures

## Long-Term Direction

Later revisions may support richer DMA windows, scatter/gather lists, or capability-checked buffer handles.

Those features should extend the CPU-local DMA service model rather than bypassing it.

<!-- AEMS-MIGRATED-SOURCE: docs/data-model-and-endianness.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/data-model-and-endianness.md -->
### Integrated source: `docs/data-model-and-endianness.md`

# ATARIX Data Model and Endianness

## Status

Draft architecture specification.

This document defines the binary data model for ATARIX architecture-defined structures, including byte order, integer widths, address widths, length fields, identifiers, timestamps, and future 32-bit / 64-bit expansion rules.

## Purpose

ATARIX begins with a W65C816 CPU, but the system architecture is larger than the CPU's native execution width.

The W65C816 is an 8/16-bit processor with a 24-bit physical address space. ATARIX also includes an FPGA fabric, mailbox protocols, DMA descriptors, discovery records, supervisor logs, memory-service objects, network boot images, and future 32-bit / 64-bit processors or accelerators.

Those structures must not be limited to a 16-bit or 24-bit worldview.

## Design Rule

CPU width is not system width.

CPU-local address width is not fabric address width.

The W65C816 may execute 8-bit and 16-bit code and use 24-bit native addresses, but ATARIX architecture-defined binary formats may use 8-bit, 16-bit, 24-bit, 32-bit, and 64-bit fields where appropriate.

## Byte Order

ATARIX architecture-defined binary structures use little-endian byte order.

This applies to:

- Discovery records.
- Mailbox headers.
- DMA descriptors.
- Capability records.
- Boot image headers.
- Fault logs.
- Supervisor records.
- Memory-service objects.
- Fabric registers wider than 8 bits.
- Future accelerator command blocks.

## Integer Widths

Standard integer widths:

```text
u8   Unsigned 8-bit integer
u16  Unsigned 16-bit integer, little-endian
u24  Unsigned 24-bit integer, little-endian
u32  Unsigned 32-bit integer, little-endian
u64  Unsigned 64-bit integer, little-endian
```

Signed integers should be avoided in hardware-facing protocols unless explicitly required.

## Address Classes

ATARIX distinguishes several address classes.

```text
W65C816 native address      u24
Future 32-bit CPU address   u32
Future 64-bit CPU address   u64
Fabric register offset      u32
Fabric resource address     u64
Memory object identifier    u64
Memory object offset        u64
Persistent storage offset   u64
```

The existence of u24 CPU-local addresses must not constrain the ATARIX fabric, memory-service architecture, or future processor cards.

## 24-Bit CPU-Local Addresses

The W65C816 directly uses 24-bit physical addresses.

Native W65C816-visible addresses should use:

```text
u24 cpu_address
```

Examples:

- CPU local memory windows.
- Rev A memory map addresses.
- CPU-visible I/O apertures.
- Short descriptors targeting native W65C816 CPU space.

A u24 address is encoded least-significant byte first.

A u24 field is not a universal ATARIX address.

## Fabric Addresses and Resource References

ATARIX fabric resources should not be limited to u24 addressing.

For fabric-level addressing, prefer:

```text
Address Space ID: u32 or u64
Resource ID:      u64
Offset:           u64
Length:           u32 or u64, depending on resource class
```

This allows a W65C816 CPU card, future 32-bit CPU card, future 64-bit CPU card, memory service, storage service, and accelerator service to participate in the same system without forcing all participants through the W65C816 native address model.

## 32-Bit Fields

Use u32 for fields that may exceed 64 KiB but do not need server-scale addressing.

Recommended u32 uses:

- DMA transfer length in v1 descriptors.
- Mailbox payload length for extended messages.
- Image length in boot headers.
- Checksums.
- Sequence numbers.
- Performance counters that may wrap acceptably.
- Fabric register offsets.
- Window sizes up to 4 GiB.
- Future 32-bit CPU local addresses.

A u32 length has a maximum representable value of 4,294,967,295 bytes.

That is adequate for many Rev A through Rev C structures, but it is not enough for future 16 GiB, 512 GiB, or larger memory-service objects.

## 64-Bit Fields

Use u64 for fields that must survive future large-memory and long-running-system requirements.

Recommended u64 uses:

- Capability identifiers.
- Memory object identifiers.
- Large memory object sizes.
- 64-bit timestamps.
- Fault-log monotonic counters.
- Persistent object IDs.
- Large byte offsets.
- Long-running performance counters.
- Future accelerator job IDs.
- Future 64-bit CPU local addresses.
- Fabric resource addresses.

A u64 length has a maximum representable value of 18,446,744,073,709,551,615 bytes.

This is sufficient for the long-term ATARIX goal of memory services that may eventually expose 16 GiB, 512 GiB, or larger memory resources.

## 32-Bit vs 64-Bit Length Policy

ATARIX should not use one length size everywhere.

Different structures should choose lengths based on expected scale and parsing cost.

### Use u16 length fields for small local records

Examples:

```text
Small TLV records
Short names
Small local descriptors
Compact firmware tables
```

### Use u32 length fields for transfer and image sizes

Examples:

```text
DMA transfer length
Boot image length
Mailbox extended payload length
Fabric buffer size
```

### Use u64 length fields for persistent or large-memory resources

Examples:

```text
Memory service object size
File-like object size
Large accelerator buffer size
Large byte offset
Persistent storage extent size
```

## Extended Length Encoding

Where a v1 format begins with a u32 length but may later need larger sizes, the format should reserve an escape value.

Recommended rule:

```text
u32 length != $FFFFFFFF
    length is encoded directly as u32

u32 length == $FFFFFFFF
    actual length follows as u64
```

This allows simple Rev A parsers to handle normal-sized objects while preserving an upgrade path for large objects.

Formats that are expected to represent large memory or storage resources from the beginning should use u64 directly and should not use the escape form.

## Alignment

Architecture-defined structures should prefer natural alignment where practical:

```text
u16 fields aligned to 2-byte boundaries
u32 fields aligned to 4-byte boundaries when practical
u64 fields aligned to 8-byte boundaries when practical
```

However, W65C816 firmware must not assume that external binary structures are naturally aligned unless the structure specification explicitly guarantees it.

## Reserved Fields

All reserved fields must be written as zero and ignored when read unless a later specification defines them.

This rule preserves forward compatibility.

## Versioning

Binary formats must include either:

- An explicit version field, or
- A parent structure that defines the version.

Major version changes may break compatibility.

Minor version changes should be backward-compatible.

Unknown optional fields should be ignored.

Unknown mandatory fields should cause parsing failure.

## Register Widths

Fabric registers may expose 8-bit, 16-bit, 32-bit, or 64-bit values.

If a register wider than the CPU's convenient access width is exposed, the register block must define:

- Access order.
- Snapshot behavior.
- Whether reads are latched.
- Whether writes commit on final byte.
- Whether partial writes are legal.

This is especially important for:

- 64-bit counters.
- Timestamps.
- Large address registers.
- Capability IDs.

## Timestamps

Architecture-level timestamps should use u64.

Preferred timestamp model:

```text
u64 seconds since epoch
```

or, where higher precision is required:

```text
u64 monotonic tick counter
```

The exact timestamp encoding must be specified by the owning subsystem.

The RTC/NTP system should not rely on 32-bit time fields for long-lived operation.

## Capability IDs

Capability identifiers should use u64.

Rationale:

- Avoids short ID exhaustion.
- Supports persistent logging.
- Supports revocation tables.
- Supports future multi-card and multi-service deployments.

## Memory Service Objects

Memory service objects should use u64 identifiers and u64 sizes.

The W65C816 CPU may map these objects through 16 KiB, 32 KiB, or 64 KiB windows, but the object itself may be much larger than the CPU address space.

A future 32-bit or 64-bit CPU card may map larger portions of the same object through a different address aperture without changing the object model.

Example:

```text
Memory Object ID: u64
Object Size:      u64
Window Offset:    u64
Window Length:    u32 or u16, depending on aperture
```

## Design Principle

Use small fields where the domain is naturally small.

Use large fields where future capacity, persistence, timestamps, or external resources are involved.

Do not force all structures into 16-bit fields merely because the first CPU is a W65C816.

Do not treat u24 as the universal ATARIX address width merely because the first CPU is a W65C816.

Do not force all structures into 64-bit fields when a small local structure would be simpler and more efficient.

## Related Documents

```text
docs/discovery-rom-format.md
docs/mailbox-protocol-v1.md
docs/dma-engine-v1.md
docs/capability-model.md
docs/netboot-ntp-v1.md
docs/supervisor-card-v1.md
docs/address-space-architecture.md
```

## Open Questions

- Final timestamp epoch and precision.
- Whether some fabric counters should be 48-bit instead of 64-bit.
- Whether boot images should use u32 length plus u64 extended length or direct u64 length.
- Whether memory service windows should prefer fixed 64 KiB mappings.
- Exact ABI naming convention for signed types, if needed.
- Whether future distributed services require 128-bit object identifiers.


---

---
document: ATX-100-CH20
title: AEMS and Engineering Gates
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-090
  - ATX-SPEC-091
implements:
  - tools/aems/aems.py
  - scripts/engineering/run_gate.sh
  - scripts/engineering/check_aems_docs.sh
verified_by:
  - engineering-gate.yml
---

# Chapter 20: AEMS and Engineering Gates

## Engineering As Architecture

Atarix treats engineering evidence as part of the architecture.

A design is not considered stable merely because it is written down or implemented. It must be linked to requirements, reviewed, tested, and verified by the Engineering Gate.

## AEMS

AEMS is the Atarix Engineering Management System.

AEMS is responsible for making repository state discoverable and auditable. Its initial responsibilities include:

- Document discovery.
- Metadata parsing.
- Documentation graph generation.
- Traceability edge reporting.
- Specification index reporting.
- Engineering Gate support.

## Documentation Graph

The documentation graph links documents through relationships such as:

- canonical ownership.
- dependencies.
- related specifications.
- implementation artifacts.
- verification artifacts.
- supersession and archival history.

## Engineering Gate

The Engineering Gate is the repository-level consistency check.

It validates architecture invariants, public headers, RTL lint, RTL simulations, and documentation graph health.

The gate is expected to grow over time into a full engineering certification system covering source code, RTL, documentation, traceability, release evidence, and performance history.

## Release Evidence

A future Atarix release should include generated AEMS evidence:

- document graph.
- traceability matrix.
- Engineering Gate summary.
- architecture manual edition.
- specification index.
- release history.

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-090-Atarix-Engineering-Management-System.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-090-Atarix-Engineering-Management-System.md -->
### Integrated source: `docs/architecture/ATX-SPEC-090-Atarix-Engineering-Management-System.md`

# ATX-SPEC-090: Atarix Engineering Management System

## Purpose

This specification defines AEMS, the Atarix Engineering Management System.

AEMS manages repository-native engineering data, including specifications, design documents, architecture reviews, issues, implementation references, tests, engineering gates, releases, and generated documentation.

## Initial Scope

AEMS SHALL support:

- Document discovery
- Metadata parsing
- Document lifecycle validation
- Specification registry generation
- Dependency graph validation
- Traceability graph validation
- Engineering Gate integration
- Documentation book building

## Non-Goals For v0.1

AEMS v0.1 does not own GitHub Projects synchronization, release publication, or generated PDF production.

## Relationship To ATX-100

AEMS supports ATX-100 by generating document maps, dependency graphs, traceability matrices, and release evidence appendices.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-091-Requirements-and-Traceability-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-091-Requirements-and-Traceability-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-091-Requirements-and-Traceability-Model.md`

# ATX-SPEC-091: Requirements and Traceability Model

## Purpose

This specification defines how Atarix records and validates traceability across architecture, implementation, verification, review, and release artifacts.

## Traceability Chain

Atarix traceability links:

```text
Requirement
  -> Design Decision
  -> Specification
  -> Architecture Review
  -> Issue
  -> Commit
  -> Implementation
  -> Test or Simulation
  -> Engineering Gate
  -> Release
```

## Initial Rules

- Every normative requirement should have a stable identifier.
- Every implementation artifact should identify the requirement or specification it supports.
- Every verified artifact should link to a test, simulation, or gate result.
- Superseded documents must remain traceable through archive metadata.
- AEMS should validate traceability graph consistency.

## Relationship To ATX-100

ATX-100 presents the stable architectural view. ATX-SPEC-091 defines the evidence chain that connects that view to implementation and release state.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-003-Engineering-Principles.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-003-Engineering-Principles.md -->
### Integrated source: `docs/design/ATX-DESIGN-003-Engineering-Principles.md`

# ATX-DESIGN-003: Engineering Principles

## Purpose

This document defines the engineering principles used to develop Atarix.

## Principle

Engineering is part of the architecture.

Atarix links vision, design, specifications, reviews, issues, implementation, simulation, engineering gates, releases, and maintenance into a single traceable process.

## Initial Process Model

```text
Vision
  -> Design
  -> Specification
  -> Architecture Review
  -> Issue Planning
  -> Implementation
  -> Simulation
  -> Engineering Gate
  -> Release
  -> Maintenance
```

## Relationship To AEMS

AEMS exists to automate and verify these engineering relationships.


---

# Chapter 21: Requirements and Traceability

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/cpu-observability-contract-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/cpu-observability-contract-v1.md -->
### Integrated source: `docs/cpu-observability-contract-v1.md`

# CPU Observability Contract v1

Status: Draft v1

## Purpose

CPU cards are a special class of ATARIX resource.

A CPU card is both a platform participant and a potential point of failure.

Core requirement:

```text
A CPU card shall be diagnosable
when the CPU is not functioning.
```

## Architectural Principles

```text
Measurement must not depend on the thing being measured.
```

```text
CPU observability shall be
architecture-independent at the interface,
architecture-specific at the implementation.
```

```text
Any two views shall be sufficient
to diagnose failure of the third.
```

## Three Views of Reality

### Supervisor View

Provides:

- power state
- thermal state
- clock state
- reset state
- watchdog state
- boot progress
- fault state

### FPGA View

Provides:

- bus activity
- trace capture
- protocol decode
- trigger events
- timing analysis
- fabric interaction visibility

### CPU View

Provides:

- monitor state
- operating system state
- software diagnostics
- runtime telemetry

## Generic CPU State Model

```text
CPU_ABSENT
CPU_POWER_GOOD
CPU_CLOCK_GOOD
CPU_RESET_ASSERTED
CPU_RESET_RELEASED
CPU_FETCHING
CPU_EXECUTING
CPU_ENUMERATED
CPU_MONITOR_RUNNING
CPU_OS_RUNNING
CPU_FAULTED
```

These states shall be presented through a common interface regardless of CPU family.

## Required Supervisor Visibility

The supervisor shall expose:

```text
Power State
Temperature
Clock State
Reset State
Heartbeat
Watchdog State
Boot Milestone
Fault State
Last Fault
Reset Cause
```

## Required FPGA Visibility

The FPGA instrumentation layer shall support:

```text
Bus Activity Detection
Instruction Fetch Detection
Interrupt Activity Detection
DMA Activity Detection
Trigger Events
Trace Capture
Protocol Decode
Timing Analysis
```

The FPGA should be able to determine:

```text
No Bus Activity
Repeated Vector Fetch
Unexpected Reset
Bus Lockup
Missing Clock Activity
```

without CPU cooperation.

## Boot Milestones

Standardized milestones:

```text
0x01 ROM_START
0x02 RAM_OK
0x03 FABRIC_ATTACH
0x04 ENUMERATION_OK
0x05 MONITOR_LOADED
0x06 OS_LOADED
```

Future CPU families shall reuse the same milestone interface.

## Runtime Health

Supervisor-visible runtime metrics:

```text
Heartbeat Counter
Watchdog Counter
Exception Counter
Fault Counter
```

Recommended registers:

```text
CPU_BOOT_MILESTONE
CPU_LAST_FAULT
CPU_STATE
```

## W65C816 Adapter Requirements

Initial implementation shall support observation of:

```text
PHI2
RESB
IRQB
NMIB
ABORTB
RDY
BE
VPA
VDA
MLB
```

and:

```text
A[23:0]
D[7:0]
```

through FPGA instrumentation.

## Engineering Test Points

Recommended minimum test points:

```text
TP_CPUCLK
TP_RESET
TP_IRQ
TP_NMI
TP_ABORT
TP_VCORE
TP_GND
```

## Multi-CPU Support

The interface shall support:

```text
CPU0
CPU1
CPU2
...
```

without requiring CPU-family-specific management software.

## Validation Integration

CPU observability shall support:

- manufacturing validation
- bring-up validation
- recovery mode diagnostics
- field diagnostics
- automated validation profiles

The observability layer must remain useful when:

- the monitor is absent
- the operating system is absent
- the CPU is halted
- the CPU is faulted
- the CPU is held in reset

## Related Documents

- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- cpu-validation-strategy-v1.md
- ADR-THREE-FABRIC-ARCHITECTURE.md
- ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-004-Human-Factors-and-Readability.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-004-Human-Factors-and-Readability.md -->
### Integrated source: `docs/design/ATX-DESIGN-004-Human-Factors-and-Readability.md`

# ATX-DESIGN-004: Human Factors and Readability

## Purpose

This document defines the human-readability requirements that guide Atarix architecture and implementation.

## Principle

Systems that cannot be understood cannot be reliably audited, repaired, or evolved.

Atarix values explicitness, determinism, traceability, and readable projections even when machine-optimized paths are used.

## Initial Requirements

- Optimized structures must provide explainable projections.
- Audit windows must be readable by humans.
- Namespaces, capabilities, services, and objects should expose reviewable state.
- Generated artifacts must identify their sources.
- Compatibility layers must explain native authority mappings.


---

# Chapter 22: Roadmap and Evolution

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/test-write.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/test-write.md -->
### Integrated source: `docs/test-write.md`

Testing GitHub write access.


---
