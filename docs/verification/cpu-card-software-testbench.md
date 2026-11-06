<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:1fd4c3fb-e188-4d63-96fa-93c2d6040281
-->

# CPU Card Software Test Bench

Status: Active (draft simulator profile)
Owner: ATARIX
Scope: Software-only validation of the Rev A CPU card, the Fabric Northbridge
mailbox path, and the bus shim RTL, before physical hardware exists.

## Purpose

Run the ATARIX hardware/software contract as a test in software first:

```text
65816 test ROM (ca65)
    -> W65C816 core (vendored CLK, cycle-accurate)
    -> emulated Rev A CPU card bus (memory map, UART, identity)
    -> Fabric Northbridge Mailbox Engine model
    -> libatarix mailbox, ring and capability code (production C)
    -> recorded bus trace
    -> atx_northbridge_bus_shim.v under Icarus Verilog
```

The simulator follows the doctrine in [`sim/README.md`](../../sim/README.md):
simulator security equals hardware security, every message traverses the
mailbox validator, failures are observable, and denial is the default.

## One-command validation

```sh
bash scripts/sim/run_testbench.sh            # offline: build, card test, negative controls, RTL co-sim
bash scripts/sim/run_cpu_vectors.sh          # pinned CPU vector sample (downloads ~220 MB)
bash scripts/sim/run_cpu_vectors.sh --full   # every opcode file (several GB)
bash scripts/rtl/run_ecp5_fit.sh             # ULX3S LFE5U-85F fit and timing (yosys + nextpnr-ecp5)
bash scripts/rtl/run_shim_formal.sh          # bus shim formal proof and its controls (yosys only)
bash scripts/engineering/run_gate.sh         # includes the offline test bench and the ECP5 fit as gate phases
```

Tools: a C11 and C++20 compiler, `cc65` (ca65/ld65), Icarus Verilog, Python 3,
and for the fit check yosys and nextpnr-ecp5.

## Components

| Path | What it is |
|---|---|
| `third_party/clk/` | Vendored CLK W65C816 and 6502-family cores, pinned (MIT). See its README. |
| `sim/cpu/vector_runner.cpp` | Replays single-instruction vectors; compares registers, RAM and every bus cycle including VDA/VPA/VPB/RWB/E/M/X/ML. |
| `sim/cpu/sample-opcodes.txt` | Opcodes run on every pull request. |
| `sim/cpu/known-deviations/` | Recorded, explained differences; never silent. |
| `sim/cpu-card/cpu_card.cpp` | Rev A CPU card: memory map, UART, board identity, faults, bus trace. |
| `sim/cpu-card/northbridge.cpp` | Mailbox Engine register block calling libatarix. |
| `sim/cpu-card/interrupts.cpp` | Interrupt controller, Vega816 Vector Pull Rewrite, supervisor register mailbox. |
| `scripts/rtl/run_ecp5_fit.sh` | Synthesis and place-and-route on the ULX3S ECP5-85F. |
| `sim/cpu-card/firmware/test_rom.s` | 65816 test ROM: boot, self-checks, mailbox cases, CRC32 in firmware. |
| `sim/rtl-cosim/` | Replays the CPU bus trace into the bus shim RTL. |
| `scripts/sim/` | Fetch, run and re-vendor scripts. |

## CPU card memory map (simulated)

From [`memory-map.md`](../memory-map.md) and [`register-map-v1.md`](../register-map-v1.md):

```text
$000000-$007FFF  RAM
$008000-$00BFFF  expansion window: unmapped (fault)
$00C000-$00C003  UART: +00 DATA, +01 STATUS (bit1 TX ready)
$00C200-$00C205  Supervisor mailbox (register path)
$00C300-$00C305  Board identity (read-only)
$00D000-$00D001  Fabric control: version, status
$00D100-$00D107  Interrupt controller
$00D200-$00D2FF  Mailbox Engine
$00E000-$00FFFF  ROM, vectors at top of bank; writes are ignored and faulted
$010000-$7FFFFF  RAM (banks $01-$7F)
other            unmapped: reads return $FF; every access is a recorded fault
```

## Mailbox Engine register profile (proposed)

`register-map-v1.md` names offsets +00 to +05 and leaves payload storage
"implementation dependent". The simulator uses this profile. It is a proposal
for review, not yet a hardware contract.

