<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:62dd9700-ba38-4ddb-8b72-58cea38436df
-->

# ADR-0016: Emulate the CPU Card with a MAME Machine Driver

Status: Proposed
Date: 2026-11-07
Owner: ATARIX
Acceptance: remains Proposed after the pull request that adds it is merged (owner decision, 2026-11-07). Acceptance is a separate owner decision, expected once milestone M4 shows an OS kernel booting under MAME.

## Context

The owner's goal is to develop the Atarix operating system inside an emulated Atarix machine before the hardware exists, and to run the same code on the hardware later. The owner first looked at QEMU, and noted being unsure whether Atarix should write its own emulator.

The repository has some emulation pieces, but none that runs W65C816 code:

- **C simulator.** `src/sim/` models two mailbox nodes: the supervisor (`$0000`) and the fabric (`$0001`).
- **Emulator plan.** `src/emulator/{fabric,discovery,mailbox}` has READMEs only.
- **CPU tests.** [cpu-test-suite-integration-v1](../cpu-test-suite-integration-v1.md) plans to validate an "emulator CPU core" against JSON tests from the `65x02` suite.

The CPU-visible machine was also undefined: the address map, registers, reset and kernel hand-off were spread across several draft documents that partly disagree. [ATX-SPEC-022](../architecture/ATX-SPEC-022-CPU-Card-Platform-Contract.md) now defines that machine as the EP-A profile. This ADR chooses what runs it.

## Decision

1. **MAME runs the CPU card.** Atarix uses MAME's existing 65C816 CPU core (`g65816`, the core behind MAME's SNES and Apple IIgs drivers) and adds one **Atarix machine driver**. The driver implements the EP-A memory map and the I/O blocks of ATX-SPEC-022:
   - UART console
   - GPIO
   - supervisor mailbox
   - board identity
   - fabric control
   - interrupt controller
   - mailbox engine
   - timer
   - discovery services
   - performance counters
2. **Atarix does not write a CPU emulator.** The W65C816 core is the hardest part to get exactly right: emulation and native modes, 8- and 16-bit register widths, banks and timing. Mature cores already exist, and writing another would delay the OS by months.
3. **The supervisor and Northbridge are behavioural models in the driver.** The RP2350 and the ECP5 are modelled by the registers and behaviour ATX-SPEC-022 defines. They are not emulated as an RP2350 or as ECP5 RTL. Their RTL and firmware are verified separately, under [ADR-0014](ADR-0014-testing-and-verification-strategy.md).
4. **One set of rules, three implementations.** The driver must reject mailbox frames and parse discovery images with the same rules as the Atarix C code:
   - mailbox frame validation from `src/sim/`;
   - discovery images built by `tools/mkdiscovery`;
   - BIB validation from `include/atarix/bib_reader.h`.

   It should share that code where it can, and use the same test vectors where it cannot. The C simulator, the emulated machine and the RTL then fail in the same ways.
5. **The OS is a separate deliverable.** The machine driver and the OS are delivered and reviewed separately:
   - The OS targets ATX-SPEC-022. It does not target MAME.
   - Behaviour only the emulator provides is tagged **[Emulator only]** in the spec. The OS must not depend on it.
6. **Conformance runs headless in CI.** The seven ATX-SPEC-022 conformance checks run with MAME's display switched off and a time limit, driven by a Lua script. They are evidence under ADR-0014.

## Evidence

- **QEMU.** Upstream QEMU has no 65C816 (or 6502-family) target and no RP2350 machine. Using QEMU would mean writing and maintaining a new TCG front end for a 16-bit CPU with banked 24-bit addressing. That is more work than a machine driver, and it would sit outside QEMU's maintained targets.
- **MAME's core is mature.** MAME's `g65816` core is in daily use by emulated SNES and Apple IIgs software. Both machines exercise native mode, 16-bit registers and bank switching. A new Atarix driver needs only memory map handlers and device models.
- **The test suite can check the core.** The core can be checked against the instruction-level tests that cpu-test-suite-integration-v1 plans, so CPU correctness is measured, not assumed.

## Alternatives Considered

- **A new QEMU target and machine.** Rejected for the reasons under Evidence.
- **An Atarix-written emulator.** This would mean extending `src/emulator/` with a new W65C816 core. Atarix would own the code end to end, but the core would need years of correctness work that already exists elsewhere.

  The `src/emulator/` fabric, discovery and mailbox models stay useful as host-side reference models and test oracles.
- **A standalone 65816 library with a custom harness.** This needs less code than QEMU. But Atarix would still have to write the debugger, the scheduling and timing, the console plumbing and the image loading that MAME already provides.
- **Hardware only.** Rejected: the OS would wait for boards.

## Consequences

- **Firmware and the OS can start now.** Both are developed against ATX-SPEC-022 inside MAME, with its debugger: breakpoints, watchpoints, trace and memory views.
- **Two artefacts to maintain.** The driver and the spec must stay in step. A spec change needs a driver change and a conformance run in the same promotion.
- **Behavioural models can drift from the RTL.** The supervisor and fabric models can diverge from the real RP2350 firmware and ECP5 RTL. The shared validation rules and test vectors (Decision 4) limit that. RTL co-simulation, for example with Verilator, is a possible later step.
- **Licensing needs an owner decision before driver code lands.**
  - MAME is distributed as a whole under GPL-2.0-or-later, and most of its source files are BSD-3-Clause.
  - Atarix is Apache-2.0. Apache-2.0 code can be combined with GPL-3.0 but not GPL-2.0-only.
  - Options:
    - Keep the driver in an Atarix fork of MAME, under BSD-3-Clause, to match MAME's files. This also allows an upstream submission.
    - Keep it under Apache-2.0 out of tree, and build the combination only under GPL-3.0.
- **Where the driver lives.** This ADR does not decide whether the driver is in a MAME fork, a separate Catalyst repository or this repository. If it is a new repository, it is admitted under the Catylist governed-scope rules (Catylist ADR-0004).

## Delivery plan

| Milestone | Delivers | Conformance check |
| --- | --- | --- |
| M0 | Driver skeleton: memory map, boot ROM, UART console. A minimal ROM prints a banner. | 1 (banner only) |
| M1 | Unmapped-access fault, interrupt controller, timer | 2, 3 |
| M2 | Supervisor mailbox, reset cause, watchdog, board identity | 4 |
| M3 | Mailbox engine and discovery services, with `src/sim/` and `tools/mkdiscovery` parity | 5, 6 |
| M4 | Quickload, monitor hand-off and BIB. OS development starts here. | 7 |
| M5 | Headless conformance in CI, as evidence under ADR-0014 | all |

## Future Revisions

Revisit this ADR in any of these cases:

- ATX-SPEC-022 grows to multiple CPUs or DMA, and the behavioural models no longer represent the fabric well enough.
- RTL co-simulation becomes the better reference.
- A maintained QEMU 65C816 target appears.
