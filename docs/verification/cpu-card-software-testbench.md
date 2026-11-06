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
Supervisor boot authority chain (CPU held in reset):
    identity -> enumeration -> capability policy -> binding -> activation
    -> reset released
65816 test ROM (ca65)
    -> W65C816 core (vendored CLK, cycle-accurate)
    -> emulated Rev A CPU card bus (memory map, UART, identity)
    -> Discovery Services, Mailbox Engine and Supervisor Request Gateway models
    -> libatarix mailbox, ring, capability and identity code (production C)
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
bash scripts/firmware/run_gpu_core_m33.sh    # display card GPU core on Cortex-M33 (RP2350 core, QEMU)
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
| `sim/cpu-card/supervisor.cpp` | Supervisor (ring −2): boot authority chain, capability policy, Supervisor Request Gateway, supervisor audit. |
| `sim/cpu-card/northbridge.cpp` | Fabric Northbridge (ring −1): endpoints, Discovery Services directory, Mailbox Engine calling libatarix. |
| `sim/cpu-card/interrupts.cpp` | Interrupt controller and Vega816 Vector Pull Rewrite. |
| `src/identity/identity_validate.c` | Identity EEPROM v1 validation (production C), used by the supervisor model. |
| `src/firmware/display-card/gpu_core.c` | Display card GPU core (C11, single precision, cglm), shared by the simulator and the RP2350 firmware. |
| `src/firmware/display-card/m33/` | Cortex-M33 startup, linker script and test for QEMU `mps2-an505`. |
| `sim/cpu-card/gpu_service.cpp` | Simulator wrapper around the GPU core (adds negative-control faults outside it). |
| `sim/cpu-card/firmware/gen_gpu_vectors.py` | GPU requests and independently computed expected results for the test ROM. |
| `third_party/cglm/` | Vendored cglm headers (MIT), pinned. See its README. |
| `rtl/atarix/atx_mailbox_tx_assembler.v` | Builds frames from the shim's writes on SUBMIT. |
| `sim/formal/bus_shim_formal.sv` | Formal harness for the bus shim. |
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
$00C200-$00C206  Supervisor Request Gateway
$00C300-$00C305  Board identity (read-only)
$00D000-$00D001  Fabric control: version, status
$00D100-$00D107  Interrupt controller
$00D200-$00D2FF  Mailbox Engine
$00D500-$00D50D  Discovery Services (ring-filtered service directory)
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

Validation order (ATX-SPEC-005 and `mailbox-protocol-v1.md`, production code):

```text
header (magic, type, length, CRC) -> sequence -> endpoint -> ring -> capability
    -> lifecycle state -> resources -> dispatch
```

Lifecycle and resources come after authority: a free queue or an ACTIVE
endpoint is not authority. An endpoint that is not ACTIVE gives
`TARGET_UNAVAILABLE` (15), a quarantined one `QUARANTINED` (12), and an
occupied reply slot `BUSY` (13).

Every decision is an audit record with the ATX-SPEC-010 fields the bench can
supply: actor node and ring (assigned by the fabric), target node and ring,
operation, authority used (capability id, 0 when none), policy result,
result, reason, cycle and the deciding stage. The host checks the authority
of every decision. Record ID, record version and identity labels are not
defined yet (gap G-AUD-1).

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

## Test topology

The CPU card runs the kernel (ring 0) in slot 0, with node `0x0100` and trust
*identified*. Its identity and ring come from its slot, never from the CPU.

| Node | Card or endpoint | Ring | Identity | Result of the boot chain |
|---|---|---|---|---|
| `0x0000` | supervisor | −2 | platform | ACTIVE; not in the kernel's directory |
| `0x0001` | fabric control | −1 | platform | ACTIVE; not in the kernel's directory |
| `0x0300` | display card, `SERVICE_GPU_COMPUTE` | 1 driver | factory | validated; capability 1 (READ); ACTIVE |
| `0x0301` | homebrew display card, `SERVICE_GPU_COMPUTE` | 1 driver | homebrew | identified; capability withheld (GPU compute needs validated); ACTIVE |
| `0x0200` | storage card, `SERVICE_BLOCK_STORAGE` | 1 driver | corrupted (one bit, stale CRC) | identity failed; QUARANTINED; no capability |
| `0xFF00` | test card, echo `SERVICE_VALIDATION` | 2 user service | development | validated; capability 2; ACTIVE |
| `0xFF01` | test card, echo `SERVICE_VALIDATION`, firmware not ready | 2 user service | development | validated; capability 3; SUSPENDED |

