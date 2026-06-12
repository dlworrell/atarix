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
