<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:986c5823-c952-4840-80a9-aa994403ad75
-->

# Atarix Architectural Pillars

## Purpose

This document defines the architectural pillars upon which the Atarix operating system is built.

All subsystems shall identify which pillars they depend upon, which pillars they modify, and which pillars they must not violate.

## Foundational Pillars

- Object Model
- Namespace Model
- Security Model
- Authority Model

## Core Runtime Pillars

- Capability Model
- Mailbox Model
- Lifecycle Model
- State Model
- Resource Model

## Operational Pillars

- Error Model
- Audit Model
- Time Model
- Versioning Model
- Policy Model

## Engineering Pillars

- Portability Model
- Test Model
- Development Process

## Core Rule

Architecture defines implementation. Implementation shall not silently redefine architecture.
