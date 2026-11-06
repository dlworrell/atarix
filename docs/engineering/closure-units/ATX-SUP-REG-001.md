---
document: ATX-SUP-REG-001
title: CPU requests supervisor actions only through the Supervisor Request Gateway
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - ATX-MBX-DENY-001
  - ATX-SPEC-016-Supervisor-Management-Fabric
  - Architecture-Review-004
implements:
  - sim/cpu-card/supervisor.cpp
verified_by:
  - sim/cpu-card/negative_control.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:4bcc749e-e2f4-472e-a430-01cdb1bb706e
-->

# ATX-SUP-REG-001: CPU requests supervisor actions only through the Supervisor Request Gateway

```yaml
closure_unit:
  id: ATX-SUP-REG-001
  claim: >-
    The $00C200 block is a Supervisor Request Gateway (ATX-SPEC-016). Every
    CPU write to COMMAND is a request, decided by the supervisor's command
    policy, answered with an ATX-SPEC-016 result code, signalled by the
    Supervisor interrupt, and recorded as supervisor audit events. The caller
    and its ring come from the slot. A kernel reset request is not executed.
  intent:
    authority: normative
    sources:
      - docs/architecture/ATX-SPEC-016-Supervisor-Management-Fabric.md  # a request is not a command
      - docs/doctrine/ARC-SEC.md            # no ambient authority; supervisor not exempt
      - docs/register-map-v1.md            # register block; still lists commands, see gap G-SUP-1
  realizations:
    emulator: required
    firmware: required                      # T23/T24 Read Board ID, T26 Reset, T41-T46
    supervisor_firmware: deferred
    physical_hardware: deferred
  evidence:
    emulator_tests: required                # host checks 8 gateway results and 32 supervisor events
    negative_control: required              # supervisor-accepts-reset: detected
  security_impact: closes the register-path authority bypass
  compatibility_impact: register-map-v1 gains RESULT (+06), writable ARG, and proposed command 06
  recovery_impact: recovery entry is denied to the kernel; who may request it is open (G-SUP-4)
  state: VERIFYING
```

## History

Previously DIVERGENT: `register-map-v1.md` documented Reset Request, Recovery
Mode and FPGA Reload as CPU-writable commands, which conflicted with the
mailbox path's ring checks. ATX-SPEC-016 resolves the conflict: the
operational fabric requests and the supervisor decides. The bench now
implements that.

## Open decisions

Gaps G-SUP-1 to G-SUP-7 in
[Architecture Review 004](../../reviews/Architecture-Review-004.md): register
profile adoption, numeric result and command encoding, the command table,
recovery and reset callers, the fault log format, and caller identity.
