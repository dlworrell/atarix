<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:430fb6d0-c3d2-40dd-849d-d8f1c1061ec9
-->

# ADR-0016: Display Card GPU Compute on a Dedicated RP2350

Status: Accepted

Date: 2026-11-06

Decision owner: Donovan Worrell (choice made in the development session that
produced this record; takes effect when merged).

## Context

The display card needs floating-point geometry (rotation, translation,
projection) for its `SERVICE_GPU_COMPUTE` (`0x6000`). The intended code is
cglm, a single-precision C library ("video chip ROM").

The W65C816 has no floating-point hardware and no practical C `float` support,
so the math must run on the display card's own processor. The CPU card
reaches it as a fabric service through the mailbox, with ring and capability
checks like any other service.

Candidates were a soft RISC-V core with floating point in the ECP5, and an
RP2350.

## Decision

Each display card carries its **own** RP2350. It runs the GPU compute firmware
on its **Cortex-M33** cores, using their single-precision FPU (FPv5-SP).

1. This is not the supervisor RP2350 of ADR-0002. The display card's RP2350
   runs at service ring and has no supervisor authority. The supervisor keeps
   its minimal, always-available role.
2. The firmware runs on the Cortex-M33 cores, not the RP2350's Hazard3 RISC-V
   cores, which have no FPU.
3. The service core is portable C11, single-precision only
   (`src/firmware/display-card/gpu_core.c`). The CPU card simulator links the
   same source, so the bench tests the code the card runs.
4. "ROM" is realized as a write-protected, signed firmware image. The
   RP2350's on-chip ROM is only its boot loader; application firmware lives in
   external QSPI flash. Secure boot (signed images) is the intended way to make
   that image behave as ROM; this is not yet implemented.
5. The CPU card sees only the mailbox service (`GPU_TRANSFORM_REQUEST` /
   `GPU_TRANSFORM_REPLY`, 16.16 fixed point). Nothing about the RP2350 is
   visible in the public interface.

## Evidence

From `scripts/firmware/run_gpu_core_m33.sh` (2026-11-06):

- The core builds for `cortex-m33` with hard float and `fpv5-sp-d16`, with
  `-Werror` and `-Wdouble-promotion`.
- Its object code contains 103 single-precision FPU instructions and no
  double-precision arithmetic: no soft-float double calls and no `.f64`
  instructions. A probe that does use `double` is flagged, as a negative
  control.
- On QEMU `mps2-an505` (the same Armv8-M Mainline core and FPv5-SP FPU), all 6
  cases pass: two transforms and four error paths.
- The transform results equal values computed independently in Python double
  precision to **0 LSB**, bit-identical to the simulator's x86 SSE build. A
  3 LSB error is rejected, as a negative control.
- In the CPU card simulator, the W65C816 test ROM reaches the same core
  through the mailbox (T31–T35). A second GPU without a capability is denied.

QEMU is not an RP2350: no RP2350 peripherals, boot ROM, clocks or timing.
This evidence covers the instruction set, FPU arithmetic and code, not the
board (closure evidence E2/E3, not E5).

## Alternatives Considered

- **Soft RISC-V core with floating point in the ECP5.** Rejected for now. It
  takes area and timing from the Fabric Northbridge FPGA and couples display
  firmware to the fabric bitstream.
- **The supervisor RP2350.** Rejected. It would mix service work with the
  supervisor's authority and availability role (ADR-0002, ring separation).
- **The W65C816 with fixed-point software.** Rejected for general geometry: it
  is slow, and cglm cannot be used.
- **Dedicated GPU silicon.** Out of scope for Rev A and B.

## Consequences

Pros:

- One more instance of a part ATARIX already uses (toolchain, debug, supply).
- Hardware single-precision FPU; cglm's scalar path runs unchanged.
- The display card fails independently of the CPU card and the fabric.
- The simulator and the card share one tested core.

Cons:

- Another firmware image, with its own update and signing path.
- The 48-byte mailbox TX window limits single-request payloads. Larger
  operations (matrix upload, batches) need a bulk or DMA path.
- RP2350-specific startup, clocks, QSPI and secure boot are not yet built or
  tested. The current tests use a QEMU board and newlib semihosting.

Follow-up work:

- RP2350 SDK build of the core, and a measured cycle count on hardware.
- Signed-image policy for the display card firmware.
- Mailbox transport between the display card RP2350 and the fabric.

## Future Revisions

A later display card may move the service to FPGA logic or dedicated
hardware. The mailbox service contract (`SERVICE_GPU_COMPUTE`) should remain
stable.
