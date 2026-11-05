<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:69c2bd04-891e-4258-9483-ce04a420a1ad
-->

# Fabric Simulator

The Fabric simulator initially represents node `0x0001`.

Initial responsibilities:

```text
Receive PING
Reply PONG
Receive VERSION_QUERY
Reply VERSION_REPLY
```

No Discovery, Directory, Capability transport, resource model, or device model is implemented in the skeleton.

The Fabric simulator must use mailbox validation and must not expose direct address/register shortcuts to callers.
