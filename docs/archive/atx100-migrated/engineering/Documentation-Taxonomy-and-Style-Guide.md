---
document: ATX-ENG-DOC-001
title: Documentation Taxonomy and Style Guide
type: Engineering Guide
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
---

# Documentation Taxonomy and Style Guide

## Purpose

This guide defines the initial documentation taxonomy, naming conventions, and lifecycle rules for Atarix documentation.

## Document Classes

Atarix uses these document classes:

- ATX-100: Master architecture reference.
- ATX-DESIGN: Design philosophy and guiding principles.
- ATX-SPEC: Normative architecture and implementation requirements.
- ATX-REVIEW: Architecture review records and reconciliation findings.
- ATX-ENG: Engineering process, gate, tooling, and release documentation.
- Archive: Historical frozen documents retained for traceability.

## Lifecycle States

Documents may use these lifecycle states:

```text
Concept
Draft
Reviewed
Verified
Integrated
Archived
```

## Metadata

Every major document SHOULD include a YAML front matter block with:

- document
- title
- type
- status
- lifecycle
- version
- canonical_owner

## Generated Material

Generated material belongs under docs/generated or an AEMS output directory.

Generated material must identify its source command and input files.

## Archival Rule

Superseded documents are not deleted. They are archived and linked to their canonical successor.
