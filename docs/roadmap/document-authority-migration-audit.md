# ATARIX Document Authority Migration Audit

Date: 2026-07-12
Repository: `dlworrell/atarix`
Status: Active migration audit

## Purpose

This audit identifies documents created in ATARIX during the period when program governance, engineering standards, project management, and implementation material were temporarily centralized. It assigns each reviewed document to its current canonical authority.

The governing authority chain is:

```text
Catylist → AES → AEMS → governed repositories
```

## Classification Rules

- Program governance and repository authority belong in **Catylist**.
- General engineering obligations and standards belong in **AES**.
- Project-management automation, compliance scanners, evidence packaging, and enforcement belong in **AEMS**.
- ATARIX architecture, hardware, FPGA, firmware, operating-system, toolchain, and project-specific evidence remain in **Atarix**.
- Company and public-facing material belongs in **Just-a-Geek-LLC**.
- Product-specific assistant/application material belongs in **JAG**.

## Reviewed Documents

| ATARIX path | Classification | Canonical destination | Action |
|---|---|---|---|
| `docs/development-philosophy.md` | General engineering principles | `dlworrell/AES/standards/AES-DEV-001-development-principles-and-check-in-discipline.md` | Marked superseded; retained as migration history |
| `docs/engineering/ATARIX-DEV-001-development-principles.md` | ATARIX-specific development profile | Atarix | Retain; this is the local AES-DEV-001 extension |
| `docs/engineering/SECURE-C-CXX.md` | Local secure-coding profile | Atarix | Retain; this is required local adoption evidence for AES-SEC-001 |
| `docs/engineering/AES-SEC-001-waivers.md` | Local waiver record | Atarix | Retain; waivers must remain with the affected codebase |
| `docs/roadmap/repository-extraction-plan.md` | Transitional program/repository governance | Catylist | Marked superseded and rewritten as a historical pointer to canonical governance |
| `docs/diagnostic-access-v1.md` | ATARIX system interface specification | Atarix | Retain; project-specific architecture contract |
| `docs/cpu-observability-contract-v1.md` | ATARIX CPU/system observability interface | Atarix | Retain; project-specific versioned contract |

## Confirmed Migration

The following general principles from the original ATARIX development philosophy are now represented in AES-DEV-001:

- architecture before specification before implementation;
- documentation as part of the system;
- versioned interfaces;
- small, reviewable changes;
- observability and diagnostic visibility;
- security by design and explicit trust boundaries;
- cryptography by reuse rather than invention;
- failure-mode and recovery requirements;
- ADR requirements;
- explicit authority models;
- acceptance evidence for significant changes.

The original ATARIX file is therefore no longer a standards authority.

## Remaining Review Queue

The following categories still require a complete path-by-path inventory:

1. Documents under `docs/roadmap/` that describe ecosystem-wide sequencing rather than ATARIX implementation.
2. Documents under `docs/engineering/` that define general requirements rather than local ATARIX profiles or evidence.
3. Schemas, validation models, evidence models, or reusable tooling that may belong in AEMS.
4. Repository templates or general document templates that may belong in `repo_templates` or AES.
5. Company, publication, or public-site material that may belong in Just-a-Geek-LLC.
6. JAG product or assistant specifications that may belong in JAG.

## Migration Procedure

For each remaining candidate:

1. Identify whether the content is general or ATARIX-specific.
2. Confirm that the destination repository exists and its authority is explicit.
3. Copy or adapt the authoritative content into the destination repository.
4. Preserve provenance and original commit history in the destination document.
5. Replace the ATARIX source with a concise supersession notice or delete it only after all inbound references are corrected.
6. Run documentation and compliance checks in both source and destination repositories.
7. Record the migration in this audit.

## Current Conclusion

Two clear migration remnants were found and corrected:

- the general development philosophy now points to AES;
- the obsolete repository extraction plan now points to Catylist, AES, and AEMS according to the accepted authority chain.

The reviewed diagnostic and observability contracts correctly remain in Atarix because they define ATARIX-specific interfaces rather than ecosystem-wide standards.
