<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:b42c0f7d-1699-4193-a783-785aaba51518
-->

# Simulator Common

Shared simulator definitions live here.

Planned files:

```text
sim_node.h
sim_types.h
sim_mailbox.h
```

This directory will contain shared node identifiers, simulator status values, and mailbox transport wrappers.

The common layer must not bypass the public ATARIX mailbox API.
