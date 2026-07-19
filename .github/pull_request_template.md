# Pull Request

## Summary

Describe the bounded engineering change and why it is needed.

## Closure Unit

List the closure-unit identifier or identifiers advanced by this pull request.

- `ATX-...`

## Intent

Identify the authoritative architecture, specification, doctrine, requirements, or acceptance criteria.

- Authority:
- Requirement IDs:
- Source paths:

## Realization

- [ ] Public header or schema
- [ ] Firmware
- [ ] Emulator or reference model
- [ ] FPGA RTL
- [ ] Kernel or operating-system consumer
- [ ] Supervisor or management plane
- [ ] Tooling
- [ ] Physical hardware
- [ ] Documentation only
- [ ] Not applicable

Describe the realization changes:

## Evidence

- [ ] E0 — structural checks
- [ ] E1 — component tests or proofs
- [ ] E2 — emulator or reference comparison
- [ ] E3 — RTL simulation, synthesis, or realization checks
- [ ] E4 — differential verification
- [ ] E5 — physical hardware evidence
- [ ] E6 — system or end-to-end evidence
- [ ] Evidence unaffected; explanation provided below

List test paths, commands, workflow runs, measurements, results, and limitations.

## Security, Compatibility, Persistence, and Recovery

- Security impact:
- Compatibility impact:
- Persistence or format impact:
- Recovery impact:
- Migration required:

## Divergence and Gaps

List missing required realizations, evidence, unresolved disagreements, assumptions, and explicitly deferred work.

## Closure State

- [ ] PROPOSED
- [ ] OPEN
- [ ] VERIFYING
- [ ] DIVERGENT
- [ ] CLOSED
- [ ] SUPERSEDED
- [ ] INDETERMINATE

State the exact scope of the closure claim:

## AES-DEV-001

- [ ] Governing architecture, specification, or ADR is linked or updated.
- [ ] Documentation is updated in this change series, or the reason it is not required is stated.
- [ ] Tests are included, or a test rationale is provided.
- [ ] Interface and versioning impact is addressed.
- [ ] Failure modes, recovery, observability, and diagnostics are addressed where applicable.
- [ ] The change is one logical, reviewable unit suitable for revert and `git bisect`.

## AES-SEC-001

- [ ] Trust boundaries and authority crossings are identified where applicable.
- [ ] External lengths, indices, serialized values, allocation arithmetic, and signed conversions are validated.
- [ ] Banned unsafe interfaces are not introduced.
- [ ] Unsafe operations are isolated and their safety invariant is documented.
- [ ] Static analysis, warning-clean build, sanitizer, and fuzz evidence is supplied where applicable.
- [ ] Custom cryptography is not introduced.
- [ ] Any exception is recorded in the repository waiver log.

## Closure Review Checklist

- [ ] Normative intent is distinguished from descriptive or recovered behavior.
- [ ] Evidence supports the stated claim rather than only temporary implementation details.
- [ ] CPU-card and FPGA-specific details have not leaked into public architecture without an explicit decision.
- [ ] Security, ownership, lifecycle, cleanup, audit, and recovery obligations were considered.
- [ ] Generated and AI-assisted artifacts identify their provenance.
- [ ] Deferred work is not mislabeled as not applicable.
- [ ] A CLOSED declaration has no unresolved required gap within its stated scope.