```text
$00D200 +00 STATUS        R   bit1 RX ready, bit7 fault latched
$00D201 +01 CONTROL       W   $01 submit TX frame, $02 acknowledge RX frame
$00D202 +02 LENGTH        R   RX payload length, low byte
$00D203 +03 SOURCE ID     R   RX source node, low byte
$00D204 +04 DEST ID       R/W TX destination node, low byte
$00D205 +05 MESSAGE TYPE  R   RX message type, low byte
$00D206 +06 RESULT        R   atarix_mailbox_status_t of the last submit
$00D207 +07 DEST ID HI    R/W
$00D208 +08 SOURCE ID HI  R
$00D209 +09 TYPE HI       R
$00D20A +0A LENGTH HI     R
$00D280-$00D2BF           TX frame window (16-byte v1 header + up to 48 payload bytes)
$00D2C0-$00D2FF           RX frame window (read-only)
```

Frames use the v1 header from [`mailbox-protocol-v1.md`](../mailbox-protocol-v1.md),
little-endian ([ADR-0011](../adr/ADR-0011-little-endian-data-model.md)). The
source node is attached by the Northbridge from the CPU card's slot. The CPU
cannot state or forge it.

Validation order (protocol order, production code):

```text
header (magic, type, length, CRC) -> sequence -> endpoint -> ring -> capability -> dispatch
```

## Interrupts and Vector Pull Rewrite

Vega816's Vector Pull Rewrite is the idea `vega816-analysis.md` says to keep
and integrate with `interrupt-architecture-v1.md`. The simulator models it on
the CPU card bus. The emulated W65C816 drives VPB on every vector fetch.

- **Controller** (`$00D100`): the "preferred v1" layout of
  `interrupt-architecture-v1.md`: `+00/+01` pending, `+02/+03` mask (a set bit
  masks), `+04` dispatch level (`$FF` none), `+05` acknowledge (source number),
  `+06` route target (only `$01` primary CPU), `+07` status. At boot everything
  is masked and nothing is pending. Sources are level-triggered: acknowledging
  clears a pending bit only after the source has deasserted.
- **Rewrite rule** (Vega816 README): VPB active and A1–A3 all high (only the
  IRQ vector matches) → the dispatch level is added to A1–A4, A0 is kept and
  the carry out of A4 is dropped. Native IRQ level *n* fetches `$FFEE + 2n`.
- **ATARIX profile** (proposed): rewrite in native mode only, and never level
  7 in native mode, because that would fetch the RESET vector.
- **Levels** (proposed): the priority order in `interrupt-architecture-v1.md`
  folded onto seven vectors: supervisor and fabric fault 0, DMA 1, mailbox 2,
  timer 3, UART 4, network and accelerator 5, external slot and debug 6.

## Supervisor register path

`register-map-v1.md` gives the CPU a raw supervisor command register at
`$00C200` (Reset Request, Recovery Mode, Read Board ID, Read Fault Log, FPGA
Reload). This path does not go through mailbox v1 ring or capability checks.
The simulator accepts only `$03` Read Board ID from the CPU (it completes and
raises the Supervisor interrupt) and rejects every other command with STATUS
`$80`. See finding 6.

## Test topology

| Node | Ring | Role |
|---|---|---|
| `0x0100` CPU card, slot 0 | kernel | requester (trust: identified) |
| `0x0000` supervisor | supervisor | must be unreachable from the kernel ring |
| `0x0001` fabric control | fabric | must be unreachable from the kernel ring |
| `0xFF00` test echo service | service | replies PONG to PING with the request's sequence and payload |

The CPU card holds exactly one capability: `READ` on the echo service.

## What is checked

The firmware runs ten mailbox cases. The host then checks the Northbridge
audit log independently of what the firmware reports.

| # | Request | Expected decision | Why |
|---|---|---|---|
| T11 | PING → echo, seq 1 | OK, PONG | granted; firmware checks every PONG field and recomputes its CRC |
| T12 | PING → supervisor, seq 2 | RING_DENIED | kernel ring cannot request the supervisor ring |
| T13 | PING → fabric control, seq 3 | RING_DENIED | kernel ring cannot request the fabric ring |
| T14 | WRITE_REGISTER → echo, seq 4 | CAPABILITY_DENIED | READ does not grant WRITE |
| T15 | PING with corrupted CRC, seq 5 | CRC_MISMATCH | rejected before the sequence is consumed |
| T16 | PING, seq 5 | OK, PONG | the same sequence is accepted once valid |
| T17 | PING replay, seq 5 | DUPLICATE | replay detection |
| T18 | PING → unknown node `0x0404`, seq 6 | POLICY_DENIED | unknown endpoints are denied by default |
| T19 | PING, seq 8 | SEQUENCE_ERROR | gap detection |
| T1A | unknown type `0x0099`, seq 7 | UNKNOWN_TYPE | unknown types never reach dispatch |

