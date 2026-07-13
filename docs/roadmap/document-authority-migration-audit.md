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
| `project-zero/README.md` | Obsolete duplicate Project Zero entry point | `docs/roadmap/atarix-project-zero-inventory.md` | Removed after migration |
| `rtl/atarix/ATX-SPEC-020-CI.md` | Misplaced verification specification | `docs/verification/ATX-SPEC-020-rtl-ci.md` | Moved to canonical documentation tree |
| `tools/aems/README.md` | AEMS-owned project documentation | `dlworrell/AEMS` | Removed from Atarix after extraction |

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

## Full-Tree Census

The document-authority census scans the complete repository rather than only `docs/`.

The initial v1 run found 165 documentation-like files and reported 18 files outside the approved documentation roots. Manual review showed that most of those 18 were legitimate component-local README files colocated with implementation, simulation, test, firmware, RTL, or interface code.

The v2 placement model therefore distinguishes:

- **canonical** documentation under `docs/`, `.github/`, or approved repository-root paths;
- **colocated** component README files beside the implementation they explain;
- **violation** paths containing misplaced normative, cross-project, or migrated documentation.

The three genuine migration violations identified by the v1 run were:

1. `project-zero/README.md`;
2. `rtl/atarix/ATX-SPEC-020-CI.md`;
3. `tools/aems/README.md`.

All three are now represented as forbidden historical paths in the v2 census and have canonical destinations recorded above.

## Remaining Review Queue

The location census is now mechanically enforceable. Semantic review remains necessary for documents inside `docs/` whose wording may still describe the wrong authority. Priority categories are:

1. `docs/architecture/ATX-SPEC-090-Atarix-Engineering-Management-System.md` and related AEMS-era material;
2. `docs/architecture/ATX-SPEC-091-Requirements-and-Traceability-Model.md`;
3. general documentation taxonomy, engineering-gate, review-template, and doctrine files that may belong in AES or AEMS;
4. repository-organization and ecosystem-sequencing documents that may belong in Catylist;
5. product-assistant material that may belong in JAG.

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

The full-tree location policy has been corrected and the three genuine off-tree migration remnants have canonical destinations. The next workflow run must use census schema `catalyst.atarix.document-authority-census.v2`; a green strict result is the evidence required to close the location portion of the migration.

The broader semantic migration is not yet complete until the remaining AEMS-, AES-, Catylist-, and JAG-oriented documents inside `docs/` are reviewed and either migrated or explicitly retained as ATARIX-local profiles or historical records.
