# Atarix RTL Scaffolds

## ATX-SPEC-020 Lookup Accelerator

Files:

```text
rtl/atarix/atx_northbridge_bus_shim.v
rtl/atarix/atx_spec_020_system_wrapper.v
rtl/atarix/atx_spec_020_accelerator.v
rtl/atarix/atx_simd_probe_core.v
rtl/atarix/atx_scalar_probe_core.v
rtl/atarix/atx_krapivin_stepper.v
rtl/atarix/atx_elias_fano_decoder.v
rtl/atarix/atx_audit_log_window.v
rtl/atarix/atx_spec_020_accelerator_tb.v
rtl/atarix/atx_spec_020_modules_tb.v
```

## Purpose

This RTL validates the ATX-SPEC-020 hardware integration direction:

```text
Mailbox request
  -> CRC gate
  -> ring gate
  -> capability gate
  -> policy gate
  -> registry-backed lookup
  -> bounded SIMD-style control-byte probe
  -> Krapivin-style bounded stepping
  -> audit event
  -> response
```

The simulations are intentionally verbose. They print log lines for starts, steps, audits, responses, module checks, and gain proxies.

## Module Roles

```text
atx_northbridge_bus_shim.v
    Physical W65C816-style bus sampling scaffold.

atx_spec_020_system_wrapper.v
    Structural integration point for accelerator + audit window.

atx_spec_020_accelerator.v
    Mailbox-authorized FSM controller and lookup engine.

atx_simd_probe_core.v
    Stateless 16-lane control-byte comparator.

atx_scalar_probe_core.v
    Scalar one-lane-at-a-time comparator baseline for gain measurement.

atx_krapivin_stepper.v
    Bounded non-linear open-addressing stepper.

atx_elias_fano_decoder.v
    Bounded Elias-Fano select/decode primitive.

atx_audit_log_window.v
    Circular audit projection byte window.
```

## Run Accelerator Simulation With Icarus Verilog

From repository root:

```sh
iverilog -g2012 \
  -o /tmp/atx_spec_020_accelerator_tb \
  rtl/atarix/atx_simd_probe_core.v \
  rtl/atarix/atx_krapivin_stepper.v \
  rtl/atarix/atx_spec_020_accelerator.v \
  rtl/atarix/atx_spec_020_accelerator_tb.v

vvp /tmp/atx_spec_020_accelerator_tb
```

The testbench emits:

```text
atx_spec_020_accelerator_tb.vcd
```

## Run Module Validation Simulation

```sh
iverilog -g2012 \
  -o /tmp/atx_spec_020_modules_tb \
  rtl/atarix/atx_audit_log_window.v \
  rtl/atarix/atx_simd_probe_core.v \
  rtl/atarix/atx_scalar_probe_core.v \
  rtl/atarix/atx_krapivin_stepper.v \
  rtl/atarix/atx_elias_fano_decoder.v \
  rtl/atarix/atx_spec_020_modules_tb.v

vvp /tmp/atx_spec_020_modules_tb
```

This simulation validates:

```text
SIMD lane match
Scalar lane match
SIMD vs scalar gain proxy
Krapivin stepper math
Elias-Fano bounded decode
Audit window write/read
```

## Optional Structural Compile Check

```sh
iverilog -g2012 \
  -o /tmp/atx_spec_020_structural_check \
  rtl/atarix/atx_audit_log_window.v \
  rtl/atarix/atx_simd_probe_core.v \
  rtl/atarix/atx_scalar_probe_core.v \
  rtl/atarix/atx_krapivin_stepper.v \
  rtl/atarix/atx_elias_fano_decoder.v \
  rtl/atarix/atx_northbridge_bus_shim.v \
  rtl/atarix/atx_spec_020_accelerator.v \
  rtl/atarix/atx_spec_020_system_wrapper.v
```

## Expected Accelerator Test Coverage

The accelerator simulation covers:

```text
authorized lane hit
CRC mismatch gate
ring denied gate
capability denied gate
policy denied gate
unknown message type rejection
table timeout
miss / sequence error
```

## Expected Gain Proxy

The current RTL validates the 16-lane control-byte comparison path.

```text
SIMD-style probe:
    checks 16 control bytes in one combinational probe window

Scalar baseline:
    checks one control byte per cycle until match or lane 15
```

Future benchmark work should measure:

```text
cycles per successful lookup
cycles per miss
cycles to authorization denial
cycles to audit commit
probe-limit behavior
scalar baseline cycles
SIMD probe equivalent cycles
```

## Architectural Constraints

This RTL must preserve:

```text
Lookup acceleration is not authority.
Registry identifiers are not raw physical authority.
CRC is integrity only.
No probe executes before authorization.
Audit commit precedes successful response.
Filters are never authority.
A corrupt or unverifiable index fails closed.
```
