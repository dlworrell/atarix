# Atarix Engineering Management System

## Status

Draft v0.1

## Purpose

The Atarix Engineering Management System, or AEMS, is a repo-native engineering assistant for keeping specifications, reviews, issues, implementation work, and engineering gates synchronized.

AEMS is intentionally small at first. It should be deterministic, auditable, scriptable, and safe to run in CI.

## Initial Commands

```sh
python3 tools/aems/aems.py list-specs
python3 tools/aems/aems.py check-specs
python3 tools/aems/aems.py next
python3 tools/aems/aems.py report
```

## Initial Scope

AEMS v0.1 tracks:

- Specification IDs
- Specification titles
- Specification file paths
- Status values
- Dependency lists
- Related issue numbers
- Engineering notes

## Non-Goals For v0.1

AEMS v0.1 does not mutate GitHub Projects, close issues, create labels, or edit milestones.

Those features should be added only after the local registry and validation flow are stable.

## Design Rules

- Markdown remains human-readable.
- JSON registry remains machine-readable.
- CI output must be deterministic.
- Missing files are failures.
- Missing issue links are warnings until issue sync exists.
- Specification dependency drift should be detected early.

## Planned Growth

Future versions should add:

- GitHub issue synchronization
- GitHub Projects v2 synchronization
- Architecture dependency graph export
- Review status tracking
- Engineering Gate integration
- Release/tag report generation
- Stale issue detection
- Duplicate issue detection
- Specification reconciliation checks
