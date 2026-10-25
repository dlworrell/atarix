# ADR: SQLite as the AEMS Engineering Evidence Index

Status: Proposed
Date: 2026-10-18
Owner: ATARIX
Review: Catylist governance issue #34; AES engineering-criteria review requested
Related: ATX-SPEC-090, ATX-SPEC-092, ATARIX issue #56

## Context
AEMS currently assembles documentation and specification relationships, Git history, GitHub Actions verification results, and release-evidence JSON artifacts. The June 17–October 18, 2026 historical recovery effort requires efficient indexed queries, source traceability, repeatable ingestion and explicit handling of missing CI evidence. JSON-only storage is portable but weak for multi-entity queries and transactional consistency.

## Proposed decision
Use a **single-file SQLite relational database** as AEMS's **rebuildable local operational index**. It is not a new source of governance or release authority. Git commits, project-owned specifications and ADRs, signed or integrity-checked JSON release manifests, and retained CI artifacts remain authoritative within their existing trust boundaries.

The index SHALL NOT be represented as one flat denormalized table. Proposed entities include repositories, commits and parents, specifications, requirements, traceability edges, workflow runs, verification results, artifacts, releases, release-evidence relationships and audit events. Identifiers and source provenance SHALL be retained across imports.

Use Python's built-in sqlite3; enforce foreign keys, unique source keys, transactions, schema-versioned migrations, indexed lookups and UTC timestamps. WAL may be used for local concurrent reads with a documented checkpoint/backup strategy. Do not commit the live binary database to Git; retain reconstructable source records and portable, versioned JSON exports.

Release promotion SHALL be fail-closed. The gate, exact commit, required evidence and artifact digests must be validated independently of cached index state. A passed gate does not itself authorize a release tag. Historical verification SHALL distinguish contemporaneous evidence, retrospective evidence and unverified state, without rewriting historical commits or pretending modern tools existed in earlier snapshots.

## Alternatives
1. JSON-only records and Python scripts: portable and reviewable, but complex for relational reporting, consistency and indexed historical queries.
2. SQLite relational index: preferred, minimal operations and strong local transactions, but binary file is unsuitable for Git diffs and requires disciplined migration/rebuild procedures.
3. PostgreSQL: stronger distributed multiwriter operation, but unjustified server and deployment overhead at current scale.
4. Git-only metadata: immutable history, but not an efficient query engine for relationships and verification status.

## Authority and standards
- **ATARIX** owns this project-local storage architecture, schema, implementation, tests and operational recovery.
- **Catylist** reviews cross-repository governance implications, authority boundaries and shared contract impact; this proposal does not request transfer of local design authority.
- **AES** owns applicable engineering requirements and evidence obligations. Review against AES-DEV-001, especially architecture/specification before implementation, versioning, review discipline, observability, trust boundaries, and recovery. Confirm applicability of AES-SEC-001 and AES-SEC-002 to actual native and secret-handling surfaces; do not assert applicability without analysis.
- **AEMS** implements discovery, indexing, reporting and progressive enforcement, without defining new standards.

Relevant: Catylist decision-process.md, Catylist ADR-0006, AES-DEV-001 R001–R007, Catylist governance issue #34.

## Security and trust boundaries
Treat imported repository metadata, CI outputs, JSON and artifact paths as untrusted. Use parameterized SQL, bounded input validation, path confinement and symlink checks, explicit file permissions, least-privilege workflow credentials, cryptographic artifact hashes, and independent manifest validation. A database record is not proof of CI execution or review approval. Protect evidence provenance against substitution and tampering.

## Recovery and migration
Version the database schema; test upgrades, failures, and compatibility. Support deterministic full reconstruction from retained authoritative sources, idempotent imports, interrupted transaction recovery, integrity checks, WAL-safe backups and restores, and reconciliation that flags missing or contradictory evidence. Do not silently downgrade verification or remove historical audit records.

## Acceptance criteria before acceptance or rollout
- Reviewed schema with key, index, foreign-key and migration definitions.
- Representative benchmark for historical commits, requirements and CI records.
- Repeatable rebuild from Git, manifests and retained evidence, with consistent query results.
- Tests for duplicate/out-of-order imports, missing/failed CI, wrong commit SHA, malformed manifests, mismatched digests, path traversal, interruption, corruption and backup/restore.
- Proven distinction between contemporaneous, retrospective and missing evidence.
- Gate qualification independent of database cache, refusing incomplete evidence.
- Review of AES applicability, security, recovery, retention and CI permissions.
- Documentation and tests included in the same reviewable PR series; branch protection and signed-commit requirements honored where applicable.

## Consequences
Benefits: efficient relational traceability, transactional updates, portable deployment, repeatable history analysis and clear reporting. Costs: schema maintenance, migration tests, source synchronization and backup/recovery procedures. The index is disposable; evidence retention and governance authority are not.

## Decision and implementation sequence
1. Submit this ADR as **Proposed** for ATARIX review and link Catylist #34.
2. Obtain AES criteria/applicability assessment and resolve governance concerns.
3. Record acceptance/rejection with rationale; update ATX-SPEC-090 and ATX-SPEC-092 in the same change series.
4. Only after acceptance, implement SQLite schema/migrations and importers in small PRs, beginning with non-blocking reports.
5. Verify pipeline behavior and then introduce release enforcement, retaining explicit waivers and migration baselines.

No approval or deployment is implied by this proposal.