Interrupt cases:

| # | Check |
|---|---|
| T21 | Boot defaults: everything masked, nothing pending, no IRQ |
| T22 | A mailbox reply while masked: source 3 pending, no IRQ, no dispatch level |
| T23 | Supervisor Read Board ID completes and raises source 1 |
| T24 | Unmask both: the supervisor IRQ is taken first at `$FFEE` (level 0), then the mailbox IRQ rewritten to `$FFF2` (level 2), each in its own handler; board ID delivered; nothing left pending |
| T25 | Mask everything: IRQ deasserts |
| T26 | A CPU Reset Request through the supervisor register path is rejected |

The host independently checks the exact vector-fetch sequence: reset
`$FFFC/D`, then `$FFEE/F` unchanged, then `$FFEE/F` → `$FFF2/3`. It also checks
that no IRQ is ever redirected to RESET, and that the supervisor accepted only
Read Board ID.

The run also checks:

- boot through the reset vector into native mode;
- RAM in bank `$00` and long RAM in banks `$01` and `$7F`;
- ROM write protection;
- board identity readable;
- that exactly one bus fault occurs (the deliberate ROM write).

## Negative controls

A test that cannot fail proves nothing. Each control below must fail, and
does:

| Control | Detected at |
|---|---|
| Northbridge skips the capability check | T14 |
| Northbridge skips the ring check | T12 |
| Northbridge accepts a bad CRC | T15 |
| Northbridge corrupts the PONG payload | T11 |
| Firmware prints PASS without running the tests | empty audit log |
| Vector Pull Rewrite does not rewrite | T24; host vector-fetch check |
| Interrupt mask ignored | T22 |
| Mailbox dispatched at level 7 | the CPU jumps to the RESET vector mid-run; host flags the redirect |
| Supervisor executes a CPU reset request | T26; host command check |
| ECP5 fit asked for an unreachable 1000 MHz | rejected |
| Vector runner given corrupted register, RAM, cycle and bus-line expectations | each reported |
| RTL never sees one mailbox byte | byte-stream mismatch |
| 14 MHz PHI2 on the 25 MHz clock (below the proven ratio) | 165/242 bytes correct: rejected |
| Formal: PHI2 phases of 3 steps (ratio too low) | counterexample |
| Formal: latency bound of 5 steps | counterexample (6 is tight) |
| Formal: shim reads the live bus instead of the held capture | counterexample |

## Evidence (2026-11-06)

CPU instruction vectors, full sets, CLK pinned at `0f49993`:

| Set | Source (pinned) | Tests | Result |
|---|---|---|---|
| W65C816, emulation and native, all 256 opcodes | SingleStepTests/65816 `307c63f` | 5,080,400 | **all pass**: registers, RAM and every bus cycle and pin |
| NMOS 6502 | dlworrell/65x02 `66ea3ef` | 2,560,000 | all pass (harness self-check) |
| NES 6502 | dlworrell/65x02 `66ea3ef` | 2,560,000 | all pass |
| WDC 65C02 | dlworrell/65x02 `66ea3ef` | 2,540,000 | registers and RAM all match; 69,821 differ only in dummy bus cycles of 11 opcodes (recorded in `sim/cpu/known-deviations/wdc65c02.txt`); WAI/STP files are empty upstream |
| Rockwell 65C02 | dlworrell/65x02 `66ea3ef` | 2,560,000 | registers and RAM all match; 70,178 differ only in bus cycles; not used by ATARIX; not gated |
| Synertek 65C02 | dlworrell/65x02 `66ea3ef` | 2,560,000 | registers and RAM all match; 230,284 differ only in bus cycles (mostly the `xF` opcodes, which are BBR/BBS on later 65C02s); not used by ATARIX; not gated |

For all 6502-family (8-bit) sets, the runner ignores the P register's B bit
(bit 4) when comparing registers, because it is not a physical register bit.
Values of B pushed to the stack are still compared exactly through RAM and
bus cycles. The 65816 comparison uses all bits.

CPU card: PASS in 35,000 CPU cycles; 11 of 11 audit decisions as expected; 6
vector fetches exactly as expected; 1 expected fault.

