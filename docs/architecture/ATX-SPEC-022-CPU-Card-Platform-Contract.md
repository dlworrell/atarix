---
document: ATX-SPEC-022
title: CPU Card Platform Contract (Rev A emulation profile)
type: Specification
status: Draft
lifecycle: Draft
version: 0.1
related:
  - ATX-SPEC-005
  - ATX-SPEC-014
  - ATX-SPEC-016
  - ATX-SPEC-021
consolidates:
  - docs/memory-map.md
  - docs/register-map-v1.md
  - docs/interrupt-architecture-v1.md
  - docs/boot-sequence-v1.md
  - docs/boot-firmware-architecture-v1.md
  - docs/cpu-card-architecture-v1.md
  - docs/clock-reset-spec.md
implements:
  - include/atarix/bib.h
  - include/atarix/discovery.h
  - include/atarix/mailbox.h
decided_by:
  - docs/adr/ADR-0016-cpu-card-emulation-with-mame.md
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:7b2a6686-f759-4672-9140-c13d22204468
-->

# ATX-SPEC-022 CPU Card Platform Contract (Rev A emulation profile)

## Status

Draft v0.1. Owner review is pending.

Each value is tagged as one of:

- **[Source]**: taken unchanged from an existing draft, which is named.
- **[Resolved]**: two existing drafts disagree, and this spec picks one. The reason is in [Conflicts resolved](#conflicts-resolved).
- **[Proposed]**: no existing document defines it. The value is new here and needs the owner's decision.

Nothing in this document is accepted until the owner approves it.

## Purpose

The specifications ATX-SPEC-001 to ATX-SPEC-021 define authority, capabilities, mailboxes, services and memory movement. They do not define what the W65C816 actually sees: addresses, registers, reset state, interrupts and the hand-off to an operating system. That information exists, but it is spread across seven draft documents (listed under `consolidates`). Those drafts are staged by board revision (Rev A, B, C), and in places they disagree.

Two things need one definite answer to that question:

1. **An emulated machine.** The ADR-0016 machine model runs the real W65C816 code that will later run on the card.
2. **The operating system.** It is developed inside that emulated machine, and must later run on hardware without changes.

This spec defines one profile, **EP-A** (emulation profile, Rev A). It covers one CPU card attached to a Fabric Northbridge and a supervisor. Firmware, the OS, the emulator and later the RTL are all built to EP-A.

## Scope and doctrine

- **This is a CPU-card contract.** It describes the CPU card's local view, below the public architecture.
  - Capability, authority, mailbox semantics and service models stay with ATX-SPEC-001 to ATX-SPEC-021.
  - No address or register in this document grants authority.
  - The interrupt design rule applies ([interrupt-architecture-v1](../interrupt-architecture-v1.md)): interrupts notify; they do not transfer ownership, grant authority or imply trust.
- **The OS must not depend on emulator-only behaviour.** Anything only an emulator provides is marked **[Emulator only]**, and the OS must not rely on it.
- **Out of scope:** the backplane pinout, multiple CPUs, DMA descriptor operation, network hardware (W5500), RP2350 firmware internals and ECP5 RTL structure. EP-A reserves addresses for some of these but defines no behaviour.

## Conventions

- **Byte order.** Registers are 8 bits wide. A multi-byte value occupies consecutive registers, least significant byte first, as [ADR-0011](../adr/ADR-0011-little-endian-data-model.md) requires. When a value spans registers, reading the low byte latches the high bytes for the next read, so that 16-bit reads stay consistent. **[Proposed]**
- **Offsets.** All register offsets are relative to the block base.
- **Reserved bits.** These read as 0 and are ignored on write.
- **Reserved registers.** These read as `$00`.

## CPU

| Item | EP-A value | Tag |
| --- | --- | --- |
| Processor | WDC W65C816S | [Source] cpu-card-architecture-v1 |
| Reset state | Emulation mode (E=1), as the W65C816S defines | [Source] W65C816S datasheet |
| Clock | 8 MHz nominal. The emulator also supports 1, 4 and 14 MHz. | [Source] cpu-card-architecture-v1, clock modes |
| ABORTB | Not driven in EP-A | [Source] interrupt-architecture-v1 (reserved for later) |
| Vector Pull Rewrite | Not present in EP-A. Interrupt dispatch reads the controller's pending register. | [Proposed] |

## Address map

### Bank `$00`

| Range | Use | Tag |
| --- | --- | --- |
| `$000000-$0000FF` | Direct page (RAM) | [Source] memory-map |
| `$000100-$0001FF` | Default stack (RAM) | [Source] memory-map |
| `$000200-$00BFFF` | Low RAM: monitor workspace and kernel bank-0 data | [Resolved] |
| `$00C000-$00DFFF` | I/O aperture (8 KiB) | [Source] memory-map, register-map-v1 |
| `$00E000-$00FFFF` | Boot ROM window: the top 8 KiB of the boot ROM, including the vectors | [Resolved] |

### Banks `$01` and above

| Range | Use | Tag |
| --- | --- | --- |
| `$010000-$EFFFFF` | CPU-local RAM | [Source] cpu-card-architecture-v1 |
| `$F00000-$F7FFFF` | Fabric and service windows. Unmapped in EP-A. | [Source] cpu-card-architecture-v1 |
| `$F80000-$FBFFFF` | Backplane BIOS aperture. Unmapped in EP-A. | [Source] cpu-card-architecture-v1 |
| `$FC0000-$FEFFFF` | Reserved. Unmapped in EP-A. | [Proposed] |
| `$FF0000-$FFFFFF` | Boot ROM, 64 KiB. Its last 8 KiB is also visible at `$00E000-$00FFFF`. | [Proposed] |

### RAM size

EP-A fills all CPU-local RAM: `$000000-$00BFFF` plus `$010000-$EFFFFF`, just under 15 MiB. The 16 MiB target in cpu-card-architecture-v1 cannot be reached exactly, because the I/O, ROM and fabric windows take part of the 24-bit space.

Firmware must measure RAM size rather than assume it. The emulator can configure a smaller amount (`-ramsize`) to test that. **[Proposed]**

### Unmapped and faulting accesses

| Access | Result | Tag |
| --- | --- | --- |
| Read of an unmapped address | Returns `$FF` | [Proposed] |
| Write to an unmapped address | Ignored | [Proposed] |
| Write to the boot ROM | Ignored | [Proposed] |
| Any of the above | Sets Fabric Fault Status bit 0 ("unassigned access") and records the 24-bit address in the fault-address registers | [Proposed] |

Recording these accesses makes them observable, as trust-nothing requires: they are not silently absorbed.

## I/O aperture

Block bases follow register-map-v1. **[Source]**

| Base | Block | EP-A |
| --- | --- | --- |
| `$00C000` | UART (console) | Present |
| `$00C100` | GPIO | Present (LEDs and switches only) |
| `$00C200` | Supervisor mailbox | Present |
| `$00C300` | Board identity | Present |
| `$00D000` | Fabric control | Present |
| `$00D100` | Interrupt controller | Present |
| `$00D200` | Mailbox engine | Present |
| `$00D300` | DMA engine | Reserved: reads as not present |
| `$00D400` | Timer | Present |
| `$00D500` | Discovery services | Present |
| `$00D600` | Performance counters | Present |

Every block is 256 bytes. Offsets beyond a block's defined registers are reserved.

### UART `$00C000`

Registers +00 DATA, +01 STATUS, +02 CONTROL and +03 BAUD, with these STATUS bits: 0 RX Ready, 1 TX Ready, 2 Framing Error, 3 Overrun. **[Source]** register-map-v1

Additions: **[Proposed]**

- CONTROL bit 0 enables the RX interrupt. CONTROL bit 1 enables the TX-ready interrupt. Both raise interrupt source 0.
- BAUD is a divisor. The emulator ignores it: the console is a byte stream.

### GPIO `$00C100`

**[Proposed]**

| Offset | Register | Access |
| --- | --- | --- |
| +00 | OUT: 8 LEDs | Read/write |
| +01 | IN: 8 switches. Bit 7 is the recovery-request switch. | Read only |

### Supervisor mailbox `$00C200`

COMMAND, STATUS and ARG0 to ARG3 at +00 to +05, with these commands: **[Source]** register-map-v1

| Code | Command |
| --- | --- |
| `$01` | Reset Request |
| `$02` | Recovery Mode |
| `$03` | Read Board ID |
| `$04` | Read Fault Log |
| `$05` | FPGA Reload |

Additions: **[Proposed]**

- **Protocol.**
  1. The CPU writes ARG0 to ARG3.
  2. The CPU writes COMMAND. The write starts the command.
  3. STATUS bit 0 (BUSY) stays set until the supervisor has written its results into ARG0 to ARG3.
  4. STATUS bit 1 (ERROR) reports a refused or unknown command.
- **New command `$06` Read Reset Cause.** It returns a code in ARG0. The codes come from clock-reset-spec:

  | ARG0 | Reset cause |
  | --- | --- |
  | 0 | Unknown |
  | 1 | Power-On |
  | 2 | Manual |
  | 3 | Recovery Request |
  | 4 | Watchdog |
  | 5 | Software |
  | 6 | Fabric Fault |

- **New command `$07` Watchdog Kick.** A watchdog is armed after reset release. If firmware does not kick it within 2 seconds, the supervisor resets the CPU with cause 4 (Watchdog). The emulator can disable the watchdog for debugging (`-nowatchdog`). **[Emulator only]**
- **Interrupt.** Command completion raises interrupt source 1.

### Board identity `$00C300`

Vendor ID, Board ID, Revision and Capability Flags at +00 to +05. **[Source]** register-map-v1

EP-A values: **[Proposed]**

| Field | Value |
| --- | --- |
| Vendor ID | `$0000` (unassigned) |
| Board ID | `$0816` |
| Revision | `$0A` |
| Capability Flags | 0 |

The emulator reports Revision `$EA`, so software can tell it is running emulated. The OS must not change its behaviour based on this value; it is for diagnostics only. **[Emulator only]**

### Fabric control `$00D000`

Registers +00 Version, +01 Status, +02 Control and +03 Fault Status. Control bits: 0 Fabric Services, 1 Discovery, 2 DMA, 3 Diagnostics. **[Source]** register-map-v1

Additions: **[Proposed]**

| Offset | Register |
| --- | --- |
| +04 to +06 | Fault address, 24-bit, little-endian |
| +07 | Staged image status |

- **Fault Status bits.** 0 unassigned access, 1 mailbox frame rejected, 2 discovery read out of range. Writing 1 to a bit clears it. Any set bit raises interrupt source 8.
- **DMA.** Control bit 2 cannot be set in EP-A. It reads as 0, because DMA stays disabled until the capability model authorises a transfer ([boot-sequence-v1](../boot-sequence-v1.md), phase 7).
- **Staged image status (+07).** Bit 0 means a boot image is staged in RAM. The emulator sets it when an image is quickloaded (see [Boot and hand-off](#boot-and-hand-off)). On hardware, netboot staging sets it. **[Proposed]**

### Interrupt controller `$00D100`

The eight-register layout from interrupt-architecture-v1. It replaces the four-register subset in register-map-v1. **[Resolved]**

| Offset | Register |
| --- | --- |
| +00 | Pending, low byte |
| +01 | Pending, high byte |
| +02 | Mask, low byte |
| +03 | Mask, high byte |
| +04 | Priority Select |
| +05 | Acknowledge |
| +06 | Route Target |
| +07 | Status |

Sources (pending-register bits): **[Source]** interrupt-architecture-v1

| Bit | Source |
| --- | --- |
| 0 | UART |
| 1 | Supervisor |
| 2 | Timer |
| 3 | Mailbox |
| 4 | DMA |
| 5 | Network |
| 6 | Accelerator |
| 7 | External Slot |
| 8 | Fabric Fault |
| 9 | Debug Request |

EP-A behaviour: **[Proposed]**

- **IRQB.** IRQB is asserted while any bit of (Pending AND NOT Mask) is set.
- **Status (+07).** Holds the number of the highest-priority unmasked pending source, in the fixed priority order of interrupt-architecture-v1, or `$FF` when none is pending. Handlers dispatch on this register.
- **Acknowledge (+05).** Writing a source number clears that pending bit. A level-triggered source re-asserts it while its condition remains.
- **Priority Select (+04) and Route Target (+06).** These read as 0. They are reserved for programmable priority and for multiple CPUs.
- **Reset state.** All sources are masked, and pending is cleared. **[Source]** interrupt-architecture-v1, boot defaults
- **NMIB.** NMIB is driven only by the supervisor, for watchdog pre-expiry and for the recovery request. **[Source]** interrupt-architecture-v1 (NMI use)

### Mailbox engine `$00D200`

Registers +00 Status, +01 Control, +02 Message Length, +03 Source ID, +04 Destination ID, +05 Message Type. Payload storage is "implementation dependent". **[Source]** register-map-v1

Frames follow ATX-SPEC-005 and `include/atarix/mailbox.h`: a 16-byte header (magic `ATARIX_MAILBOX_MAGIC_V1`, type, length, sequence, CRC-32), then a payload of up to 4096 bytes. The eight-bit length and ID registers cannot describe those frames, so EP-A moves whole frames through data ports. **[Proposed]**

**Registers**

| Offset | Register | Notes |
| --- | --- | --- |
| +00 | Status | Bit 0 TX ready, 1 RX frame available, 2 last frame rejected |
| +01 | Control | Bit 0 SEND, 1 RX DONE, 2 TX RESET, 3 RX interrupt enable |
| +02 to +03 | TX length | 16-bit |
| +04 to +05 | Destination node | 16-bit |
| +06 | TX DATA port | Write only |
| +08 to +09 | RX length | 16-bit, read only |
| +0A to +0B | Source node | 16-bit, read only |
| +0C | RX DATA port | Read only |
| +0E | Reject reason | Read only |

The original fields map onto this layout as follows: Message Length becomes TX/RX length, Source and Destination ID become 16-bit node IDs, and Message Type is the `type` field inside the frame header.

**Sending**

1. Wait until TX ready is set.
2. Write TX length and the destination node.
3. Write the frame's bytes, header first, to TX DATA.
4. Write SEND.

The engine then:

- checks magic, length, CRC and the sequence window, as `src/sim/sim_mailbox.c` does today;
- forwards the frame to the destination node, or rejects it and sets Fault Status bit 1.

The reject reason uses the same codes as the simulator's validation result. That way, the C simulator and the emulated machine fail the same way.

**Receiving**

1. RX frame available raises interrupt source 3 if RX interrupt enable is set.
2. The CPU reads RX length and the source node.
3. The CPU reads the frame from RX DATA.
4. The CPU writes RX DONE.

**Node IDs**

These are the `sim/` node IDs.

| Node | ID |
| --- | --- |
| Supervisor | `$0000` |
| Fabric | `$0001` |
| This CPU card | `$0100` |

### Timer `$00D400`

Registers +00 to +01 Counter, +02 to +03 Compare and +04 Control. **[Source]** register-map-v1

EP-A behaviour: **[Proposed]**

- **Counter.** A 16-bit up-counter at 1 MHz. It is independent of the CPU clock, so tick lengths do not change when the clock mode does.
- **Control bits.**
  - Bit 0 enables counting.
  - Bit 1 enables the compare interrupt.
  - Bit 2 is auto-reload: on a compare match, the counter returns to 0.
- **Compare match.** A match sets pending source 2.

A 1 MHz counter with auto-reload gives scheduler ticks from 1 µs to 65.5 ms.

### Discovery services `$00D500`

Registers +00 Device Count, +01 Selected Device, +02 to +05 Vendor ID and Device ID, and +06 Capability Pointer. **[Source]** register-map-v1

To read the full discovery image, firmware also needs the image itself. **[Proposed]**

| Offset | Register |
| --- | --- |
| +08 to +0A | Image offset, 24-bit |
| +0B | Image DATA port, which auto-increments the offset |
| +0C to +0E | Image length |

- **Image format.** The image is ATDX: magic `ATARIX_DISCOVERY_MAGIC_ATDX`, a 32-byte header and 8-byte-aligned records, as `include/atarix/discovery.h` defines. Firmware parses it with the same rules as `src/discovery/`.
- **Image source.** The emulator builds the image with `tools/mkdiscovery`, so the emulated machine and the host tools share one source.
- **Out-of-range reads.** A read past the end of the image returns `$FF` and sets Fault Status bit 2.

### Performance counters `$00D600`

Registers +00 Cycle Counter, +02 Interrupt Count, +03 DMA Count and +04 Mailbox Count. **[Source]** register-map-v1

EP-A widens these to 32 bits, little-endian: **[Proposed]**

| Offset | Counter |
| --- | --- |
| +00 to +03 | Cycles |
| +04 to +07 | Interrupts taken |
| +08 to +0B | DMA operations (always 0 in EP-A) |
| +0C to +0F | Mailbox frames |

Reading +00 latches all four counters.

## Boot and hand-off

### Reset

The supervisor holds the CPU in reset until the fabric is ready, then releases it. **[Source]** clock-reset-spec, boot-sequence-v1

In EP-A, the emulated supervisor releases reset at once, with reset cause 1 (Power-On). After a supervisor Reset Request, the cause is 5 (Software). **[Proposed]**

### Boot ROM

The CPU fetches its reset vector from `$00FFFC`, in the boot ROM window. The ROM monitor then runs boot-sequence-v1 phases 4 to 8 **[Source]**:

1. Enter native mode.
2. Set the stack and direct page.
3. Set up the UART, and print the banner fields (ROM version, reset reason, CPU mode, detected RAM).
4. Run the diagnostics.
5. Scan the discovery image.
6. Start services.
7. Select the next stage.

### Staged images

The kernel image format is `struct atarix_kernel_header`, from boot-firmware-architecture-v1. **[Source]**

A kernel image is staged by one of two paths:

- **Serial upload.** The monitor receives it over the UART. **[Source]** boot-sequence-v1, serial loader
- **Quickload.** The emulator writes the image into RAM at the header's `load_addr` and sets staged image status bit 0. **[Emulator only]**

For the OS, both paths are the same: the monitor finds a staged image, checks its header and checksum, builds the BIB and hands off. Quickload does not bypass the firmware.

### Kernel entry state

The monitor hands off to the kernel in this state: **[Proposed]**

| Item | State at kernel entry |
| --- | --- |
| CPU mode | Native (E=0), with 16-bit accumulator and index registers (m=0, x=0) |
| Interrupts | I=1 (disabled). All controller sources masked. |
| D | `$0000` |
| DB | `$00` |
| S | Within `$000100-$0001FF`. The kernel replaces it. |
| A | BIB address, bits 0 to 15 |
| X | BIB address, bits 16 to 23 |
| PC | `entry_addr` from the kernel header |
| BIB | An `atarix_bib_header_v1` (64-byte header, magic `ATARXBIB`, CRC-32C), built in RAM by the monitor. It includes the memory map reference, the console service identity, the supervisor service identity and the boot image identity TLVs. |
| Fabric | Services and discovery enabled. DMA disabled. |

## Emulator obligations

The ADR-0016 machine driver must implement everything above that is tagged **[Source]**, **[Resolved]** or **[Proposed]**. It must also:

- **Console.** Expose the UART as the machine's console, as a byte stream on standard I/O or a socket.
- **Quickload.** Load an Atarix kernel image by quickload, and a boot ROM image by ROM option.
- **Discovery image.** Build the discovery image with `tools/mkdiscovery`.
- **Mailbox checks.** Share the mailbox frame validation rules with `src/sim/`, so that the C simulator, the emulated machine and later the RTL reject the same frames.
- **Exposure.** Not expose emulator-only behaviour other than the items tagged **[Emulator only]**.

## Conformance

EP-A is met when one ROM monitor image and one test kernel pass all of these checks, with no changes between the emulator and later hardware:

1. **Boot banner.** The ROM boots and prints the boot-sequence-v1 banner, including the reset cause and the detected RAM.
2. **Unmapped access.** An unmapped read returns `$FF` and sets Fabric Fault Status bit 0.
3. **Timer interrupt.** The timer raises interrupt source 2 at the programmed rate. Acknowledging it clears the pending bit.
4. **Supervisor and watchdog.** The supervisor's Read Board ID and Read Reset Cause commands return the EP-A values. A missed watchdog kick resets the CPU with cause 4.
5. **Mailbox round trip.** A mailbox echo frame to the fabric node returns. A frame with a bad CRC is rejected, with the same reason code as `src/sim/`.
6. **Discovery.** The discovery image read through the data port matches the `tools/mkdiscovery` output byte for byte.
7. **Hand-off.** A quickloaded test kernel receives control in the kernel entry state above. Its BIB passes `include/atarix/bib_reader.h` validation.

These checks run headless in CI under the emulator. They become evidence under [ADR-0014](../adr/ADR-0014-testing-and-verification-strategy.md).

## Conflicts resolved

| Topic | Disagreement | EP-A choice | Why |
| --- | --- | --- | --- |
| UART block size | memory-map gives `$00C000-$00C00F`. register-map-v1 gives `$00C000-$00C0FF`. | 256-byte block | register-map-v1 is the later "first stable contract", and every block in it is 256 bytes. |
| Interrupt controller | register-map-v1 has 4 registers and 8 sources. interrupt-architecture-v1 has 8 registers and 10 sources. | 8 registers, 10 sources | interrupt-architecture-v1 itself calls the 4-register draft "the minimal compatibility subset". |
| Low RAM end | memory-map ends low RAM at `$007FFF`, with an expansion window at `$008000-$00BFFF`. cpu-card-architecture-v1 makes `$000200-$00BFFF` RAM. | RAM to `$00BFFF` | The expansion window has no defined user. Keeping it as RAM maximises bank-0 space for the kernel. |
| ROM location | memory-map puts ROM at `$00E000-$00FFFF`. cpu-card-architecture-v1 adds a recovery ROM region at `$FC0000-$FFFFFF`. | 64 KiB at `$FF0000`, top 8 KiB mirrored to bank 0 | Vectors must be in bank 0. A larger ROM in high memory leaves space for the monitor and diagnostics. |
| Revision staging | memory-map stages the supervisor mailbox and identity in Rev B, and the FPGA blocks in Rev C. | All present in EP-A | Emulation has no board cost. The OS needs the full register set now. Hardware revisions may implement a subset and report it through discovery. |

## Open questions for the owner

1. **Register widths.** Mailbox, discovery and the performance counters use wider registers than register-map-v1 sketched. Should register-map-v1 be updated to match, or retired in favour of this spec?
2. **Kernel entry state.** The 16-bit accumulator and index mode, and BIB pointer in A/X, is a proposal. Is another calling convention preferred (for example, a fixed BIB address)?
3. **Watchdog.** Are 2 s and the `$07` kick command right for the supervisor watchdog in EP-A?
4. **Vendor ID.** Is there an Atarix vendor ID to use in place of `$0000`?
5. **Version status.** Should EP-A become Rev A's hardware target as well, or stay an emulation-first profile that hardware revisions converge on?
