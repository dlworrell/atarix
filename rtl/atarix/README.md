# Atarix RTL Scaffolds

## ATX-SPEC-020 Lookup Accelerator

Files:

```text
rtl/atarix/atx_spec_020_accelerator.v
rtl/atarix/atx_spec_020_accelerator_tb.v
```

## Purpose

This scaffold validates the ATX-SPEC-020 hardware integration direction:

```text
Mailbox request
  -> CRC gate
  -> ring gate
  -> capability gate
  -> policy gate
  -> registry-backed lookup
  -> bounded SIMD-style control-byte probe
  -> audit event
  -> response
```

The simulation is intentionally verbose. It prints one log line per start, audit, response, and test result.

## Run With Icarus Verilog

From repository root:

```sh
iverilog -g2012 \
  -o /tmp/atx_spec_020_accelerator_tb \
  rtl/atarix/atx_spec_020_accelerator.v \
  rtl/atarix/atx_spec_020_accelerator_tb.v

vvp /tmp/atx_spec_020_accelerator_tb
```

The testbench also emits:

```text
atx_spec_020_accelerator_tb.vcd
```

which can be opened in GTKWave.

## Expected Test Coverage

The initial simulation covers:

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

The current scaffold validates the 16-lane control-byte comparison path.

This is not yet a full benchmark.

The first expected performance gain proxy is:

```text
1 vector-style probe checks 16 control bytes
instead of a scalar 16-step control-byte loop
```

Future work should add a scalar baseline module and measure:

```text
cycles per successful lookup
cycles per miss
cycles to authorization denial
cycles to audit commit
probe-limit behavior
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
