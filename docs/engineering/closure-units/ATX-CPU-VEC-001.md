---
document: ATX-CPU-VEC-001
title: Emulated W65C816 instruction conformance
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
implements:
  - sim/cpu/vector_runner.cpp
  - third_party/clk
verified_by:
  - .github/workflows/cpu-vectors.yml
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:c8091f3a-cc37-48d4-a68e-cdf0b285966a
-->

# ATX-CPU-VEC-001: Emulated W65C816 instruction conformance

```yaml
closure_unit:
  id: ATX-CPU-VEC-001
  claim: >-
    The CPU model used by the ATARIX simulator matches W65C816 instruction-level
    behavior, in emulation and native mode, for registers, memory and every bus
    cycle including VDA, VPA, VPB, RWB, E, M, X and ML.
  intent:
    authority: normative
    sources:
      - docs/cpu-test-suite-integration-v1.md
      - docs/cpu-observability-contract-v1.md
  realizations:
    emulator_cpu_core: required        # third_party/clk at 0f49993 (vendored, MIT)
    vector_runner: required            # sim/cpu/vector_runner.cpp
    hardware_validation_rom: deferred  # generator from vectors, for Rev A bring-up
    physical_hardware: deferred
  evidence:
    w65c816_full_vectors: required     # E4: 5,080,400 / 5,080,400 pass
    harness_self_check_6502: required  # E4: NMOS 6502 2,560,000 / 2,560,000 pass
    runner_negative_control: required  # E1: register, RAM, cycle, line corruption detected
    pr_sample: required                # sim/cpu/sample-opcodes.txt in CI
    hardware_capture: deferred
  security_impact: none directly; the CPU model is not an authority
  compatibility_impact: none
  recovery_impact: none
  state: VERIFYING
```

## Evidence

- Full SingleStepTests/65816 (`307c63f`), 512 files: 5,080,400 tests pass,
  0 fail, 0 known deviations (2026-11-06, about 79 s).
- dlworrell/65x02 (`66ea3ef`) NMOS 6502 and NES 6502: all 2,560,000 pass each.
  This shows the harness compares exactly.
- WDC 65C02: registers and RAM all match; dummy-cycle differences in 11
  opcodes are recorded in `sim/cpu/known-deviations/wdc65c02.txt` and are
  not adjudicated. ATARIX does not use the 65C02.

## Gaps

- No hardware capture exists to adjudicate a disagreement between the vector
  set and a real W65C816. Two independent models agreeing is E4, not E5.
- The vector-to-ROM generator for hardware bring-up is not written.
- WAI and STP have no vectors in the 65C02 set (empty upstream files).
