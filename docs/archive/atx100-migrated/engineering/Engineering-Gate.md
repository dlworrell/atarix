# Engineering Gate

## Status

Draft v0.2

## Purpose

The Atarix Engineering Gate is the single validation entry point for repository health.

Local command:

```sh
bash scripts/engineering/run_gate.sh
```

GitHub Actions workflow:

```text
.github/workflows/engineering-gate.yml
```

## Current Gate Phases

```text
1. Environment report
2. Repository structure checks
3. RTL file inventory checks
4. Architecture text invariant checks
5. Public header self-compile in C
6. Public header self-compile in C++
7. Warning-as-error enforcement for public headers
8. Verilator RTL lint
9. ATX-SPEC-020 RTL simulation gate
10. Artifact and summary generation
```

## Required Invariants

The gate currently verifies these architecture invariants:

```text
Lookup acceleration is not authority.
Every accelerated lookup must be reconstructable inside a bounded audit window.
POSIX is a compatibility personality.
```

## Current Build Tools

```text
gcc
g++
iverilog
vvp
verilator
```

## Current Gate Scripts

```text
scripts/engineering/run_gate.sh
scripts/engineering/check_headers.sh
scripts/engineering/check_rtl_lint.sh
scripts/rtl/run_atx_spec_020_sims.sh
```

## Current Artifacts

```text
build/engineering-gate/artifacts/summary.txt
build/engineering-gate/artifacts/header-checks.log
build/engineering-gate/artifacts/verilator-lint.log
build/engineering-gate/artifacts/atx_spec_020_rtl/
```

The nested RTL artifact directory may include:

```text
summary.txt
atx_spec_020_accelerator_tb.log
atx_spec_020_modules_tb.log
atx_spec_020_accelerator_tb.vcd
atx_spec_020_modules_tb.vcd
```

## Header Gate

Every public header under:

```text
include/atarix/*.h
```

is compiled independently in C and C++ mode.

The intent is to catch:

```text
missing includes
include-order dependency bugs
C/C++ ABI wrapper mistakes
warnings in public headers
undeclared public types
```

## RTL Gate

The RTL gate currently runs:

```text
Verilator lint
Icarus Verilog accelerator simulation
Icarus Verilog module validation simulation
```

The RTL simulation validates:

```text
authorization gates
audit-before-response behavior
SIMD-style control-byte probe path
scalar baseline probe path
Krapivin stepper math
Elias-Fano bounded decode
audit log window readback
```

## Extension Plan

Future gate phases should add:

```text
Markdown link validation
Architecture review reference validation
C source compilation
Unit tests
ECP5 synthesis smoke test
Timing report generation
Security invariant tests
Policy invariant tests
Recovery invariant tests
Performance regression thresholds
Randomized RTL tests
Coverage reporting
```

## Rule

The gate should become stricter over time, but it should remain one command.

```text
One project.
One gate.
One answer.
```
