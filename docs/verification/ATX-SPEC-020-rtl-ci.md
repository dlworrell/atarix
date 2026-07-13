# ATX-SPEC-020 RTL CI

Status: Active
Owner: ATARIX
Scope: RTL validation for ATX-SPEC-020

## One-command Local Validation

From repository root:

```sh
bash scripts/rtl/run_atx_spec_020_sims.sh
```

The script performs the same validation used by GitHub Actions:

```text
1. Compile accelerator simulation.
2. Run accelerator simulation.
3. Require ATX020 RESULT PASS.
4. Compile module validation simulation.
5. Run module validation simulation.
6. Require ATX020 MODULES RESULT PASS.
7. Collect logs.
8. Collect VCD waveform files.
9. Generate a summary file.
```

## GitHub Actions Workflow

Workflow file:

```text
.github/workflows/rtl-atx-spec-020.yml
```

The workflow:

```text
1. Checks out the repository.
2. Installs Icarus Verilog.
3. Prints tool versions.
4. Runs scripts/rtl/run_atx_spec_020_sims.sh.
5. Uploads simulation logs and waveforms.
```

## Artifact Path

```text
build/rtl/atx_spec_020/artifacts/
```

Expected files:

```text
summary.txt
atx_spec_020_accelerator_tb.log
atx_spec_020_modules_tb.log
atx_spec_020_accelerator_tb.vcd
atx_spec_020_modules_tb.vcd
```

## Pass Criteria

Accelerator simulation must print:

```text
ATX020 RESULT PASS
```

Module validation simulation must print:

```text
ATX020 MODULES RESULT PASS
```

## Scalar-versus-SIMD Gain Proxy

The module validation simulation logs the scalar baseline cycle count with a line like:

```text
PASS gain proxy simd=1-cycle combinational scalar_cycles=N
```

This is not a full system benchmark. It validates the expected probe-stage advantage:

```text
SIMD-style probe checks 16 control bytes in one combinational probe window.
Scalar baseline checks one control byte per cycle until match or lane 15.
```

## Migration Note

This document was moved from `rtl/atarix/ATX-SPEC-020-CI.md` so normative verification documentation resides under the authoritative Atarix documentation tree while remaining adjacent by reference to the RTL implementation it governs.
