# ATARIX Closure-Based Development Cycle

Status: Draft
Owner: ATARIX
Authority: Project process
Governing model: Catylist CAT-002, when adopted

## 1. Purpose

This document applies the Catalyst engineering representation, closure, and recovery model to day-to-day ATARIX development.

ATARIX development shall advance small, reviewable closure units from intent through realization to evidence. A subsystem is not complete merely because a specification exists, code compiles, an FPGA bitstream is produced, or one hardware demonstration succeeds.

## 2. Representation model

Every consequential ATARIX capability shall identify three representation classes:

```text
                         Intent
              architecture and specification
                    /                   \
                   /                     \
          Realization ---------------- Evidence
     headers, firmware, HDL,       tests, simulation,
       emulator and hardware       measurements and traces
```

The transformations between these representations are part of the engineering record.

## 3. Closure unit

The default unit of substantive development is a closure unit.

A closure unit shall be small enough to review coherently. Examples include:

- one mailbox operation;
- one discovery record type;
- one capability rule;
- one DMA descriptor behavior;
- one supervisor command;
- one boot-stage transition;
- one memory-object lifecycle rule.

A closure unit shall identify:

```yaml
closure_unit:
  id: ATX-MBX-PING-001
  claim: A valid mailbox PING returns PONG and preserves the transaction identifier.
  intent:
    authority: normative
    sources:
      - docs/mailbox-protocol-v1.md
  realizations:
    public_header: required
    firmware: required
    emulator: required
    rtl: required
    physical_hardware: deferred
  evidence:
    encoding_vectors: required
    emulator_tests: required
    rtl_simulation: required
    differential_test: required
    hardware_capture: deferred
  security_impact: capability check and audit generation
  compatibility_impact: none
  recovery_impact: transaction identifier preserved in trace records
  state: OPEN
```

`deferred` is not equivalent to `not_applicable`. Deferred work remains an open gap unless the declared claim explicitly excludes that realization or evidence level.

## 4. Development sequence

### 4.1 Select a bounded capability

Work shall begin by selecting one bounded architectural behavior rather than a broad subsystem label.

Preferred:

> Implement and verify mailbox PING request/reply semantics.

Avoid:

> Implement the mailbox system.

### 4.2 Establish intent

Before implementation, identify the authoritative specification, architectural doctrine, externally visible behavior, security constraints, error behavior, and compatibility expectations.

Intent should describe stable architectural semantics. W65C816, ECP5, memory-address, register-allocation, and tool-specific choices shall remain implementation details unless explicitly elevated by architecture.

### 4.3 Derive evidence obligations

Each normative requirement shall identify evidence capable of supporting or falsifying it.

Example:

```text
MBX-REQ-001  A valid PING returns PONG.
MBX-REQ-002  The response preserves the transaction identifier.
MBX-ERR-001  An invalid length is rejected.
MBX-SEC-001  A request without required authority is rejected.
MBX-AUD-001  A rejected unauthorized request generates an audit event.
MBX-CON-001  Emulator and RTL produce equivalent observable behavior.
```

Evidence obligations should be written before or with the realization, not retrofitted after behavior becomes difficult to change.

### 4.4 Implement applicable realizations

A closure unit may require changes to:

- public headers;
- firmware;
- emulator models;
- FPGA RTL;
- kernel consumers;
- supervisor firmware;
- diagnostic and manufacturing tools;
- physical hardware.

The closure record shall state which realizations are required, deferred, or not applicable.

### 4.5 Produce evidence

ATARIX uses the following evidence ladder:

- **E0 — Structural:** formatting, links, schemas, compilation, linting;
- **E1 — Component:** unit tests, local assertions, static checks;
- **E2 — Reference:** emulator or known-good model comparison;
- **E3 — RTL/realization:** HDL simulation, synthesis checks, firmware integration;
- **E4 — Differential:** common vectors run against independent realizations;
- **E5 — Physical:** FPGA or board execution, captures, electrical measurements;
- **E6 — System:** end-to-end boot, recovery, security, and workload behavior.

The required level is claim-dependent. A physical claim cannot be closed solely with emulator evidence.

### 4.6 Review evidence against intent

Review shall ask:

- Does each normative requirement have supporting evidence?
- Did the tests encode temporary implementation details?
- Did implementation expose an omitted or incorrect requirement?
- Did a CPU-card or FPGA-specific constraint leak into public architecture?
- Does the change preserve ATARIX security, lifecycle, cleanup, and audit doctrine?
- Are error, recovery, and compatibility paths covered?

