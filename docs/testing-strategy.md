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