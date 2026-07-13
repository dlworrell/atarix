# ATARIX Development Philosophy

Status: Superseded as a general engineering standard

## Canonical Authority

The general engineering principles originally captured here have been migrated into:

- `dlworrell/AES/standards/AES-DEV-001-development-principles-and-check-in-discipline.md`

ATARIX-specific extensions are maintained in:

- `docs/engineering/ATARIX-DEV-001-development-principles.md`

This file remains as a historical migration record. It is not the canonical source for Catalyst-wide engineering requirements.

## Original Purpose

This document originally defined how ATARIX was developed and supplied source material for the Catalyst engineering standard.

The original principles included:

- architecture before specification before implementation;
- versioned interfaces;
- small commits, pull requests, and reviews;
- observable systems;
- security by design;
- cryptography by reuse rather than invention;
- explicit recovery behavior;
- architecture decision records;
- explicit ring and capability authority models;
- preference for understandable, observable, auditable, recoverable, and secure systems over clever systems.

## Current Ownership

General requirements belong in AES.

ATARIX retains only project-specific extensions, including:

- the Vega816, BB816, Sun/NuBus/UPA, curl, and OpenBSD lineage;
- ATARIX protocol and interface examples;
- ATARIX-specific hardware, FPGA, firmware, operating-system, and toolchain expectations;
- ring and capability authority details;
- project-specific ADR examples.

## Migration Rule

Future general engineering requirements discovered during ATARIX work must be proposed in AES rather than added here. ATARIX may reference and extend AES locally, but it must not become a second standards authority.