The result shall be one of:

- **OPEN** — required intent, realization, evidence, or traceability is missing;
- **VERIFYING** — realizations exist and required evidence is in progress;
- **DIVERGENT** — representations disagree and require an engineering decision;
- **CLOSED** — the declared claim is supported within its stated scope;
- **INDETERMINATE** — surviving information cannot support a defensible result.

## 5. Pull-request requirements

Every substantive pull request shall include a closure declaration identifying:

- closure-unit identifiers;
- authoritative intent;
- realizations changed;
- evidence added or updated;
- security, compatibility, persistence, and recovery impact;
- unresolved gaps;
- resulting closure state.

A realization change shall normally add or update evidence. When evidence is unaffected, the pull request shall explain why.

A documentation-only change may remain a valid closure increment, but it shall not claim realization or operational closure that it does not establish.

## 6. Commit discipline

ATARIX retains one logical change per commit and one authoritative document per documentation commit.

A normal closure sequence may be:

```text
1. Specify mailbox PING semantics.
2. Add mailbox PING evidence obligations and vectors.
3. Implement mailbox PING in the emulator.
4. Implement mailbox PING in firmware.
5. Implement mailbox PING in RTL.
6. Add emulator/RTL differential verification.
7. Record physical validation evidence.
8. Close the mailbox PING capability.
```

Each commit should represent one identifiable transformation between intent, realization, and evidence.

## 7. CI adoption

CI enforcement shall be introduced incrementally after closure metadata stabilizes.

Initial gates should verify:

1. closure identifiers use a valid format;
2. referenced intent and evidence paths exist;
3. required pull-request sections are present;
4. public-interface changes identify authoritative specifications;
5. realization changes identify evidence changes or a reviewed explanation;
6. a CLOSED declaration contains no unresolved required gaps.

Later gates may calculate representation coverage, validate machine-readable closure records, and produce subsystem closure reports.

## 8. First-boot application

The first-boot milestone shall be treated as a set of closure units rather than one undifferentiated demonstration:

```text
Power On
  -> Supervisor initialization
  -> Fabric initialization
  -> CPU ROM entry
  -> Identity EEPROM read
  -> Discovery ROM parsing
  -> Service Directory construction
  -> Boot Information Block creation
  -> Kernel Stub handoff
```

Each transition shall have:

- an authoritative contract;
- an applicable realization in supervisor, fabric, ROM, emulator, or kernel;
- ordered trace evidence;
- success and fault-path evidence;
- build and version provenance.

The appearance of a monitor prompt is necessary evidence, but it is not sufficient by itself to close the first-boot architecture.

## 9. Recovery-by-design

ATARIX formats and services should preserve the edge information required for later diagnosis and recovery.

Where applicable, designs should include:

- stable opaque object identities;
- format versions and bounded lengths;
- generation counters;
- transaction identifiers;
- checksums or hashes;
- commit markers and journals;
- ECC, parity, replication, or erasure coding;
- mapping provenance;
- supervisor fault and audit records;
- compatible readers and migration behavior.

Recovery reports shall distinguish content recovery, object identity, logical placement, chronology, and integrity verification rather than reducing them to one percentage.

## 10. Probabilistic and future quantum accelerators

A future probabilistic or quantum ATARIX service shall use claim-relative closure.

A result object shall preserve, as applicable:

- requested computation;
- compiler and lowering chain;
- hardware and calibration identity;
- raw observations or measurement records;
- correction, decoding, or mitigation method;
- statistical confidence and error bounds;
- verification resource cost;
- assumptions and excluded claims.

Quantum recovery shall target logical information and bounded computational claims, not an impossible complete reconstruction of an unknown physical quantum state.

A quantum-service response shall therefore return a provenance-bearing result object rather than an unqualified scalar answer.

## 11. Initial adoption plan

ATARIX shall adopt this process in four stages:

1. use the pull-request closure declaration for new substantive work;
2. select one first-boot capability as the initial closure-unit pilot;
3. add a machine-readable closure record after the pilot identifies stable fields;
4. introduce CI validation only after the record format has been exercised in real work.

The recommended pilot is the Boot Information Block because it connects specification, a public header, ROM and emulator producers, a kernel consumer, binary test vectors, malformed-input tests, and first-boot evidence.