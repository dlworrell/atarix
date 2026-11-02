<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:5445bcc3-d5ec-41b0-8f7c-cad4a2332f3d
-->

# ADR-0001: ULX3S LFE5U-85F Selection

Status: Accepted

## Context
Need a fabric controller platform.

## Decision
Use ULX3S with LFE5U-85F.

## Evidence
- Open toolchain support
- Large FPGA resource budget
- Existing community support
- Suitable for rapid prototyping

## Alternatives Considered
- Custom FPGA board
- Smaller ECP5 variants

## Consequences
Faster development and lower initial risk.

## Future Revisions
May migrate to custom ATARIX fabric controller card.