The echo service replies PONG to PING with the request's sequence and
payload.

## Boot authority chain

The supervisor model (`supervisor.cpp`, ring −2) runs the chain from
ADR-RING-SECURITY-MODEL and ADR-THREE-FABRIC-ARCHITECTURE before the CPU card
leaves reset. Nothing in the bench grants a capability by hand.

| Step | What happens | Audit event |
|---|---|---|
| Power-on | The CPU card is held in reset. | `power-on` |
| Identity validation | Each card's Identity EEPROM v1 is checked with `atarix_identity_validate_v1()`: magic, version, lengths, CRC, vendor, device, classes. A failing card is quarantined. A homebrew vendor or a homebrew or unknown trust class gives *identified*; otherwise *validated*. | `identity-validated` / `identity-failed` |
| Enumeration | The fabric creates one endpoint per declared service, in state CREATED (QUARANTINED for a failed card). Enumeration describes; it grants nothing. | `enumeration-complete` |
| Capability evaluation | Supervisor policy grants the kernel READ on a service only if the service type is in the policy and the provider's trust meets it: VALIDATION needs *identified*, GPU compute needs *validated*. | `capability-granted` / `capability-withheld` |
| Service binding | The kernel's ring-filtered directory is published at Discovery Services. | `service-binding` |
| Transport activation | Endpoints become ACTIVE, or SUSPENDED when the card's service firmware is not ready. | `transport-activated` |
| Reset release | The CPU card starts. | `reset-released` |

## Discovery Services and service binding

The firmware binds every service by service type and instance number through
`$00D500`, and checks each entry's lifecycle state and capability. No service
node id is compiled into the ROM. The only fixed node ids are three probes it
addresses on purpose although the directory does not list them: the
supervisor, fabric control, and an empty node. They show that an unlisted
node is still checked, not merely hidden.

`register-map-v1.md` defines `+00` to `+06`. The rest is a proposed profile
(gaps G-BOOT-1 and G-BOOT-2):

```text
$00D500 +00 COUNT         R   entries in the caller's ring-filtered view
$00D501 +01 SELECT        R/W entry index (the only writable register)
$00D502 +02/+03 VENDOR    R   provider vendor id, low 16 bits
$00D504 +04/+05 DEVICE    R   provider device id, low 16 bits
$00D506 +06 CAPABILITY    R   capability id granted to the caller for this entry, 0 = none
$00D507 +07 RING          R   provider ring, wire value low byte
$00D508 +08/+09 SERVICE   R   service type
$00D50A +0A/+0B NODE      R   node id: a transport address, not identity or authority
$00D50C +0C STATE         R   endpoint lifecycle state (ATX-SPEC-005, 0-7)
$00D50D +0D INSTANCE      R   service instance
```

Ring filtering: the view lists only endpoints in the caller's ring or less
privileged rings, so the kernel never sees the supervisor or fabric
endpoints.

## Supervisor Request Gateway

`register-map-v1.md` lists supervisor commands at `$00C200` as plain register
writes. ATX-SPEC-016 says the operational fabric may only *request*
supervisor actions, through a gateway and a policy check. The bench
implements the gateway (closure unit ATX-SUP-REG-001):

```text
$00C200 +00 COMMAND   W   writing submits the request with ARG0-ARG3
$00C201 +01 STATUS    R   bit1 done, bit7 not executed; reading clears done
                          and deasserts interrupt source 1 (Supervisor)
$00C202 +02..+05 ARG  R/W request arguments in, results out
$00C206 +06 RESULT    R   ATX-SPEC-016 result of the last request
```

Results (proposed values): 0 EXECUTED, 1 DENIED, 2 DEFERRED,
3 REQUIRES_APPROVAL, 4 QUARANTINED, 5 UNSUPPORTED, 6 UNKNOWN, 7 FAILED.
Every completion raises the Supervisor interrupt and adds a
`request-received` event and a result event to the supervisor audit. The
caller and its ring come from the slot.

Command policy for a kernel caller (proposed; gaps G-SUP-2 to G-SUP-5):

