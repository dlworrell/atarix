<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:4beae852-16e5-476a-87e2-230a0b43464e
-->

# ATARIX Source Tree

This directory contains implementation source code for the ATARIX project.

Planned layout:

```text
src/
├── firmware/
│   ├── monitor/
│   ├── bios/
│   └── discovery/
├── kernel/
│   ├── bootstrap/
│   └── services/
├── emulator/
│   ├── fabric/
│   ├── discovery/
│   └── mailbox/
└── common/
```

The architecture and specifications in `docs/` are intended to drive the implementation contained under this tree.
