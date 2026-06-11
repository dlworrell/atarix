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
