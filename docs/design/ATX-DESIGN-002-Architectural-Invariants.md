---
document: ATX-DESIGN-002
title: Architectural Invariants
type: Design Document
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:487b7828-1095-4c12-8aaf-8caf3a9097e1
-->


# ATX-DESIGN-002: Architectural Invariants

## Purpose

This document records architectural properties that must remain true unless explicitly changed by formal architecture review.

## Initial Invariants

- Object identity is never authority.
- Authority derives from capabilities and policy.
- Ambient authority is prohibited.
- Compatibility personalities must not weaken native security semantics.
- Inter-object communication occurs through defined transport mechanisms.
- State transitions must be auditable.
- Hardware acceleration may speed checks but must not bypass checks.
- Engineering changes require specification, review, verification, and gate evidence.

## Relationship To ATX-100

Stable invariants should be integrated into ATX-100 as canonical architectural constraints.
