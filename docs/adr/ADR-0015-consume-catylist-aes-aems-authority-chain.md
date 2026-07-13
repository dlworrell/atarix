# ADR-0015: Consume the Catylist, AES, and AEMS Authority Chain

Status: Accepted
Date: 2026-07-13
Owner: ATARIX

## Context

ATARIX temporarily accumulated program governance, general engineering standards, project-management tooling, implementation documentation, and evidence in one repository while the wider Catalyst repository structure was being established.

The accepted authority chain is now:

```text
Catylist -> AES -> AEMS -> governed repositories
```

Catylist defines program governance and repository authority. AES defines normative engineering standards. AEMS implements, measures, reports, and enforces those standards. ATARIX is a governed implementation repository.

## Decision

ATARIX shall consume upstream governance and standards rather than redefine them locally.

ATARIX retains only:

- ATARIX architecture and design;
- hardware, FPGA, firmware, operating-system, protocol, and toolchain specifications;
- local profiles that extend AES requirements without weakening them;
- project-specific ADRs;
- implementation, tests, verification, and evidence;
- explicit local waivers tied to ATARIX code or architecture.

General governance belongs in Catylist. General engineering obligations belong in AES. Reusable compliance scanners, evidence packaging, CI generation, and repository-management automation belong in AEMS.

## Local Profiles

Local documents such as `ATARIX-DEV-001-development-principles.md`, `SECURE-C-CXX.md`, and the ATARIX waiver register remain valid as project-specific adoption and extension records. They shall reference, not replace, their upstream AES standards.

## Evidence Direction

Authority flows downward into ATARIX through Catylist, AES, and AEMS. ATARIX produces evidence upward through AEMS, including architecture records, tests, RTL simulations, static analysis, traceability, waivers, and compliance reports.

## Consequences

### Positive

- ATARIX documentation remains focused on how the ATARIX system works.
- General policy has a single canonical owner.
- AEMS can validate ATARIX against versioned AES requirements.
- Document migration and future authority disputes have deterministic resolution.
- Local component documentation may remain beside implementation when it is descriptive rather than normative cross-project policy.

### Negative

- ATARIX depends on the availability and versioning discipline of upstream governance, standards, and tooling.
- Existing references to migrated documents must be maintained during transition.
- Local engineering gates must be updated when canonical paths change.

## Alternatives Considered

### Keep all authority inside ATARIX

Rejected because it duplicates policy, creates circular authority, and prevents reuse across other Catalyst repositories.

### Make AEMS the standards authority

Rejected because executable enforcement should not become the source of normative requirements. AEMS executes Catylist and AES authority.

### Allow each project to define independent standards

Rejected because it prevents consistent evidence, compliance comparison, and shared tooling.

## Validation

The full-tree document-authority census and engineering gate shall verify that:

- misplaced normative documents do not reappear outside approved authority locations;
- local component READMEs are distinguished from cross-project policy;
- migrated canonical documents are referenced by current paths;
- upstream standards and local profiles remain traceable.
