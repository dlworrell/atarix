<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:40a7b99e-040a-44c7-bdab-265b28ca708a
-->

# Simulator Mailbox

The simulator mailbox layer wraps the public ATARIX mailbox protocol.

It must validate:

```text
Header
Length
CRC
Sequence
Message type
```

before any virtual node receives a message.

The simulator mailbox layer is not allowed to provide direct delivery shortcuts that bypass protocol validation.
