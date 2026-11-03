<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:82ed3ef4-4217-488f-b827-9ec7cb330a22
-->

# ADR-0003: Local CPU Bus

Status: Accepted

## Context
Should the W65C816 bus be exposed across the backplane?

## Decision
No. The CPU bus remains local to the CPU card.

## Evidence
- Improved signal integrity
- Better scalability
- Easier SMP evolution
- Cleaner DMA architecture

## Alternatives Considered
- Shared raw bus
- VME-style architecture
- Direct backplane extension

## Consequences
Requires bridge logic and fabric interfaces.

## Future Revisions
Revisit only if evidence supports a shared-bus design.