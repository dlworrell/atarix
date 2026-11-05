<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:1462f90e-bbfe-405e-94de-9bcd45c4c8b9
-->

# Supervisor Simulator

The Supervisor simulator represents node `0x0000`.

Initial responsibilities:

```text
Generate PING
Generate VERSION_QUERY
Receive PONG
Receive VERSION_REPLY
```

The Supervisor simulator must use mailbox validation and must not directly mutate Fabric state.
