<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:641d414d-aff6-4ea0-87ce-5f2bbf2dc9fc
-->

# Simulator Tests

Simulator tests will validate virtual ATARIX node behavior under CI.

Initial planned tests:

```text
test_ping_pong
test_version_query
```

The tests must verify that traffic traverses the mailbox validator and does not bypass protocol validation.
