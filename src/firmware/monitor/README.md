<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:d1e156a6-3c35-49d8-81c9-f4e160a0d21a
-->

# ATARIX ROM Monitor

This directory contains the ATARIX ROM monitor, written for ca65 (cc65) against [ATX-SPEC-022](../../../docs/architecture/ATX-SPEC-022-CPU-Card-Platform-Contract.md) EP-A.

Build the 64 KiB boot ROM with `src/firmware/monitor/build.sh [output-directory]`. To run it, see [the MAME driver](../../../src/emulator/mame/README.md).

Milestone M0 implements the reset path, the boot banner, RAM sizing and a console echo loop.

Initial goals:

```text
RESET
  -> initialize CPU state
  -> initialize stack and direct page
  -> initialize serial console
  -> print monitor banner
  -> enter command loop
```

Initial monitor commands are expected to include memory dump, memory edit, device list, service list, boot, and reset.