| Command | Result | Why |
|---|---|---|
| `01` Reset Request | REQUIRES_APPROVAL | a system reset from the operational fabric needs an operator; nothing is reset |
| `02` Recovery Mode | DENIED | recovery is not available to the operational fabric |
| `03` Read Board ID | EXECUTED | observation; board id returned in ARG0–ARG3 |
| `04` Read Fault Log | UNSUPPORTED | the record format is not specified |
| `05` FPGA Reload | DENIED | firmware and bitstream state are supervisor-only |
| `06` Report Fault (proposed; ARG0/ARG1 = node) | EXECUTED or DENIED | the supervisor quarantines a less privileged endpoint; a supervisor, fabric or kernel endpoint cannot be reported |
| anything else | UNKNOWN | not in the command table |

## GPU compute service

cglm is single-precision C, so it cannot run on the W65C816. Per
[ADR-0016](../adr/ADR-0016-display-card-gpu-compute-rp2350.md), each display
card has its own RP2350 (not the supervisor's), running this service on its
Cortex-M33 cores. The service core, `src/firmware/display-card/gpu_core.c`, is
the same C source in the simulator and in the card firmware. The CPU card
reaches it only through the mailbox, with the same header, sequence, ring and
capability checks as any other service.

**On the RP2350's core:** `scripts/firmware/run_gpu_core_m33.sh` checks:

- the core cross-compiles for `cortex-m33` / FPv5-SP with `-Werror`;
- it contains 103 single-precision FPU instructions and no double-precision
  arithmetic;
- on QEMU `mps2-an505` it passes all 6 cases at **0 LSB**, bit-identical to the
  simulator's x86 SSE build;
- negative controls: a `double` probe is flagged, and a 3 LSB error is
  rejected.

QEMU is not an RP2350 (no RP2350 peripherals, boot ROM or timing).

- **Messages** (proposed; added to `mailbox.h` and `mailbox-protocol-v1.md`):
  `GPU_TRANSFORM_REQUEST` `0x0040` and `GPU_TRANSFORM_REPLY` `0x0041`.
- **Authority:** a transform computes a result and changes no state, so it
  needs `READ` (proposed) on the target GPU's handle.
- **Number format:** signed 16.16 fixed point, which the W65C816 builds and
  reads with integer arithmetic. The service converts to `float`, runs cglm
  and rounds back.
- **Request** (44 bytes, little-endian):

  ```text
  +0  u16 operation (1 = TRANSFORM_POINT)   +2  u16 flags (0)
  +4  point x, y, z                         +16 axis x, y, z (normalized; zero rejected)
  +28 angle (radians)                       +32 translation x, y, z
  ```

  The point is rotated about the axis, then translated (cglm
  `glm_translate_make`, `glm_rotate`, `glm_mat4_mulv3`).
- **Reply** (16 bytes): `u32 status` then x, y, z. Status: 0 ok, 1 bad length,
  2 bad operation, 3 out of range, 4 zero axis. Service errors are carried in
  the reply; the mailbox decision is still a successful dispatch.
- **Independent expected values:** `gen_gpu_vectors.py` computes them in Python
  double precision with Rodrigues' formula, not with cglm. The firmware accepts
  ±2 LSB. Measured difference from cglm: **0 LSB on every coordinate**.

## What is checked

The firmware first binds its services (T20), then runs the cases below. The
host then checks the Northbridge audit log, the supervisor audit and the
gateway results independently of what the firmware reports.

| # | Check |
|---|---|
| T20 | Discovery Services lists exactly the five chassis services, none in a platform ring. Echo instance 0 is ACTIVE with a capability; echo instance 1 is SUSPENDED with a capability; GPU instance 0 is ACTIVE with a capability; the homebrew GPU is ACTIVE without one; the storage card is QUARANTINED without one. Node ids for later cases come from here. |

Mailbox cases (the echo node is the one bound in T20):

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
| T26 | A Reset Request through the gateway gets REQUIRES_APPROVAL; STATUS bit 7 set; nothing is reset |

GPU cases (sequence numbers 8–12):

| # | Request | Expected |
|---|---|---|
| T31 | Rotate (1,0,0) 90° about Z, translate (2,3,4) | (2,4,4) within ±2 LSB |
| T32 | Rotate (1,2,3) 30° about the unnormalized axis (1,1,0), translate (0.5,0,−1) | (2.627661, 0.872339, 1.951624) within ±2 LSB |
| T33 | Unsupported operation 7 | reply status 2 |
| T34 | 40-byte request | reply status 1 |
| T35 | Request to the homebrew GPU | `CAPABILITY_DENIED`, no reply: policy withheld the capability |

Lifecycle and resource cases (ATX-SPEC-005), sequence numbers 13–16:

| # | Request | Expected decision | Why |
|---|---|---|---|
| T36 | PING → storage card | CAPABILITY_DENIED | listed in the directory but quarantined at boot: lookup is not access |
| T37 | PING → echo instance 1 | TARGET_UNAVAILABLE | capability held, endpoint SUSPENDED |
| T38 | PING → echo (seq 15), then PING (seq 16) without acknowledging the reply | OK, then BUSY | one reply slot; the firmware then checks the slot still holds the reply to 15 |

Supervisor Request Gateway cases. For each, the firmware also checks that
STATUS shows completion, that bit 7 agrees with RESULT, and that reading
STATUS cleared the interrupt:

| # | Request | Expected result |
|---|---|---|
| T41 | `02` Recovery Mode | DENIED |
| T42 | `04` Read Fault Log | UNSUPPORTED |
| T43 | `05` FPGA Reload | DENIED |
| T44 | `7E` (not a command) | UNKNOWN |
| T45 | `06` Report Fault, node `0x0000` (supervisor) | DENIED |
| T46 | `06` Report Fault, the GPU bound in T20 | EXECUTED: the supervisor quarantines it |
| T47 | Directory shows the GPU QUARANTINED; GPU request (seq 17) | QUARANTINED, although the kernel still holds the capability |

For each reply the firmware checks the reply type, source node, length,
sequence and recomputed CRC. The host checks the GPU service log independently
(statuses 0, 0, 2, 1 from `0x0300` only).

The host independently checks:

- all 21 mailbox decisions, in order, including the authority (capability
  id) each allowed decision used and the actor ring;
- the 32 supervisor events, in order: the 15 boot-chain events, then each
  gateway request as received and decided, and the GPU's quarantine;
- the 8 gateway requests: command, argument, caller, caller ring and result;
  and that no reset was executed;
- the exact vector-fetch sequence: reset `$FFFC/D`, then `$FFEE/F` unchanged,
  then `$FFEE/F` → `$FFF2/3`; and that no IRQ is ever redirected to RESET.

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
| Supervisor gateway executes a CPU reset request | T26; host gateway and reset checks |
| Northbridge delivers to endpoints that are not ACTIVE | T37 |
| Northbridge overwrites an unacknowledged reply | T38 |
| Directory lists supervisor and fabric endpoints to the kernel | T20 (directory count and ring check) |
| CPU released from reset before enumeration and binding | T20 (empty directory); host boot-event check |
| Capabilities granted from discovery instead of policy | T20 (homebrew GPU and storage card hold capabilities); host checks |
| Cards trusted without identity validation | T20 (storage card not quarantined); host boot-event check |
| ECP5 fit asked for an unreachable 1000 MHz | rejected |
| Vector runner given corrupted register, RAM, cycle and bus-line expectations | each reported |
| RTL never sees one mailbox byte | byte-stream mismatch |
| 14 MHz PHI2 on the 25 MHz clock (below the proven ratio) | 589/757 writes correct: rejected |
| Formal: PHI2 phases of 3 steps (ratio too low) | counterexample |
| Formal: latency bound of 5 steps | counterexample (6 is tight) |
| Formal: shim reads the live bus instead of the held capture | counterexample |
| RTL never sees one SUBMIT | 20 of 21 frames; audit mismatch |
| GPU service rotates the wrong way | T31 |
| GPU result 3 LSB off (just outside the tolerance) | T31 |

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

CPU card: PASS in 154,000 CPU cycles; 21 of 21 mailbox decisions as
expected, with the expected authority; 32 of 32 supervisor events; 8 of 8
gateway results; 6 vector fetches exactly as expected; 4 GPU requests served
as expected; 1 expected fault. Negative controls: 17 of 17 detected.

RTL co-simulation (shim + TX frame assembler), functional timing: all 757
mailbox-block writes (address and data) and all 21 frames match the C model.
Each RTL frame's message type and sequence number also match the C
Northbridge audit log, SUBMIT by SUBMIT.

RTL co-simulation at W65C816 datasheet timing (10 ns write-data and address
hold, edges not aligned to the system clock):

| PHI2 | System clock | Original shim (TX bytes only) | Current shim + assembler |
|---|---|---|---|
| 4 MHz (bring-up) | 25 MHz (ULX3S oscillator) | 118 of 242 captured, 15 correct | **all writes and all frames match** |
| 14 MHz (target) | 62.5 MHz (PLL) | not run | **all writes and all frames match** |
| 14 MHz | 25 MHz | not run | writes and frames corrupted: below the proven ratio, rejected as a negative control |

## Bus shim formal proof

`scripts/rtl/run_shim_formal.sh` proves `atx_northbridge_bus_shim.v` with the
harness `sim/formal/bus_shim_formal.sv`, using Yosys only: `clk2fflogic` for
the two unrelated clocks, then built-in SAT k-induction. It needs no external
solver; this Ubuntu's yosys-smtbmc did not work with z3 or boolector on this
model.

The model and properties:

- The system clock toggles every step. Every PHI2 phase lasts at least *H*
  steps.
- Chip select, R/W, the address and the data are unconstrained: they may
  change at any step, including at the PHI2 edge.
- For all such inputs:
  1. no write is delivered twice;
  2. no write is lost;
  3. each delivered address and data byte equal what the CPU wrote at the PHI2 fall;
  4. delivery takes at most 3 system-clock periods.

Results:

| Run | Result |
|---|---|
| *H* = 4 steps (each PHI2 phase ≥ 2 system clocks) | **proven** (k-induction, depth 7) |
| *H* = 3, 2, 1 | counterexample: the ratio is the true minimum |
| Latency bound 3 clock periods | proven; 2.5 gives a counterexample |
| Mutant: reads the live bus | counterexample |
| Original scaffold, the same data properties (*H* = 4, 8 and 16) | counterexample at every ratio |

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
| `atx_northbridge_bus_shim` | 365 MHz | 41 / 83,640 | 37 / 83,640 | 0 |
| `atx_mailbox_tx_assembler` | 108 MHz (needs 56 MHz for 14 MHz PHI2) | 1,509 / 83,640 | 1,051 / 83,640 | 0 |
| `atx_spec_020_system_wrapper` | 76.7 MHz | 726 / 83,640 | 188 / 83,640 | 1 MULT18X18D / 156 |

Out of context means no I/O pins: no pinout is decided, and the ATX-SPEC-020
wrapper has more ports than the ULX3S has GPIO.

## Findings

1. **Bus shim framing (RTL): resolved.** The scaffold started a frame (SOF)
   on the first write after chip select deasserted, which marked 154 starts for
   11 frames, because a real W65C816 fetches instructions between stores. A
   byte stream cannot carry framing: firmware writes the TX window in any
   order. The shim now delivers (address, data) pairs for the whole mailbox
   block. `atx_mailbox_tx_assembler.v` keeps the window and emits a frame on
   SUBMIT, the same rule as the C model. All frames match the C model and its
   audit at functional and datasheet timing. The assembler copies the window
   in one cycle, so both buffers are registers (1,509 LUTs); a block-RAM design
   is possible if area matters.
2. **Public headers are not C++-clean.** They use C11 `_Static_assert`. The
   header gate compiles them as C++ only with `-D_Static_assert=static_assert`,
   and this bench does the same. Fixing it properly belongs in the headers, as a
   separate change.
3. **`atarix_ring_to_wire()` mapped an invalid ring to APPLICATION: resolved.**
   It was exploitable: a capability for kernel..application admitted a
   requester with ring 5 or −7. Invalid rings now map to `0x8000`, which no
   range matches, and the capability evaluator and policy refuse invalid and
   quarantined requesters (`tests/ring/test_ring_wire_fail_closed.c`, which
   fails against the old code). Gap G-RING-4.
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
6. **Supervisor register path bypassed mailbox authority: resolved in the
   bench.** The documented `$00C200` commands included Reset Request, Recovery
   Mode and FPGA Reload as plain register writes. The block is now a
   Supervisor Request Gateway per ATX-SPEC-016 (see above).
   `register-map-v1.md` still needs to adopt the profile (gaps G-SUP-1 to
   G-SUP-7).
7. **Emulation vectors overlap native IRQ levels.** With Vector Pull Rewrite,
   native IRQ levels 3–6 fetch `$FFF4`–`$FFFA`, which are the emulation-mode
   COP, ABORT and NMI vectors. This is the same as Vega816. An NMI or ABORT
   taken before the switch to native mode would run an IRQ-level handler.
8. **Audit log window is built from registers, not block RAM.** yosys reports
   `storage_matrix` in `atx_audit_log_window.v` mapped to registers. That is
   fine at its current size; it will not scale to a large audit window.
9. **ATX-SPEC-020 index messages are rejected by the mailbox validator.**
   `mailbox.h` declares `INDEX_QUERY` to `INDEX_STATUS` (`0x0020`–`0x0023`), but
   `atarix_mailbox_message_type_known()` in `src/mailbox/mailbox.c` does not
   list them. Every such message fails with `UNKNOWN_TYPE` before dispatch.
   Not changed here, because whether it is intended is not recorded.
10. **No ring can request a more privileged ring.** `atarix_ring_can_access()`
    allows a request only to the requester's own or a less privileged ring,
    so an application cannot use a user service or a driver. The bench passes
    only because its requester is the kernel. This needs an architecture
    decision (gap G-RING-3).
11. **Specification gaps.** Every place where a specification was silent,
    contradictory, or not encoded, and the choice this bench made, is
    recorded in [Architecture Review 004](../reviews/Architecture-Review-004.md).

## Open decisions for review

- The Mailbox Engine register profile above (offsets +06 to +0A and the frame windows).
- The mapping from message type to required operation (`READ` for query
  messages, `WRITE` for state changes, `RESET` for reset requests) in
  `Northbridge::operation_for`.
- Unknown destination reported as `POLICY_DENIED` (G-MBX-4).
- The CPU card's ring (kernel), the echo service's ring (user service) and
  the GPU service's ring (driver).
- Requests toward more privileged rings (G-RING-3), the gateway profile and
  command policy (G-SUP-1 to G-SUP-5), the Discovery Services profile
  (G-BOOT-1), and the boot capability policy (G-BOOT-5): see
  Architecture Review 004.
- Whether to fork `SingleStepTests/65816` into `dlworrell/` for durability.
- The interrupt level mapping, native-only rewrite, and the ban on level 7 in native mode.
- Which interrupt controller layout is authoritative (finding 5).
- The framing rule adopted for the RTL (frame on SUBMIT, as the C model; finding 1).
- The GPU message types `0x0040`/`0x0041`, their payload layout, the 16.16
  format, and `READ` as the operation a GPU transform needs. (The processor is
  decided: a dedicated RP2350, ADR-0016.)
- The display card firmware's signed-image policy (RP2350 secure boot), so the
  GPU image behaves as ROM.
- Whether the ATX-SPEC-020 index messages should be accepted by the validator (finding 9).
- Whether a 62.5 MHz (or other PLL) system clock is accepted for 8 and 14 MHz operation.
- The board pinout for the Northbridge on the ULX3S GPIO.

## Related hardware projects

The hardware-side summary of this build (each board part, the requirements it
exposed, and what is left to build) is
[Rev A Hardware in Simulation](../hardware/rev-a-hardware-in-simulation.md).

| Repository | Role in this bench |
|---|---|
| `dlworrell/CLK` | Vendored W65C816 core (`third_party/clk`) |
| `dlworrell/65x02` | Pinned 6502/65C02 vectors (harness self-check) |
| `dlworrell/Vega816` | Source of the Vector Pull Rewrite rule and IRQ-level layout modeled here |
| `dlworrell/ulx3s` (`14cac31`) | Target FPGA (LFE5U-85F, CABGA381) and 25 MHz clock for the fit check |
| `dlworrell/BB816-MATX-PCIE` | Not modeled. Its connector is the BB816 bus that `vega816-analysis.md` says not to keep as the ATARIX interface. Its bank-zero board (per-CPU bank-zero isolation) is the next candidate, with DMA. |
| `dlworrell/cglm` (`58d8c15`) | Vendored headers (`third_party/cglm`). The video chip's firmware: it runs on the display card's dedicated RP2350 (ADR-0016), not the W65C816. Shared core in `src/firmware/display-card/`, tested in the simulator and on an emulated Cortex-M33. |

## Closure units

Recorded under [`docs/engineering/closure-units/`](../engineering/closure-units/ATX-SIM-CLOSURE-INDEX.md).
