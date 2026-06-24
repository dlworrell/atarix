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