<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:67c7ff54-eea3-4ce8-8fd4-318f4e61e86e
-->

# Architecture Review 004

## Status

Review complete. Findings open.

Machine-assisted review (Claude Code, requested by the owner on 2026-11-06). Not a substitute for owner review.

## Scope

Reviewed against `main` at `9f9d24fd41cd4ad23b5ffdf4ecd3e73c7c1da89a` (after release candidates #123 and #124).

Read in full:

```text
ATX-100 index and chapters 01, 09, 15, 20
ATX-SPEC-001 through ATX-SPEC-011
ATX-SPEC-013, 014, 016, 018, 019
ATX-SPEC-020 Hardware Mailbox Integration Notes
ADR-00X Capability Evaluation Model
ADR-RING-SECURITY-MODEL (rings)
Architecture Review 002 and 003
```

Read in part (headings and normative statements):

```text
ATX-SPEC-020 Hash Table and Lookup Acceleration Model
ATX-SPEC-021 Memory and Data Movement Model
ATX-SPEC-090, 091, 092
ADR-00Y, ADR-00Z, ADR-THREE-FABRIC-ARCHITECTURE
rtl/atarix/atx_spec_020_accelerator.v and its CI testbench
include/atarix/index.h
```

Checked mechanically across all 155 Markdown files under `docs/`: document identifiers, status lines, cross-references to ATX-SPEC and ADR identifiers, and relative Markdown links.

Not reviewed: ATX-SPEC-015 (covered by Architecture Review 003), hardware, boot, and card documents outside `docs/architecture/` and `docs/adr/`, and the research tree.

Also reviewed: release candidate #123 (document headers and census licence-text rule), recorded at the end.

## Executive Summary

The core doctrine is coherent and repeated consistently across ATX-SPEC-001 through ATX-SPEC-019: name is not identity, identity is not authority, lookup is not access, observation is not control, recovery does not restore authority, and unknown state fails closed.

The open problems are reconciliation gaps between document families, not flaws in the doctrine:

- The access-decision order is stated four different ways.
- The ADRs and the ATX-SPEC series use different fabric vocabularies.
- Document identifiers are reused, so ATX-100's single-canonical-home rule does not hold yet.
- Two foundation specifications that other specifications depend on do not exist in the repository.
- The ATX-SPEC-020 accelerator RTL accepts an expected generation but never checks it.

## Review Result

```text
PASS WITH FINDINGS
Blocking for implementation of the evaluation path: Finding 001
Blocking for ATX-SPEC-020 RTL claims: Finding 006
All other findings: clarification or housekeeping
```

## Findings

### Finding 001: The Access-Decision Order Is Defined Four Ways

The same decision chain appears with different steps and order:

```text
ATX-SPEC-001  identity -> ring -> ownership -> capability -> revocation
              -> expiration -> lifecycle -> resource policy -> operation
ATX-SPEC-008  lookup -> identity -> ring -> ownership -> capability
              -> lifecycle -> resource policy -> operation
ADR-00X       identity -> trust -> revocation -> ring -> ownership
              -> capability -> resource -> operation
ATX-SPEC-020  CRC -> ring -> capability -> policy -> lookup
  (HW notes)
```

Differences:

- ADR-00X checks revocation before ring; ATX-SPEC-001 checks it after capability.
- ADR-00X has a trust step that ATX-SPEC-001 and ATX-SPEC-008 do not.
- ATX-SPEC-001 has expiration and lifecycle steps that ADR-00X does not; ATX-SPEC-008 omits revocation and expiration.
- ATX-SPEC-013 makes policy a distinct decision ("a valid capability describes what could be done; policy decides whether it may be done now"), but no chain except the ATX-SPEC-020 hardware pipeline shows a policy step.

Order matters for fail-closed behaviour, for which error is reported, and for what an early step can leak (for example, whether a revoked caller learns that a target exists).

Recommendation: define one normative evaluation order in a single canonical home (ATX-SPEC-001 or ATX-SPEC-013), include trust, revocation, expiration, lifecycle, and policy explicitly, and have ADR-00X, ATX-SPEC-008, and ATX-SPEC-020 reference it rather than restate it.

### Finding 002: Two Fabric Vocabularies With No Mapping

The ATX-SPEC series (001, 002, 008, 009, 013, 016) describes two fabric classes:

```text
Operational Fabric
Supervisor Management Fabric
```

The accepted ADRs (ADR-THREE-FABRIC-ARCHITECTURE, ADR-RING-SECURITY-MODEL) describe three:

```text
Management Fabric
Enumeration Fabric
Transport Fabric
```

No document maps one set onto the other. A reader cannot tell whether Enumeration and Transport are planes of the Operational Fabric (ATX-SPEC-008 mentions "discovery and service planes"), or whether Management Fabric and Supervisor Management Fabric are the same thing.

Recommendation: add the mapping to ATX-SPEC-016 or to an ATX-100 chapter, and use one vocabulary in normative text.

### Finding 003: Ring Assignment Of Services Differs

ADR-00X places Services at ring 2 and Applications at ring 3. ADR-RING-SECURITY-MODEL places drivers and privileged runtime services at ring 1, user-space services at ring 2, and applications at ring 3.

Recommendation: make ADR-RING-SECURITY-MODEL canonical for ring membership and correct ADR-00X to "privileged services (1) / user services (2)".

### Finding 004: Root Of Trust Is Stated Two Ways

ADR-00X and ADR-00Z state that the Supervisor is the root of trust ("Supervisor Owns Trust"). ATX-SPEC-014 starts the chain of trust at "Root of Trust / ROM -> Supervisor Boot Firmware". Both can be true (ROM is the hardware root, the Supervisor owns runtime trust), but neither document says so.

Recommendation: state the distinction once in ATX-SPEC-014 and reference it from the ADRs.

### Finding 005: Document Identifiers Are Reused

ATX-100 requires that each concept has exactly one canonical home. Today:

```text
ADR-0001        ADR-0001-ulx3s-selection.md
                ADR-0001-fabric-centric-architecture.md
ATX-SPEC-020    ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md
                ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md
ADR-00X/Y/Z     Accepted, but not numbered
```

Near-duplicates remain as drafts beside their successors, without supersession metadata:

```text
docs/core-doctrines.md            ~ ADR-00Z (same text, different status)
docs/capability-model.md          ~ ATX-SPEC-003
docs/data-model-and-endianness.md ~ data-model-and-endianness-v1.md
docs/discovery-rom-format.md      ~ discovery-rom-format-v1.md
```

Recommendation: renumber the second ADR-0001, number ADR-00X/Y/Z, give the ATX-SPEC-020 notes a sub-identifier (for example ATX-SPEC-020-N1), and mark the older drafts superseded with a pointer, per the ATX-100 migration model (reduce or archive, do not delete).

### Finding 006: ATX-SPEC-020 Accelerator Ignores The Expected Generation

`include/atarix/index.h` and the RTL accept `expected_generation` in an index query. `rtl/atarix/atx_spec_020_accelerator.v` only prints it; it never compares it with `table_generation`. A query pinned to generation 4 succeeds against a table at generation 5 and reports generation 5.

The specifications rely on generation pinning: ATX-SPEC-007 and ATX-SPEC-008 tell clients that need stability to pin a binding generation, and ATX-SPEC-020 requires "generation-aware behavior" and that direct identity lookup "validate lifecycle, generation, authority, and policy". The CI testbench always uses equal generations, so this is untested.

Recommendation: add a generation-mismatch status (or define 0 as "any generation" and reject other mismatches), and add a mismatch case to `atx_spec_020_accelerator_ci_tb.v`.

### Finding 007: ATX-SPEC-020 Notes Contradict ATX-100 Chapter 15

ATX-100 chapter 15 states that the RTL simulations validate correctness and a scalar-versus-SIMD gain proxy, and the ATX-SPEC-020 RTL CI passes. The ATX-SPEC-020 Hardware Mailbox Integration Notes still say that "the RTL sketches discussed so far are not yet a verified implementation" and that `include/atarix/index.h` was "not found", although it now exists.

Recommendation: update or archive the notes so they describe the implemented state, and keep the open RTL correction items (for example the CRC parameter confirmation) as tracked issues.

### Finding 008: Foundation Specifications Are Missing

ATX-SPEC-012 Versioning Model is referenced by 17 specifications (003, 005 to 011, 013 to 016, 018 to 021, 092) and ATX-SPEC-017 Storage and Persistence Model by 13 (006, 008 to 011, 013 to 016, 018 to 021). Neither file exists. `architecture-backlog.md` lists both as planned, ATX-SPEC-012 at high priority and first in its sequence, yet ATX-SPEC-013 to 021 were written after it, and several of them state that versioning and persistence behaviour "follows" or is "defined in" these specifications. Architecture Review 002 also lists both in its scope.

Recommendation: write ATX-SPEC-012 next, as the backlog already orders it, and until then mark downstream references as pending so they are not read as satisfied.

### Finding 009: Cleanup Escalation Levels Collide

ATX-SPEC-013 defines cleanup levels L1 to L8 (L5 is "Recovery Cleanup"). ATX-SPEC-018 defines recovery cleanup levels L1 to L6 (L1 is "Recovery Cleanup"). The same label means different steps in the two ladders.

Recommendation: prefix the ladders (for example PCL1..PCL8 and RCL1..RCL6) or define the recovery ladder as a sub-ladder of ATX-SPEC-013 L5 and above. This extends Architecture Review 002 Finding 003 (lifecycle state names).

### Finding 010: ATX-100 Is Mostly An Outline

Four of 22 ATX-100 chapters exist (1, 9, 15, 20). ATX-SPEC-001 to 011 are Draft v0.2 and 013 to 021 are Draft v0.1. The migration model (stable material moves into ATX-100) has started but is not yet the canonical entry point it describes.

Recommendation: no action required now; use Findings 001 to 005 as the first integration candidates, since they are exactly the cross-document concepts ATX-100 is meant to own.

## Release Candidate #123

Merged 2026-11-05. Additive only: 158 files.

- 154 Markdown files gained the three-line SPDX header (copyright, Apache-2.0, Catalyst file ID). All IDs are valid UUIDv4 and none is duplicated in the repository. In the 15 files with YAML front matter, the header follows the front matter, so the metadata still parses.
- `LICENSES/Apache-2.0.txt` added; the document authority census accepts REUSE licence texts directly under `LICENSES/` and still rejects anything else there. New tests cover both cases and pass; the strict census passes on `main`.
- Not architectural; no conflict with any specification.
- Nits: 17 Markdown files still lack headers (code of conduct, reviews, research, `docs/test-write.md`); confirm whether they are intentionally held. The copyright line has no year, unlike other Catylist repositories.

Result: PASS.

## Major Strengths

- Authority separation is consistent across every specification read.
- Policy is positioned correctly as a separate decision from capability possession.
- Recovery, bootstrap, and supervisor authority are each scoped and cannot silently become runtime authority.
- The accelerator RTL performs ring, capability, and policy checks before any probe is issued, matching the ATX-SPEC-020 pipeline rule that membership probing must not precede authorization.
