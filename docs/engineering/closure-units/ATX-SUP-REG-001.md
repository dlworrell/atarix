---
document: ATX-SUP-REG-001
title: CPU cannot command the supervisor through the register path
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - ATX-MBX-DENY-001
implements:
  - sim/cpu-card/interrupts.cpp
verified_by:
  - sim/cpu-card/negative_control.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:4bcc749e-e2f4-472e-a430-01cdb1bb706e
-->

# ATX-SUP-REG-001: CPU cannot command the supervisor through the register path

```yaml
closure_unit:
  id: ATX-SUP-REG-001
  claim: >-
    Through the $00C200 supervisor mailbox registers, the CPU card may only
    read board identity. Reset, recovery, fault-log and FPGA-reload commands
    are rejected with an explicit status, so the register path cannot bypass
    the ring and capability checks of the mailbox path.
  intent:
    authority: conflicting
    sources:
      - docs/register-map-v1.md            # lists the commands as plain register writes
      - docs/supervisor-card-v1.md
      - docs/doctrine/ARC-SEC.md            # no ambient authority; supervisor not exempt
  realizations:
    emulator: required
    firmware: required                      # T23 (Read Board ID), T26 (Reset rejected)
    supervisor_firmware: deferred
    physical_hardware: deferred
  evidence:
    emulator_tests: required
    negative_control: required              # supervisor-accepts-reset: detected
  security_impact: closes an authority bypass, if the architecture agrees
  compatibility_impact: register-map-v1 command list would change
  recovery_impact: recovery must be initiated by the supervisor or a human, not by CPU register writes
  state: DIVERGENT
```

## Divergence

`register-map-v1.md` documents Reset Request, Recovery Mode and FPGA Reload as
CPU-writable commands. The simulator rejects them because the mailbox path
denies the kernel ring any request to the supervisor ring. One of the two must
change. This needs an engineering decision.
