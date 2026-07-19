# ATARIX Boot Information Block v1

Status: Draft specification  
Closure unit: `ATX-BIB-001`  
Authority: Normative when approved  
Producer: boot firmware or architecturally equivalent emulator  
Consumer: kernel entry code

## 1. Purpose

The Boot Information Block (BIB) is the versioned, self-describing firmware-to-kernel handoff object for ATARIX.

The BIB communicates the minimum information required for a kernel to discover the boot environment without depending on CPU-card-local addresses, firmware-private structures, or undocumented fabric state.

The BIB is an architectural interface. CPU width, compiler structure packing, native pointer width, and physical fabric topology shall not alter its serialized representation.

## 2. Closure claim

This specification defines the intent representation for closure unit `ATX-BIB-001`.

The closure unit remains `OPEN` until the public header, producer, consumer, diagnostic decoder, conformance vectors, malformed-input tests, round-trip tests, and first-boot evidence agree with this specification.

## 3. Design requirements

The BIB shall:

1. have an unambiguous format identity;
2. carry explicit major and minor versions;
3. declare its complete serialized length;
4. use fixed-width little-endian integers;
5. contain no native pointers;
6. validate before any referenced object is dereferenced;
7. permit compatible extension without changing the fixed header;
8. reject unknown incompatible semantics;
9. identify referenced objects by fabric-visible address and bounded length;
10. include an integrity check over the complete object;
11. leave reserved fields zero for deterministic encoding;
12. preserve enough provenance to diagnose the handoff that produced it.

## 4. Serialized organization

A BIB consists of a fixed 64-byte header followed by zero or more aligned Type-Length-Value records.

```text
+-------------------------------+
| Fixed header (64 bytes)       |
+-------------------------------+
| TLV record                    |
+-------------------------------+
| TLV record                    |
+-------------------------------+
| Padding, if required          |
+-------------------------------+
```

All multi-byte integer fields are encoded little-endian.

The complete BIB shall be aligned to 8 bytes. Each TLV record shall begin at an 8-byte boundary. Padding bytes shall be zero.

## 5. Fixed header

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 8 | `magic` | ASCII `ATARXBIB` |
| 8 | 2 | `version_major` | incompatible format version |
| 10 | 2 | `version_minor` | backward-compatible revision |
| 12 | 2 | `header_length` | fixed header length; v1 value is 64 |
| 14 | 2 | `flags` | header-level behavior flags |
| 16 | 4 | `total_length` | complete BIB length in bytes |
| 20 | 4 | `crc32c` | CRC-32C over the complete BIB with this field zeroed |
| 24 | 8 | `boot_id` | boot-attempt identifier |
| 32 | 8 | `producer_id` | opaque identity of the producing firmware object |
| 40 | 8 | `created_counter` | producer monotonic or boot-local sequence counter |
| 48 | 4 | `tlv_offset` | offset to the first TLV; v1 value is 64 |
| 52 | 4 | `tlv_count` | number of TLV records |
| 56 | 8 | `reserved` | shall be zero |

### 5.1 Magic

The exact eight bytes shall be:

```text
41 54 41 52 58 42 49 42
 A  T  A  R  X  B  I  B
```

### 5.2 Versions

A consumer supporting major version 1 shall reject any other major version.

A consumer may accept a greater minor version when all mandatory records are understood and all unknown records are marked optional.

### 5.3 Lengths

`header_length` shall be at least 64 and no greater than `total_length`.

`total_length` shall:

- be at least `header_length`;
- be a multiple of 8;
- not exceed the implementation-defined maximum accepted BIB size;
- fit completely inside the trusted handoff memory window supplied to the consumer.

The initial recommended maximum accepted size is 64 KiB.

### 5.4 Integrity

CRC-32C uses the Castagnoli polynomial.

The producer shall set `crc32c` to zero, calculate the CRC over exactly `total_length` bytes, and then write the calculated value into the field.

The consumer shall verify the CRC before processing any TLV payload or following any referenced object location.

CRC provides corruption detection, not authenticity or authority.

### 5.5 Boot identity and provenance

`boot_id` identifies one boot attempt and should not be reused intentionally.

`producer_id` is an opaque architectural object identity. It is not a raw address, slot number, or CPU-local pointer.

