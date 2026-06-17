# Engineering Gate

## Status

Draft v0.1

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
5. C header ABI smoke test
6. ATX-SPEC-020 RTL simulation gate
7. Artifact and summary generation
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
iverilog
vvp
```

## Current Artifacts

```text
build/engineering-gate/artifacts/summary.txt
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

## Extension Plan

Future gate phases should add:

```text
Markdown link validation
Architecture review reference validation
C source compilation
Unit tests
RTL lint
ECP5 synthesis smoke test
Timing report generation
Security invariant tests
Policy invariant tests
Recovery invariant tests
Performance regression thresholds
```

## Rule

The gate should become stricter over time, but it should remain one command.

```text
One project.
One gate.
One answer.
```