RTL co-simulation, functional timing: 242 of 242 mailbox bytes captured by the
shim, in order.

RTL co-simulation at W65C816 datasheet timing (10 ns write-data and address
hold, edges not aligned to the system clock):

| PHI2 | System clock | Original shim | Current shim |
|---|---|---|---|
| 4 MHz (bring-up) | 25 MHz (ULX3S oscillator) | 118 of 242 captured, 15 correct | **242 of 242 correct** |
| 14 MHz (target) | 62.5 MHz (PLL) | not run | **242 of 242 correct** |
| 14 MHz | 25 MHz | not run | 165 of 242: below the proven ratio, rejected as a negative control |

## Bus shim formal proof

`scripts/rtl/run_shim_formal.sh` proves `atx_northbridge_bus_shim.v` with the
harness `sim/formal/bus_shim_formal.sv`, using Yosys only: `clk2fflogic` for
the two unrelated clocks, then built-in SAT k-induction. It needs no external
solver; this Ubuntu's yosys-smtbmc did not work with z3 or boolector on this
model.

The model and properties:

- The system clock toggles every step. Every PHI2 phase lasts at least *H*
  steps.
- Chip select, R/W and data are unconstrained: they may change at any step,
  including at the PHI2 edge.
- For all such inputs:
  1. no write is delivered twice;
  2. no write is lost;
  3. each delivered byte and frame flag equal what the CPU wrote at the PHI2 fall;
  4. delivery takes at most 3 system-clock periods.

Results:

| Run | Result |
|---|---|
| *H* = 4 steps (each PHI2 phase ≥ 2 system clocks) | **proven** (k-induction, depth 7) |
| *H* = 3, 2, 1 | counterexample: the ratio is the true minimum |
| Latency bound 3 clock periods | proven; 2.5 gives a counterexample |
| Mutant: reads the live bus | counterexample |
| Original scaffold, same properties (*H* = 4, 8 and 16) | counterexample at every ratio |

**Clock requirement:** each PHI2 phase must last at least two system-clock
periods, so f_clk ≥ 4 × f_PHI2 at 50% duty:

| PHI2 (docs/rev-a-hardware-baseline.md) | Minimum system clock | On the ULX3S |
|---|---|---|
| 1 MHz debug | 4 MHz | 25 MHz oscillator |
| 4 MHz bring-up | 16 MHz | 25 MHz oscillator |
| 8 MHz conservative | 32 MHz | PLL needed |
| 14 MHz target | 56 MHz | PLL needed (62.5 MHz verified) |

The proof treats time in steps. It does not model metastability; the
two-flop synchronizer is the standard protection for that. The datasheet-timing
co-simulation covers sub-step timing.

ECP5 fit on the LFE5U-85F-6BG381C (the ULX3S part, ADR-0001), placed and
routed out of context, with a 25 MHz target (the ULX3S oscillator):

| Top | Fmax | LUT4 | FF | DSP |
|---|---|---|---|---|
| `atx_northbridge_bus_shim` | 411 MHz system clock, 723 MHz PHI2 domain | 31 / 83,640 | 24 / 83,640 | 0 |
| `atx_spec_020_system_wrapper` | 76.7 MHz | 726 / 83,640 | 188 / 83,640 | 1 MULT18X18D / 156 |

Out of context means no I/O pins: no pinout is decided, and the ATX-SPEC-020
wrapper has more ports than the ULX3S has GPIO.

## Findings

1. **Bus shim framing (RTL).** `atx_northbridge_bus_shim.v` starts a frame
   (SOF) whenever chip select deasserts. A real W65C816 fetches instructions
   between stores, so chip select drops between most mailbox writes. The
   result is 140 SOF markers for 10 actual frames. Byte capture is correct;
   frame boundaries are not usable. The co-simulation reports this rather than
   failing until the framing rule is specified (for example, frame on CONTROL
   submit as the C model does). `--expect-sof-per-frame` makes it a hard
   failure once fixed.
2. **Public headers are not C++-clean.** They use C11 `_Static_assert`. The
   header gate compiles them as C++ only with `-D_Static_assert=static_assert`,
   and this bench does the same. Fixing it properly belongs in the headers, as a
   separate change.
3. **`atarix_ring_to_wire()` maps an invalid ring to APPLICATION** rather than
   failing closed (`include/atarix/rings.h`). Not exercised by this bench;
   recorded for review.
