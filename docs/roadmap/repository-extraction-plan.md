# Repository Extraction Plan

ATARIX, AEMS, JAG, and Just a Geek should be maintained as separate repositories with explicit dependency direction.

## Target repositories

```text
.github.com/dlworrell/aems
.github.com/dlworrell/atarix
.github.com/dlworrell/jag
.github.com/dlworrell/just-a-geek
```

## Ownership boundaries

### aems

AEMS is the engineering standard and reference tooling.

AEMS owns:

- AEMS specifications and terminology
- artifact model
- engineering graph model
- documentation governance
- requirements and traceability model
- validation and evidence model
- reusable CLI/reference implementation
- schemas and conformance tests

AEMS must not depend on ATARIX.

### atarix

ATARIX is the modular workstation architecture and implementation project.

ATARIX owns:

- ATX architecture specifications
- hardware, firmware, operating-system, and toolchain design
- ATX-100 canonical architecture book
- project-specific requirements and verification evidence
- AEMS configuration for the ATARIX project

ATARIX may depend on AEMS.

### jag

JAG is the future AI-assisted engineering assistant and tool layer.

JAG owns:

- AI engineering assistant experiments
- knowledge graph query tools
- architecture review assistants
- repository-analysis tools
- AEMS-aware workflow agents

JAG may depend on AEMS.

### just-a-geek

Just a Geek is the public/company/brand repository.

Just a Geek owns:

- website content
- company history
- public pages for AEMS, ATARIX, and JAG
- portfolio/archive material
- publication index

Just a Geek may publish outputs from AEMS, ATARIX, and JAG, but should not own their source material.

## Extraction order

1. Stabilize the current ATARIX documentation migration.
2. Create empty `aems`, `jag`, and `just-a-geek` repositories.
3. Extract AEMS specifications and reusable tools from ATARIX into `aems`.
4. Leave ATARIX project-specific configuration in `atarix`.
5. Add `aems` as a submodule, vendored tool, or CI dependency only after AEMS has a stable public interface.
6. Create `jag` as a placeholder repo with a product charter, not active code, until the AEMS graph model is stable.
7. Create `just-a-geek` as a minimal static-site repository later.

## Dependency rule

```text
AEMS  -> standalone
ATARIX -> may consume AEMS
JAG    -> may consume AEMS and ATARIX metadata
Just a Geek -> may publish outputs from all repos
```

No repository should create circular build dependencies.

## Immediate next actions

- Open tracking issues in `atarix` for AEMS, JAG, and Just a Geek extraction.
- Finish the current migration PR before moving files out.
- Create empty destination repositories manually in GitHub.
- Populate destination repositories only after ATARIX CI is green.
