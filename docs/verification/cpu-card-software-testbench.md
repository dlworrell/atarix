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
bash scripts/engineering/run_gate.sh         # includes the offline test bench as a gate phase
```

Tools: a C11 and C++20 compiler, `cc65` (ca65/ld65), Icarus Verilog, Python 3.

## Components

| Path | What it is |
|---|---|
| `third_party/clk/` | Vendored CLK W65C816 and 6502-family cores, pinned (MIT). See its README. |
| `sim/cpu/vector_runner.cpp` | Replays single-instruction vectors; compares registers, RAM and every bus cycle including VDA/VPA/VPB/RWB/E/M/X/ML. |
| `sim/cpu/sample-opcodes.txt` | Opcodes run on every pull request. |
| `sim/cpu/known-deviations/` | Recorded, explained differences; never silent. |
| `sim/cpu-card/cpu_card.cpp` | Rev A CPU card: memory map, UART, board identity, faults, bus trace. |
| `sim/cpu-card/northbridge.cpp` | Mailbox Engine register block calling libatarix. |
| `sim/cpu-card/firmware/test_rom.s` | 65816 test ROM: boot, self-checks, mailbox cases, CRC32 in firmware. |
| `sim/rtl-cosim/` | Replays the CPU bus trace into the bus shim RTL. |
| `scripts/sim/` | Fetch, run and re-vendor scripts. |

## CPU card memory map (simulated)

From [`memory-map.md`](../memory-map.md) and [`register-map-v1.md`](../register-map-v1.md):

```text
$000000-$007FFF  RAM
$008000-$00BFFF  expansion window: unmapped (fault)
$00C000-$00C003  UART: +00 DATA, +01 STATUS (bit1 TX ready)
$00C300-$00C305  Board identity (read-only)
$00D000-$00D001  Fabric control: version, status
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
| Vector runner given corrupted register, RAM, cycle and bus-line expectations | each reported |
| RTL never sees one mailbox byte | byte-stream mismatch |

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

CPU card: PASS in 31,000 CPU cycles; 10 of 10 audit decisions as expected; 1
expected fault.

RTL co-simulation: 220 of 220 mailbox bytes captured by the shim, in order.

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

## Open decisions for review

- The Mailbox Engine register profile above (offsets +06 to +0A and the frame windows).
- The mapping from message type to required operation (`READ` for query
  messages, `WRITE` for state changes, `RESET` for reset requests) in
  `Northbridge::operation_for`.
- Unknown destination reported as `POLICY_DENIED` (the v1 status set has no
  "no such endpoint" value).
- The CPU card's ring (kernel) and the test echo service's ring (service).
- Whether to fork `SingleStepTests/65816` into `dlworrell/` for durability.

## Closure units

Recorded under [`docs/engineering/closure-units/`](../engineering/closure-units/ATX-SIM-CLOSURE-INDEX.md).
