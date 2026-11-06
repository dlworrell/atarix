<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:7360adae-81f1-4125-8680-7b41e701db53
-->

# Rev A Hardware in Simulation

Status: Active (2026-11-06)

## Purpose

Before any ATARIX board exists, the Rev A hardware was built in software and
tested: the CPU card, the Fabric Northbridge FPGA, the supervisor, the
display card and the cards around them. This page is the hardware-side record
of that build. It tells you, part by part, what each simulated piece stands
for on the real board, where its design came from, what has been proven, and
what you still have to build before the part exists in copper.

Read this if you are designing a board, writing the fabric RTL or supervisor
firmware, or planning bench bring-up. For how the software bench works and
what each test checks, see the
[CPU card software test bench](../verification/cpu-card-software-testbench.md).
For specification gaps the build exposed, see
[Architecture Review 004](../reviews/Architecture-Review-004.md).

## Evidence levels

Every claim below carries the evidence level it reached, from the ladder in
the [closure development cycle](../engineering/closure-development-cycle.md):

| Level | Meaning here |
|---|---|
| E2 | Matches a reference model or emulator |
| E3 | RTL simulation, formal proof, synthesis or firmware on an emulated core |
| E5 | Measured on real hardware |

Nothing in this page has reached E5. No board has been built.

## The board, part by part

| Real part | Software stand-in | Design source | Proven | Level | Still needed for hardware |
|---|---|---|---|---|---|
| W65C816S CPU | Vendored CLK core, cycle-accurate, VDA/VPA/VPB/RWB/E/M/X/ML driven every cycle (`third_party/clk`) | `dlworrell/CLK` `0f49993` | 5,080,400 of 5,080,400 single-instruction vectors pass, including every bus cycle and pin | E2 | A board; measured timing at 1, 4, 8 and 14 MHz |
| CPU card bus, RAM, ROM, UART, identity block | `sim/cpu-card/cpu_card.cpp` on the Rev A map | `memory-map.md`, `register-map-v1.md` | Boot from the reset vector into native mode; RAM in banks `$00`, `$01`, `$7F`; ROM write protection; every unmapped access is a fault | E2 | Schematic, address decode, ROM and SRAM parts |
| Fabric Northbridge bus capture (ECP5) | `rtl/atarix/atx_northbridge_bus_shim.v` | ATX-SPEC-020 scaffold, rewritten | Formally proven: no write lost, duplicated or corrupted, delivery within 3 clocks, for f_clk ≥ 4 × f_PHI2. Matches every write the CPU makes at datasheet timing (10 ns hold) | E3 | Pin constraints, PHI2 on a clock-capable pin, address decode feeding `cs_n` |
| Fabric Northbridge mailbox TX framing (ECP5) | `rtl/atarix/atx_mailbox_tx_assembler.v` | Framing rule of the C model (frame on SUBMIT) | All 21 frames match the C model and its audit at functional and datasheet timing | E3 | RX path, Mailbox Engine registers, and validation in RTL (today these exist only in the C model) |
| ECP5 resources | `scripts/rtl/run_ecp5_fit.sh` (yosys, nextpnr-ecp5) | ULX3S LFE5U-85F-6BG381C, ADR-0001 | Fits and meets the 25 MHz board clock out of context (table below) | E3 | A real pinout; a fit with I/O and PLL in context |
| Fabric Mailbox Engine, Discovery Services, interrupt controller | `sim/cpu-card/northbridge.cpp`, `interrupts.cpp` | `register-map-v1.md`, `interrupt-architecture-v1.md`, Vega816 | Register behavior, Vector Pull Rewrite, ring-filtered service directory, mailbox validation through production `libatarix` | E2 | RTL for all three |
| Supervisor RP2350 | `sim/cpu-card/supervisor.cpp` | ADR-0002, ATX-SPEC-016, `identity-eeprom-v1.md` | Boot authority chain with the CPU held in reset; Supervisor Request Gateway with ATX-SPEC-016 results; supervisor audit | E2 | Supervisor firmware, reset and clock control wiring, identity EEPROM bus |
| Card Identity EEPROMs | `atarix_identity_validate_v1()` on 512-byte v1 records | `identity-eeprom-v1.md` | Header, CRC and identity checks; a one-bit corruption is caught and the card quarantined | E2 | EEPROM parts, I2C wiring, provisioning tool |
| Display card RP2350 (GPU compute) | `src/firmware/display-card/gpu_core.c` | ADR-0016, `dlworrell/cglm` `58d8c15` | Single-precision only (103 FPU instructions, no double); all cases pass on an emulated Cortex-M33 at 0 LSB from independent values | E3 | RP2350 SDK build, signed firmware image, mailbox transport to the fabric, a board |
| Backplane | Not modelled | `dlworrell/BB816-MATX-PCIE` | Nothing | none | Decide the connector; `vega816-analysis.md` says not to keep the BB816 bus as the ATARIX interface |
| Storage and test cards | Mailbox endpoints with identities | Bench only | Used to test policy: quarantine, homebrew trust, a SUSPENDED service | E2 | Real cards |

