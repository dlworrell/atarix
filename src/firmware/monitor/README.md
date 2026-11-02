<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:d1e156a6-3c35-49d8-81c9-f4e160a0d21a
-->

# ATARIX ROM Monitor

This directory will contain the first ATARIX ROM monitor implementation.

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
