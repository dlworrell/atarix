# Repository Extraction Plan

Status: Superseded transitional plan

## Canonical Authority

Repository governance and authority boundaries are now defined by:

- `dlworrell/Catylist/docs/adr/ADR-003-authority-chain.md`
- `dlworrell/Catylist/docs/architecture/governance-model.md`
- `dlworrell/Catylist/docs/architecture/repository-taxonomy.md`

Engineering standards are owned by:

- `dlworrell/AES`

Project management, assessment, evidence packaging, and standards enforcement are owned by:

- `dlworrell/AEMS`

This file remains as a historical record of the earlier repository-splitting plan. It is not authoritative.

## Corrected Authority Boundaries

### Catylist

Catylist owns:

- Catalyst program governance;
- repository relationships and taxonomy;
- authority boundaries;
- governance ADRs;
- lifecycle and decision processes.

### AES

AES owns:

- engineering principles and standards;
- required engineering evidence;
- standard templates and normative terminology;
- secure coding and development requirements;
- engineering-standard ADRs and revision history.

### AEMS

AEMS owns:

- Catalyst project-management automation;
- repository inventory and classification;
- compliance scanners;
- evidence packaging and reporting;
- dashboards and enforcement mechanisms;
- schemas used to express scanner and evidence output.

AEMS verifies AES. It does not define AES requirements.

### Atarix

Atarix owns:

- ATX architecture specifications;
- hardware, FPGA, firmware, operating-system, and toolchain design;
- ATX-100 canonical architecture material;
- project-specific requirements, ADRs, tests, and verification evidence;
- Atarix-specific extensions of AES requirements.

### JAG

JAG owns:

- application and assistant behavior;
- repository-analysis and architecture-review tools specific to JAG;
- AEMS-aware workflow integrations;
- product-specific specifications and evidence.

### Just-a-Geek-LLC

Just-a-Geek-LLC owns:

- company and public-facing material;
- website and publication content;
- organizational history and portfolio material.

It does not own Catalyst engineering governance or standards.

## Current Dependency Rule

```text
Catylist → AES → AEMS → governed repositories
```

Atarix and other implementation repositories may consume AEMS tooling and reference AES and Catylist documents. Circular policy dependencies are prohibited.

## Historical Extraction Outcome

The major repository separation described by the original plan has occurred:

- Catylist exists as the program governance repository;
- AES exists as the engineering standards repository;
- AEMS exists as the project manager and enforcement orchestrator;
- JAG and Just-a-Geek-LLC exist as separate repositories;
- Atarix remains the system architecture and implementation repository.

Remaining document migration is tracked in:

- `docs/roadmap/document-authority-migration-audit.md`
