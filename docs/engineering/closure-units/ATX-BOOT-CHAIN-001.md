---
document: ATX-BOOT-CHAIN-001
title: The supervisor runs the boot authority chain before the CPU card leaves reset
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - Architecture-Review-004
implements:
  - sim/cpu-card/supervisor.cpp
  - sim/cpu-card/northbridge.cpp
  - src/identity/identity_validate.c
verified_by:
  - sim/cpu-card/negative_control.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:d746c4c2-b730-422c-a8bd-deb8f052ca39
-->

# ATX-BOOT-CHAIN-001: The supervisor runs the boot authority chain before the CPU card leaves reset

```yaml
closure_unit:
  id: ATX-BOOT-CHAIN-001
  claim: >-
    While the CPU card is held in reset, the supervisor validates every
    card's Identity EEPROM, the fabric enumerates the declared services, the
    supervisor's policy (not discovery) grants capabilities by service type
    and provider trust, the kernel's ring-filtered directory is published at
    Discovery Services, and endpoints are activated. Only then is the CPU
    released. The CPU binds services by type from the directory; no service
    node id is compiled into its firmware.
  intent:
    authority: normative
    sources:
      - docs/ADR-RING-SECURITY-MODEL.md             # boot-time authority chain
      - docs/ADR-THREE-FABRIC-ARCHITECTURE.md       # boot sequence; enumeration describes
      - docs/identity-eeprom-v1.md                  # validation steps
      - docs/service-directory-format-v1.md         # ring filtering
  realizations:
    emulator: required
    firmware: required                              # T20 binding, T36-T37, T47
    supervisor_firmware: deferred
    fabric_rtl: deferred
    physical_hardware: deferred
  evidence:
    emulator_tests: required                        # host checks the 15 boot events in order
    negative_controls: required                     # release-before-enumeration, grant-by-discovery, skip-identity-validation, list-platform-rings: all detected
  security_impact: >-
    identity is not authority; enumeration is not authority; a card that
    fails identity is quarantined; a homebrew card is identified, not
    validated
  compatibility_impact: Discovery Services register profile extends register-map-v1 (G-BOOT-1)
  recovery_impact: none in the bench; supervisor recovery paths are not modeled
  state: VERIFYING
```

## Open decisions

Gaps G-BOOT-1 to G-BOOT-7 in
[Architecture Review 004](../../reviews/Architecture-Review-004.md): the
Discovery Services profile, identifier widths, whether a failed card is
listed, trust mapping, the boot capability policy, a board class for display
cards, and boot-chain audit events.