## Hardware repositories used

| Repository | Pinned | What the build took from it |
|---|---|---|
| `dlworrell/CLK` | `0f49993` | The W65C816 and 6502-family cores, vendored as `third_party/clk` |
| `SingleStepTests/65816` | `307c63f` | Instruction vectors for the W65C816 (downloaded, not vendored) |
| `dlworrell/65x02` | `66ea3ef` | 6502 and 65C02 vectors, used to self-check the vector runner |
| `dlworrell/Vega816` | `8dfb370` (as read) | The Vector Pull Rewrite rule and the seven-level IRQ vector layout |
| `dlworrell/ulx3s` | `14cac31` | The target FPGA (LFE5U-85F, CABGA381) and its 25 MHz oscillator |
| `dlworrell/cglm` | `58d8c15` | The display card's math library, vendored as headers in `third_party/cglm` |
| `dlworrell/BB816-MATX-PCIE` | `afcb666` (as read) | Reviewed, not modelled |

## Hardware requirements the build found

These are requirements on the real boards. Each one comes from a result, not
from preference.

### CPU-to-fabric clocking

The W65C816 holds write data and address for only about 10 ns after PHI2
falls. The original shim sampled them from the FPGA clock, up to 40 ns late
at 25 MHz, and lost or corrupted most writes at datasheet timing.

The shim now registers address, data, chip select and R/W **on the PHI2
falling edge** and passes each write to the FPGA clock through a toggle and a
two-flop synchronizer. That gives three board requirements:

1. **PHI2 must reach the ECP5 on a clock-capable input pin.** The capture
   register is clocked by PHI2 itself.
2. **The FPGA clock must be at least 4 × PHI2** (each PHI2 phase at least two
   FPGA clock periods, at 50% duty). This bound is formally proven, and 3:1
   gives a counterexample, so it is the true minimum.
3. **8 MHz and 14 MHz need an ECP5 PLL** above the ULX3S 25 MHz oscillator.

| PHI2 (`rev-a-hardware-baseline.md`) | Minimum FPGA clock | On the ULX3S | Verified |
|---|---|---|---|
| 1 MHz debug | 4 MHz | 25 MHz oscillator | by the proof |
| 4 MHz bring-up | 16 MHz | 25 MHz oscillator | co-simulation at datasheet timing |
| 8 MHz conservative | 32 MHz | PLL needed | by the proof |
| 14 MHz target | 56 MHz | PLL needed | co-simulation at 62.5 MHz; 14 MHz on 25 MHz fails, as expected |

The proof treats time in steps and does not model metastability; the
two-flop synchronizer is the standard protection for that.

### Signals the fabric needs from the CPU card

For the mailbox block, the shim uses PHI2, RWB, D0–D7, A0–A7 and a chip
select for the `$00D2xx` page. Something must decode bank `$00`, page `$D2`
into that chip select; the shim does not. Vector Pull Rewrite also needs VPB
and the IRQ level from the interrupt controller, and drives A1–A4 during a
vector fetch. Interrupts need IRQB driven from the fabric. All of these are
in the CPU signal list of `cpu-card-architecture-v1.md`; the board must route
them to the fabric, not only to diagnostics headers.

### ECP5 resource use

Placed and routed out of context on the LFE5U-85F-6BG381C, 25 MHz target:

| Module | Fmax | LUT4 (of 83,640) | Flip-flops | DSP |
|---|---|---|---|---|
| `atx_northbridge_bus_shim` | 365 MHz | 41 | 37 | 0 |
| `atx_mailbox_tx_assembler` | 108 MHz (56 MHz needed for 14 MHz PHI2) | 1,509 | 1,051 | 0 |
| `atx_spec_020_system_wrapper` | 76.7 MHz | 726 | 188 | 1 of 156 |

The assembler keeps both 64-byte buffers in registers because it copies the
window in one cycle. A block-RAM version is possible if area matters.

### Interrupts and vectors

- Vector Pull Rewrite rewrites only in native mode, and the dispatcher never
  emits level 7 in native mode: level 7 would fetch the RESET vector.
- Native IRQ levels 3–6 fetch `$FFF4`–`$FFFA`, which are also the
  emulation-mode COP, ABORT and NMI vectors (as on Vega816). An NMI or ABORT
  taken before the switch to native mode would run an IRQ handler.
- The interrupt controller register layout differs between
  `register-map-v1.md` and `interrupt-architecture-v1.md`. The model uses the
  "preferred v1" layout. Decide before writing RTL.

### Supervisor and cards

- The supervisor must be able to hold the CPU card in reset until identity
  validation, enumeration, capability evaluation, binding and transport
  activation are done. This matches phases 2 to 6 of `clock-reset-spec.md`.
- The supervisor must read every card's Identity EEPROM before release.
- The CPU reaches the supervisor only through the `$00C200` request gateway.
  No CPU-writable signal may drive reset, power, the watchdog or FPGA reload
  directly (ATX-SPEC-016).

### Display card

- The GPU service runs on the display card's **own** RP2350, on its
  Cortex-M33 cores. The Hazard3 RISC-V cores have no FPU.
- Its firmware lives in QSPI flash. It behaves as ROM only with a signed,
  write-protected image (RP2350 secure boot), which is not yet set up.
- The 48-byte mailbox payload window limits a single request. Matrix uploads
  or batches need a bulk or DMA path.

## Relation to the hardware bench plan

The [hardware bench validation plan](bench-validation-plan-v1.md) brings up
the supervisor and fabric before the CPU card. The software build covers the
same ground first:

| Bench plan phase | Software equivalent today |
|---|---|
| 1 Supervisor bring-up | Supervisor model: boot chain, gateway, audit (no RP2350 firmware) |
| 2 Fabric bring-up | Bus shim and TX assembler RTL, formal proof, ECP5 fit |
| 3 Supervisor–fabric mailbox | Not modelled: the bench's supervisor and fabric talk through C++ calls |
| 4 Discovery hardware | Discovery Services model with ring filtering |
| 5 Capability policy hardware | Policy-granted capabilities; 17 injected defects, all detected |
| 6 First CPU card | Emulated W65C816 runs the test ROM: reset hold and release, vector fetch, fabric service requests |

## What is not built yet

In rough order of what blocks a first board:

1. Board pinout for the Northbridge on the ULX3S GPIO, with PHI2 on a
   clock-capable pin, and a constraints file.
2. ECP5 PLL configuration for 8 and 14 MHz operation.
3. Fabric RTL for the Mailbox Engine registers and RX path, Discovery
   Services and the interrupt controller with Vector Pull Rewrite.
4. Supervisor RP2350 firmware: reset hold, identity reads, the request
   gateway, the supervisor–fabric link (bench plan phase 3).
5. Display card RP2350 SDK build, signed image and mailbox transport.
6. The backplane connector decision.
7. Captures from real hardware (E5) for every claim above.

## Reproduce the results

From the repository root:

```sh
bash scripts/sim/run_testbench.sh         # CPU card, negative controls, RTL co-simulation at datasheet timing
bash scripts/rtl/run_shim_formal.sh       # bus shim proof and its counterexamples (yosys)
bash scripts/rtl/run_ecp5_fit.sh          # ECP5 fit and timing (yosys, nextpnr-ecp5)
bash scripts/firmware/run_gpu_core_m33.sh # display card GPU core on an emulated Cortex-M33 (QEMU)
bash scripts/sim/run_cpu_vectors.sh       # W65C816 instruction vectors (downloads about 220 MB)
```

The Engineering Gate (`scripts/engineering/run_gate.sh`) runs all but the last
on every push.