4. **Bus shim timing (RTL): resolved.** The original shim sampled the PHI2
   edge, chip select and data from the system clock, up to one clock (40 ns at
   25 MHz) after the fall. The W65C816 holds them for only about 10 ns. It
   missed or corrupted most writes at datasheet timing, and it fails the
   formal properties at every clock ratio.

   The shim now registers data, chip select and R/W on the PHI2 falling edge.
   It hands each byte to the system clock through a toggle and a two-flop
   synchronizer. It is proven correct for f_clk ≥ 4 × f_PHI2, and the
   datasheet-timing co-simulations are gating. Hardware consequence: PHI2 must
   reach the FPGA on a clock-capable pin, and 8 or 14 MHz operation needs an
   ECP5 PLL above the 25 MHz oscillator.
5. **Interrupt controller layout conflict.** `register-map-v1.md` defines
   `+00 Pending, +01 Mask, +02 Priority, +03 Acknowledge`.
   `interrupt-architecture-v1.md` calls that a "compatibility subset" of its
   preferred `+00 Pending Low, +01 Pending High, +02 Mask Low, ...`, but the
   offsets disagree, so it is not a subset. The simulator uses the preferred
   layout.
6. **Supervisor register path bypasses mailbox authority.** The documented
   `$00C200` commands include Reset Request, Recovery Mode and FPGA Reload as
   plain register writes, with no ring or capability check. That conflicts with
   "kernel ring cannot request the supervisor ring" on the mailbox path. The
   simulator rejects everything except Read Board ID.
7. **Emulation vectors overlap native IRQ levels.** With Vector Pull Rewrite,
   native IRQ levels 3–6 fetch `$FFF4`–`$FFFA`, which are the emulation-mode
   COP, ABORT and NMI vectors. This is the same as Vega816. An NMI or ABORT
   taken before the switch to native mode would run an IRQ-level handler.
8. **Audit log window is built from registers, not block RAM.** yosys reports
   `storage_matrix` in `atx_audit_log_window.v` mapped to registers. That is
   fine at its current size; it will not scale to a large audit window.

## Open decisions for review

- The Mailbox Engine register profile above (offsets +06 to +0A and the frame windows).
- The mapping from message type to required operation (`READ` for query
  messages, `WRITE` for state changes, `RESET` for reset requests) in
  `Northbridge::operation_for`.
- Unknown destination reported as `POLICY_DENIED` (the v1 status set has no
  "no such endpoint" value).
- The CPU card's ring (kernel) and the test echo service's ring (service).
- Whether to fork `SingleStepTests/65816` into `dlworrell/` for durability.
- The interrupt level mapping, native-only rewrite, and the ban on level 7 in native mode.
- Which interrupt controller layout is authoritative (finding 5).
- Which supervisor commands, if any, the CPU may issue by register (finding 6).
- The shim's framing rule (finding 1). Capture timing is resolved (finding 4).
- Whether a 62.5 MHz (or other PLL) system clock is accepted for 8 and 14 MHz operation.
- The board pinout for the Northbridge on the ULX3S GPIO.

## Related hardware projects

| Repository | Role in this bench |
|---|---|
| `dlworrell/CLK` | Vendored W65C816 core (`third_party/clk`) |
| `dlworrell/65x02` | Pinned 6502/65C02 vectors (harness self-check) |
| `dlworrell/Vega816` | Source of the Vector Pull Rewrite rule and IRQ-level layout modeled here |
| `dlworrell/ulx3s` (`14cac31`) | Target FPGA (LFE5U-85F, CABGA381) and 25 MHz clock for the fit check |
| `dlworrell/BB816-MATX-PCIE` | Not modeled. Its connector is the BB816 bus that `vega816-analysis.md` says not to keep as the ATARIX interface. Its bank-zero board (per-CPU bank-zero isolation) is the next candidate, with DMA. |
| `dlworrell/cglm` | Not used yet. Intended as video-chip ROM content. It is single-precision `float` C, so it cannot run on the W65C816. It fits as the boot-ROM firmware of a display or GPU card's own processor (a soft RISC-V core with floating point in the ECP5, or an RP2350, whose Cortex-M33 has a single-precision FPU), exposed to the CPU card as `SERVICE_GPU_COMPUTE` (`0x6000`) over the mailbox. In the simulator, the same service can call cglm directly as the reference model. |

## Closure units

Recorded under [`docs/engineering/closure-units/`](../engineering/closure-units/ATX-SIM-CLOSURE-INDEX.md).