`created_counter` provides ordering evidence. It need not represent wall-clock time.

## 6. Header flags

| Bit | Name | Meaning |
|---:|---|---|
| 0 | `RECOVERY_BOOT` | firmware entered a recovery path |
| 1 | `DEGRADED_BOOT` | one or more expected services were unavailable |
| 2 | `SECURE_POLICY_ACTIVE` | firmware reports that the configured secure-boot policy was applied |
| 3 | `DIAGNOSTIC_BOOT` | diagnostic verbosity or instrumentation was requested |
| 4-15 | Reserved | shall be zero |

A flag is evidence supplied by the producer. It does not grant authority to the consumer.

## 7. TLV record header

Every TLV begins with this 16-byte header:

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 2 | `type` | record type |
| 2 | 2 | `flags` | record behavior flags |
| 4 | 4 | `length` | entire record length, including header and padding |
| 8 | 4 | `payload_length` | meaningful payload bytes |
| 12 | 4 | `reserved` | shall be zero |

`length` shall be at least 16, shall be a multiple of 8, and shall not exceed the remaining BIB bytes.

`payload_length` shall not exceed `length - 16`.

### 7.1 TLV flags

| Bit | Name | Meaning |
|---:|---|---|
| 0 | `MANDATORY` | consumer shall reject the BIB when this type is unknown |
| 1 | `SENSITIVE` | diagnostic renderers shall avoid exposing payload contents by default |
| 2-15 | Reserved | shall be zero |

An unknown TLV without `MANDATORY` set shall be skipped using its validated `length`.

## 8. Standard TLV types

| Type | Name | Requirement |
|---:|---|---|
| `0x0001` | Service Directory Reference | Mandatory |
| `0x0002` | Memory Map Reference | Mandatory |
| `0x0003` | Boot Device Identity | Optional |
| `0x0004` | Boot Image Identity | Optional |
| `0x0005` | Boot Arguments | Optional |
| `0x0006` | Console Service Identity | Optional |
| `0x0007` | Supervisor Service Identity | Optional |
| `0x0008` | Audit Log Reference | Optional |
| `0x0009` | Firmware Build Identity | Optional |
| `0x000A` | Entropy Seed Reference | Optional and sensitive |

Types `0x8000` through `0xFFFF` are reserved for experimental project-local records and shall not be emitted by release firmware without an approved specification.

## 9. Reference payload

Service-directory, memory-map, audit-log, and entropy-seed references use the common reference payload:

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 8 | `object_id` | opaque identity of the referenced object |
| 8 | 8 | `address` | fabric-visible byte address |
| 16 | 8 | `length` | bounded byte length |
| 24 | 4 | `format_major` | referenced-object major version |
| 28 | 4 | `format_minor` | referenced-object minor version |
| 32 | 4 | `integrity_kind` | integrity algorithm identifier |
| 36 | 4 | `integrity_length` | integrity data bytes following this fixed portion |
| 40 | variable | integrity data | digest or checksum bytes |

The consumer shall validate `address + length` without unsigned overflow before mapping or reading the referenced object.

A zero address or zero length is invalid for a mandatory reference.

The existence of a reference does not itself authorize access. The consumer shall obtain or already possess the capability required to map the referenced object.

## 10. Identity payload

Boot-device, boot-image, console-service, supervisor-service, and firmware-build records begin with:

| Offset | Size | Field | Meaning |
|---:|---:|---|---|
| 0 | 8 | `object_id` | opaque architectural identity |
| 8 | 4 | `identity_kind` | type-specific identity namespace |
| 12 | 4 | `identity_length` | following identity bytes |
| 16 | variable | identity | bytes defined by `identity_kind` |

Identity records shall not expose CPU-local pointers or mutable table indices as durable identity.

## 11. Boot arguments

The Boot Arguments payload contains UTF-8 bytes without a required terminating NUL.

The payload may be empty. Embedded NUL bytes are forbidden.

Boot arguments are untrusted input and shall not be interpreted as authority, capabilities, or permission to weaken policy.

## 12. Producer requirements

A producer shall:

