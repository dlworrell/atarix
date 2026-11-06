<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:2952d22f-c708-4c1a-aea4-68b99ddd4c4e
-->

# ATX-SPEC-020 Hardware Mailbox Integration Notes

## Status

Draft v0.1

## Purpose

This note records the hardware and mailbox integration decisions for ATX-SPEC-020 Hash Table and Lookup Acceleration Model.

It captures the W65C816 plus ECP5 Fabric Northbridge integration model, corrected message payload direction, RTL correction notes, and implementation TODOs.

## Core Architectural Decision

The lookup accelerator is a service behind the Atarix mailbox framework.

```text
Client
  -> Mailbox frame
  -> Ring authorization
  -> Capability context validation
  -> Lookup service
  -> Audit window commit
  -> Reply
```

The accelerator is not directly exposed as an ambient hardware primitive.

```text
Lookup acceleration is not authority.
```

## Mailbox-First Rule

All hardware lookup acceleration requests must enter through an Atarix mailbox message.

No lookup accelerator may execute a table probe, bitmap query, membership-filter query, Elias-Fano decode, or other accelerated search operation before mailbox authorization succeeds.

## Capability Context Rule

Capability authority should be derived from mailbox context and native capability state.

The caller should not be trusted merely because it supplies a capability-like scalar in the payload.

Recommended direction:

```text
Mailbox header/source/ring context
  -> native capability lookup
  -> policy check
  -> accelerator execution permission
```

Payloads may carry a capability reference or descriptor only if the receiving service validates it against native capability state.

## Registry Identifier Rule

Accelerator requests should not expose raw physical table addresses as caller-controlled authority.

Prefer:

```text
registry_id
table_id
index_id
view_id
```

instead of:

```text
table_base
raw physical pointer
fabric-local memory address
```

The service resolves the identifier to physical placement after authorization.

## CRC Rule

CRC validation is an integrity check, not an authority check.

CRC may detect bus corruption, framing errors, or malformed transfers.

CRC must not be used for authentication, policy approval, authority, or anti-tamper guarantees.

## Deterministic Probe Limit Rule

Every probe-capable lookup structure must define a bounded forward-progress limit.

For hardware acceleration, this limit may map to a fabric down-counter.

If the probe sequence exceeds the limit, the operation must terminate with an explicit sequence error rather than hanging the memory engine.

## Audit Window Rule

The accelerator must emit an audit-window record before reporting successful completion to the CPU when the operation is authority-bearing or policy-relevant.

The audit window should include:

```text
Message sequence
Message type
Source identity
Target service identity
Registry or table identifier
Capability context result
Policy result
Structure type
Generation
Probe count or query window
Result status
Returned object or row identifier where applicable
```

Human-readable text output may be used for early development, but the final format should be a structured audit event that can also project to text.

## Corrected Message Direction

Initial sketch used raw fields such as:

```text
table_base
cap_token
fingerprint
probe_limit
```

Corrected architecture should move toward declarative service requests:

```text
message_type: INDEX_QUERY
registry_id or table_id
query_type
key or key hash
expected generation
probe budget
projection flags
```

The accelerator service chooses the physical implementation.

Clients request results.

Services choose implementations.

## Proposed Message Types

Initial proposed registry additions:

```text
ATARIX_MAILBOX_MESSAGE_INDEX_QUERY
ATARIX_MAILBOX_MESSAGE_INDEX_MUTATE
ATARIX_MAILBOX_MESSAGE_INDEX_RECOVER
ATARIX_MAILBOX_MESSAGE_INDEX_STATUS
```

Legacy or lower-level probe-specific messages may exist inside the service boundary, but should not be the public compatibility interface unless explicitly approved by architecture review.

## Current Mailbox Payloads

The public ABI scaffold is maintained in [`include/atarix/index.h`](../../include/atarix/index.h).
Use that header rather than the earlier payload sketch in this note.

The v1 query and reply payloads are each 16 bytes. The reply includes a
reserved 32-bit field, and the header checks both payload sizes with static
assertions. The query carries `expected_generation`; carrying that field
does not establish that the RTL enforces it.

The payload names a registry or table. It does not grant authority. The
receiving service must validate native mailbox, capability, policy,
lifecycle, and recovery state before use.

## RTL Correction Notes

The repository contains an [accelerator implementation](../../rtl/atarix/atx_spec_020_accelerator.v),
a [CI testbench](../../rtl/atarix/atx_spec_020_accelerator_ci_tb.v), and a
[simulation workflow](../../.github/workflows/rtl-atx-spec-020.yml). These are
implementation and test artifacts, not evidence that every architectural
requirement is satisfied.

[Architecture Review 004, Finding 006](https://github.com/dlworrell/atarix/pull/125)
identifies an open defect: `req_expected_generation` is accepted but is not
compared with `table_generation`. The current RTL does not enforce a caller's expected generation. Existing
simulation coverage does not close this gap. Resolving the status semantics,
implementing the check, and adding mismatch tests are separate engineering work.

The earlier RTL correction checklist is retained below for traceability.
Each item needs implementation and test evidence before it can be marked
complete; this list is not a statement that every item remains absent:

```text
1. Replace any typo such as landmark_offset with resolved_lane_offset.
2. Do not expose raw table_base as public authority.
3. Treat caller-provided capability tokens as references requiring native validation, not authority.
4. CRC32 is integrity only, not security.
5. Confirm CRC polynomial, bit reflection, seed, and final XOR against the mailbox framing contract.
6. Ensure the priority encoder has deterministic default behavior.
7. Ensure Elias-Fano select/rank logic is implemented as a verified primitive, not an accidental behavioral loop.
8. Audit commit must occur before successful completion is signaled for authority-bearing operations.
9. Probe-limit exhaustion must return an explicit sequence error.
10. Filter positive results must always be confirmed by authoritative lookup.
```

## ECP5 Northbridge Pipeline Requirement

The hardware pipeline should enforce this order:

```text
Frame receive
  -> Magic/header validation
  -> CRC integrity validation
  -> mailbox ring authorization
  -> capability context validation
  -> policy validation
  -> optional membership precheck
  -> authoritative table/index lookup
  -> ordered/bitmap decode if required
  -> audit-window commit
  -> reply
```

No implementation may move membership probing ahead of authorization when that would leak presence information.

## Elias-Fano Hardware Direction

Elias-Fano acceleration should expose verified rank/select style primitives.

The implementation must produce a human-readable ordered projection for audit windows.

The architecture should not depend on an unbounded bit-by-bit walk.

## Roaring Bitmap Direction

Roaring bitmap routing should be developed as a separate implementation task.

Required properties:

```text
Container type visibility
Query plan auditability
Explicit result window projection
Policy-preserving intersection
Recovery-checkable container metadata
```

## Prime Catalog Direction

Hardware and software table sizing should use signed offline prime catalogs where appropriate.

The catalog is a sizing aid only.

It is not authority.

## Summary

The hardware integration is architecturally sound if implemented as a mailbox-authorized lookup service rather than as a raw probe primitive.

Required corrections are:

```text
Use registry/table identifiers instead of raw addresses.
Validate capability through native mailbox context.
Treat CRC as integrity only.
Make probe limits mandatory.
Commit audit windows before successful completion.
Keep filters non-authoritative.
Expose human-readable projections.
```
