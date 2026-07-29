---
document: ATX-SPEC-092
title: Engineering Release and Configuration Management
type: Specification
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-090
  - ATX-SPEC-091
verified_by:
  - create-engineering-tag.yml
implements:
  - .github/workflows/create-engineering-tag.yml
---

# ATX-SPEC-092: Engineering Release and Configuration Management

## Purpose

This specification defines how Atarix establishes, names, verifies, records, reproduces, supersedes, and retains engineering baselines and releases.

A release is not merely a Git tag. It is a verified configuration state connected to architecture, specifications, implementation, tests, engineering gates, artifacts, and provenance evidence.

## Core Rule

**A release identity is evidence of a verified configuration, not authority to alter that configuration.**

Release records, tags, manifests, and artifacts describe a selected engineering state. They do not replace capability, policy, lifecycle, review, or repository authority.

## Scope

ATX-SPEC-092 applies to:

- Engineering Gate baselines
- Architecture and specification milestones
- Source releases
- FPGA and firmware bitstream releases
- Tooling and simulator releases
- Generated documentation releases
- Configuration manifests
- Build and verification artifacts
- Reproducibility evidence
- Supersession and rollback records

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-090 Atarix Engineering Management System
- ATX-SPEC-091 Requirements and Traceability Model

ATX-SPEC-090 manages release records and generated evidence.

ATX-SPEC-091 connects requirements, implementation, verification, gates, and releases.

ATX-SPEC-092 defines the release and configuration-management semantics applied to that evidence chain.

## Configuration Baseline

A configuration baseline is an immutable reference to a selected repository state and its associated evidence.

A baseline SHALL identify at minimum:

- Repository identity
- Commit SHA
- Ref or branch from which the baseline was selected
- Annotated tag name
- Tag annotation
- Creation timestamp
- Creating actor or automation identity
- Engineering Gate result
- Applicable specification and architecture identifiers
- Artifact manifest, when artifacts are produced
- Toolchain identity sufficient to interpret the verification result

A baseline MAY additionally identify:

- Compiler and simulator versions
- Operating-system runner image
- FPGA synthesis tool versions
- Dependency-lock manifests
- Generated-document hashes
- Hardware target and board revision
- Reproduction instructions

## Release Classes

Atarix defines the following initial release classes.

### Engineering Gate Baseline

A verified intermediate state created after the Engineering Gate passes.

Example:

```text
atx-eng-gate-2026-06-17-pass
```

### Specification Milestone

A baseline recording adoption, revision, or retirement of one or more specifications.

Example:

```text
atx-spec-021-v0.1-draft
```

### Product Release

A versioned source, firmware, RTL, tooling, or documentation release intended for broader consumption.

Example:

```text
v0.2.0
```

### Recovery Baseline

A known-good state retained to support rollback, reconciliation, or forensic comparison.

Recovery baselines SHALL remain distinguishable from ordinary development tags.

## Tag Requirements

Engineering release tags SHALL:

- Use a Git-valid tag name
- Be annotated tags rather than lightweight tags
- Resolve to one immutable commit
- Include a human-readable annotation
- Be unique within the repository
- Be pushed and verified against the remote repository
- Never be silently moved or overwritten
- Remain traceable after supersession

Tag creation automation SHALL reject:

- Empty tag names
- Invalid Git reference syntax
- Empty annotations
- Duplicate local tags
- Duplicate remote tags
- A remote tag that resolves to an unexpected commit

Force-updating a release tag is forbidden.

A correction SHALL create a new tag and a supersession record.

## Naming Strategy

Tag names should be deterministic, readable, sortable, and scoped to their release class.

Recommended patterns:

```text
atx-eng-gate-YYYY-MM-DD-pass
atx-spec-NNN-vMAJOR.MINOR-status
atx-recovery-YYYY-MM-DD-description
vMAJOR.MINOR.PATCH
```

A tag name SHALL NOT contain explanatory prose that belongs in the annotation.

## Promotion Rule

A configuration state SHALL NOT be promoted to an Engineering Gate baseline unless the required Engineering Gate reports:

```text
FAIL=0
ENGINEERING GATE RESULT: PASS
```

A product release MAY impose stronger promotion requirements, including:

- Full software test-suite pass
- RTL lint pass
- Simulation pass
- Synthesis pass
- Hardware-in-the-loop pass
- Security review
- Architecture review
- Documentation graph closure
- Reproducibility confirmation

The required gate profile SHALL be recorded in the release manifest.

## Release Manifest

AEMS SHALL support a machine-readable release manifest.

The initial logical model is:

```yaml
release:
  id: atx-eng-gate-2026-06-17-pass
  class: engineering-gate-baseline
  repository: dlworrell/atarix
  commit: <full-commit-sha>
  source_ref: main
  status: verified
  gate_result: pass
  created_at: <UTC timestamp>
  created_by: github-actions[bot]
  specifications:
    - ATX-SPEC-020
    - ATX-SPEC-021
    - ATX-SPEC-090
    - ATX-SPEC-091
    - ATX-SPEC-092
  architecture:
    - ATX-100
  artifacts: []
```

The exact storage format may evolve, but the semantic fields SHALL remain traceable.

## Provenance Rule

Every promoted release SHALL preserve enough provenance to answer:

- What source state was released?
- Who or what selected it?
- Which gate profile was required?
- Which checks ran?
- Which checks passed, failed, or were waived?
- Which toolchain produced the evidence?
- Which artifacts were generated?
- Which specifications and requirements were covered?
- Has the release been superseded, withdrawn, or retained as current?

A release without sufficient provenance SHALL be treated as unverified.

## Artifact Integrity

Generated release artifacts SHOULD be accompanied by a manifest containing:

- Artifact name
- Artifact role
- Byte length
- Cryptographic digest
- Generating workflow and job
- Source commit
- Toolchain identifier
- Target platform or hardware revision

An artifact digest mismatch SHALL invalidate that artifact's release evidence.

Artifact integrity does not make an artifact authoritative; capability, policy, lifecycle, and deployment validation still apply.

## Reproducibility

Atarix SHOULD support reproduction of a release from the recorded source commit and declared toolchain.

A reproducibility check SHOULD distinguish:

- Bit-for-bit reproducible artifacts
- Semantically reproducible artifacts
- Non-reproducible artifacts with documented causes

Non-deterministic metadata, timestamps, paths, and toolchain salts SHOULD be identified and minimized.

## Supersession And Withdrawal

A release may be:

- Current
- Superseded
- Withdrawn
- Retained for recovery
- Archived for historical reference

Supersession SHALL NOT delete the earlier release record.

A supersession record SHALL identify:

- Prior release
- Replacement release
- Reason
- Decision authority
- Date
- Relevant issue, review, or incident

A withdrawn release SHALL remain discoverable unless legal or security requirements require restricted retention.

## Rollback And Recovery

Rollback SHALL select a previously verified configuration baseline rather than reconstructing an undocumented state.

Before rollback, the system SHALL evaluate:

- Data-format compatibility
- Schema migration state
- Capability and policy compatibility
- Hardware revision compatibility
- Recovery and reconciliation requirements
- Known security defects

Rollback does not erase later audit history.

## AEMS Responsibilities

AEMS SHALL eventually support:

- Release manifest generation
- Tag-to-commit verification
- Gate-to-release traceability
- Release history reporting
- Supersession relationships
- Artifact inventory and digest validation
- Missing provenance detection
- Release-class validation
- Configuration drift reporting

AEMS SHOULD be able to generate a human-readable release appendix for ATX-100.

## Workflow Requirements

The initial `Create Engineering Tag` workflow SHALL:

- Accept branch/ref, tag name, and annotation as separate inputs
- Validate all inputs before mutation
- Check out only the requested branch/ref
- Create an annotated tag
- Reject duplicate tags
- Push the tag explicitly
- Verify the remote tag resolves to the selected commit
- Produce a human-readable GitHub job summary

The workflow SHALL NOT accept a free-form Git command from the user.

## Audit Window

Tagging and release promotion SHALL provide a bounded audit record containing:

- Requested ref
- Resolved commit
- Tag name
- Annotation
- Actor
- Push result
- Verification result
- Workflow run identity

## Failure Semantics

Release automation SHALL fail closed when:

- Inputs are malformed
- The requested ref does not exist
- The tag already exists
- Tag creation fails
- Push authorization fails
- Remote verification fails
- The remote tag resolves to a different commit
- Required release evidence is missing

Partial success SHALL be reported explicitly.

## Initial Implementation Status

The initial implementation is:

```text
.github/workflows/create-engineering-tag.yml
```

It implements safe annotated-tag creation, duplicate rejection, remote verification, and a human-readable summary.

AEMS release-manifest persistence and automated gate-to-release promotion remain future work.

## Acceptance Criteria For v0.1

ATX-SPEC-092 v0.1 is ready for implementation review when:

1. The tag workflow passes a successful annotated-tag creation test.
2. Duplicate-tag creation is rejected.
3. Invalid tag syntax is rejected.
4. Remote tag resolution is verified against the selected commit.
5. The workflow emits a readable release report.
6. AEMS recognizes ATX-SPEC-092 in the document graph.
7. A tracking issue exists for release manifests and release-history persistence.