1. initialize the complete allocation to zero;
2. emit records in ascending type order unless repeated type semantics specify otherwise;
3. emit exactly one Service Directory Reference;
4. emit exactly one Memory Map Reference;
5. ensure every referenced object is complete before publishing the BIB;
6. write the CRC after all other bytes are final;
7. publish the BIB location and bounded handoff window atomically with transfer of control where practical;
8. retain enough diagnostic evidence to reproduce or decode the published BIB.

The producer shall not modify the BIB after transferring control to the kernel.

## 13. Consumer validation order

A consumer shall validate in this order:

1. bounded handoff window is large enough for the fixed header;
2. magic;
3. supported major version;
4. header length;
5. total length and alignment;
6. CRC-32C;
7. header reserved fields and unknown header flags;
8. TLV region bounds;
9. each TLV header and record bounds;
10. mandatory-type support;
11. required-record cardinality;
12. payload-specific bounds and reserved fields;
13. referenced-object arithmetic and mapping authority;
14. referenced-object format and integrity.

A validation failure shall fail closed. The consumer shall not partially trust records parsed before the failure.

## 14. Failure reporting

When a diagnostic channel is available, rejection should report:

- closure unit `ATX-BIB-001`;
- validation stage;
- byte offset when known;
- stable error identifier;
- observed version and length values where safe;
- boot identifier when the fixed header was readable.

Sensitive payloads shall not be included by default.

## 15. Required stable error identifiers

| Identifier | Meaning |
|---|---|
| `BIB_E_WINDOW` | bounded handoff window invalid |
| `BIB_E_MAGIC` | magic mismatch |
| `BIB_E_VERSION` | unsupported major version |
| `BIB_E_HEADER_LENGTH` | invalid fixed-header length |
| `BIB_E_TOTAL_LENGTH` | invalid complete length or alignment |
| `BIB_E_CRC` | integrity check failed |
| `BIB_E_HEADER_RESERVED` | reserved header bits or bytes nonzero |
| `BIB_E_TLV_BOUNDS` | TLV exceeds BIB bounds |
| `BIB_E_TLV_RESERVED` | reserved TLV bits or bytes nonzero |
| `BIB_E_UNKNOWN_MANDATORY` | unsupported mandatory record |
| `BIB_E_REQUIRED_RECORD` | required record absent or duplicated |
| `BIB_E_REFERENCE_OVERFLOW` | referenced range arithmetic overflow |
| `BIB_E_REFERENCE_AUTHORITY` | referenced object cannot be mapped with available authority |
| `BIB_E_REFERENCE_INTEGRITY` | referenced-object integrity validation failed |

## 16. Compatibility

Major version changes may alter fixed-header or record semantics incompatibly.

Minor version changes may:

- define previously reserved optional TLV types;
- add optional flags from the reserved range;
- clarify validation behavior without changing accepted serialized meaning.

A minor version shall not change the meaning, width, offset, or byte order of an existing field.

## 17. Recovery and forensic requirements

Implementations should preserve:

- the exact serialized BIB from significant boot attempts;
- a decoded, human-readable rendering;
- producer build identity;
- consumer build identity;
- validation result;
- first failing byte offset or validation stage;
- hashes of referenced service-directory and memory-map objects where practical.

A recovered BIB shall be classified according to the governing recovery model. A reconstructed or inferred handoff object shall never be represented as the exact original serialized BIB.

## 18. Evidence obligations

Conformance evidence shall include:

- fixed-header size and field-offset assertions;
- deterministic valid golden vectors;
- cross-language or independent-decoder comparison where practical;
- invalid magic tests;
- unsupported-major-version tests;
- truncated-header and truncated-record tests;
- total-length and alignment tests;
- CRC failure tests;
- unknown optional and unknown mandatory record tests;
- duplicate and missing required-record tests;
- integer-overflow tests for every offset-plus-length calculation;
- producer/consumer round-trip tests;
- emulator and hardware capture comparison when hardware exists.

Randomized tests shall record their seed.

## 19. Explicitly deferred questions

The following are not silently decided by this draft:

- the final integrity algorithm identifiers for referenced objects;
- whether release firmware requires cryptographic authentication in addition to CRC;
- the maximum BIB size for constrained CPU cards;
- the mechanism used to pass the BIB address and bounded window to each CPU architecture;
- whether repeated records are permitted for any future standard type.

These require review before the closure unit can become `CLOSED`.
