<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:9a866b5e-05ad-4a30-beda-c2604cd2011b
-->

# BIOS API v1

Hybrid firmware model. Firmware performs discovery, builds the Service Directory and Boot Information Block, then transfers control to the kernel. A small BIOS remains available for console, diagnostics, recovery, supervisor access, and firmware updates.

Service classes:
- Console
- Block I/O
- Network bootstrap
- Supervisor
- Firmware update
- Diagnostic monitor

BIOS calls return carry clear on success and carry set on failure.
