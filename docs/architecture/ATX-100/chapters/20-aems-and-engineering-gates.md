---
document: ATX-100-CH20
title: AEMS and Engineering Gates
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-090
  - ATX-SPEC-091
implements:
  - tools/aems/aems.py
  - scripts/engineering/run_gate.sh
  - scripts/engineering/check_aems_docs.sh
verified_by:
  - engineering-gate.yml
---

# Chapter 20: AEMS and Engineering Gates

## Engineering As Architecture

Atarix treats engineering evidence as part of the architecture.

A design is not considered stable merely because it is written down or implemented. It must be linked to requirements, reviewed, tested, and verified by the Engineering Gate.

## AEMS

AEMS is the Atarix Engineering Management System.

AEMS is responsible for making repository state discoverable and auditable. Its initial responsibilities include:

- Document discovery.
- Metadata parsing.
- Documentation graph generation.
- Traceability edge reporting.
- Specification index reporting.
- Engineering Gate support.

## Documentation Graph

The documentation graph links documents through relationships such as:

- canonical ownership.
- dependencies.
- related specifications.
- implementation artifacts.
- verification artifacts.
- supersession and archival history.

## Engineering Gate

The Engineering Gate is the repository-level consistency check.

It validates architecture invariants, public headers, RTL lint, RTL simulations, and documentation graph health.

The gate is expected to grow over time into a full engineering certification system covering source code, RTL, documentation, traceability, release evidence, and performance history.

## Release Evidence

A future Atarix release should include generated AEMS evidence:

- document graph.
- traceability matrix.
- Engineering Gate summary.
- architecture manual edition.
- specification index.
- release history.
