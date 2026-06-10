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