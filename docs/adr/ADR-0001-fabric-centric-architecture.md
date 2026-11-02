<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:522e3dc8-b9d7-45b5-b1f3-a0c3ae82d1b5
-->

# ADR-0001 Fabric-Centric Architecture

Status: Accepted

Date: 2026-06-11

## Context

The project must support future processor cards, capability-mediated services, and a workstation-class expansion model.

## Decision

The W65C816 local bus remains local to the CPU card.

All system-wide communication occurs through the Fabric Northbridge.

## Consequences

Benefits:

- Heterogeneous processor support
- Stable service model
- Improved observability
- Easier DMA integration

Costs:

- Additional latency
- More FPGA complexity
- More firmware infrastructure

## Future Revisions

Refine mailbox, DMA, and service contracts.
