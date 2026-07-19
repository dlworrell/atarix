#ifndef ATARIX_BIB_H
#define ATARIX_BIB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_BIB_VERSION_MAJOR_V1 1u
#define ATARIX_BIB_VERSION_MINOR_V0 0u

#define ATARIX_BIB_HEADER_SIZE_V1 64u
#define ATARIX_BIB_TLV_HEADER_SIZE_V1 16u
#define ATARIX_BIB_REFERENCE_FIXED_SIZE_V1 40u
#define ATARIX_BIB_IDENTITY_FIXED_SIZE_V1 16u
#define ATARIX_BIB_ALIGNMENT_V1 8u
#define ATARIX_BIB_RECOMMENDED_MAX_SIZE_V1 (64u * 1024u)

#define ATARIX_BIB_MAGIC_BYTE_0 ((uint8_t)'A')
#define ATARIX_BIB_MAGIC_BYTE_1 ((uint8_t)'T')
#define ATARIX_BIB_MAGIC_BYTE_2 ((uint8_t)'A')
#define ATARIX_BIB_MAGIC_BYTE_3 ((uint8_t)'R')
#define ATARIX_BIB_MAGIC_BYTE_4 ((uint8_t)'X')
#define ATARIX_BIB_MAGIC_BYTE_5 ((uint8_t)'B')
#define ATARIX_BIB_MAGIC_BYTE_6 ((uint8_t)'I')
#define ATARIX_BIB_MAGIC_BYTE_7 ((uint8_t)'B')

#define ATARIX_BIB_HEADER_KNOWN_FLAGS_V1 ((uint16_t)0x000Fu)
#define ATARIX_BIB_TLV_KNOWN_FLAGS_V1 ((uint16_t)0x0003u)

typedef uint64_t atarix_bib_object_id_t;
typedef uint64_t atarix_bib_address_t;

typedef enum atarix_bib_header_flags_v1 {
    ATARIX_BIB_FLAG_RECOVERY_BOOT = 1u << 0,
    ATARIX_BIB_FLAG_DEGRADED_BOOT = 1u << 1,
    ATARIX_BIB_FLAG_SECURE_POLICY_ACTIVE = 1u << 2,
    ATARIX_BIB_FLAG_DIAGNOSTIC_BOOT = 1u << 3
} atarix_bib_header_flags_v1_t;

typedef enum atarix_bib_tlv_flags_v1 {
    ATARIX_BIB_TLV_FLAG_MANDATORY = 1u << 0,
    ATARIX_BIB_TLV_FLAG_SENSITIVE = 1u << 1
} atarix_bib_tlv_flags_v1_t;

typedef enum atarix_bib_tlv_type_v1 {
    ATARIX_BIB_TLV_RESERVED = 0x0000,
    ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE = 0x0001,
    ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE = 0x0002,
    ATARIX_BIB_TLV_BOOT_DEVICE_IDENTITY = 0x0003,
    ATARIX_BIB_TLV_BOOT_IMAGE_IDENTITY = 0x0004,
    ATARIX_BIB_TLV_BOOT_ARGUMENTS = 0x0005,
    ATARIX_BIB_TLV_CONSOLE_SERVICE_IDENTITY = 0x0006,
    ATARIX_BIB_TLV_SUPERVISOR_SERVICE_IDENTITY = 0x0007,
    ATARIX_BIB_TLV_AUDIT_LOG_REFERENCE = 0x0008,
    ATARIX_BIB_TLV_FIRMWARE_BUILD_IDENTITY = 0x0009,
    ATARIX_BIB_TLV_ENTROPY_SEED_REFERENCE = 0x000A,
    ATARIX_BIB_TLV_EXPERIMENTAL_BEGIN = 0x8000,
    ATARIX_BIB_TLV_EXPERIMENTAL_END = 0xFFFF
} atarix_bib_tlv_type_v1_t;

typedef enum atarix_bib_error {
    ATARIX_BIB_OK = 0,
    ATARIX_BIB_E_WINDOW,
    ATARIX_BIB_E_MAGIC,
    ATARIX_BIB_E_VERSION,
    ATARIX_BIB_E_HEADER_LENGTH,
    ATARIX_BIB_E_TOTAL_LENGTH,
    ATARIX_BIB_E_CRC,
    ATARIX_BIB_E_HEADER_RESERVED,
    ATARIX_BIB_E_TLV_BOUNDS,
    ATARIX_BIB_E_TLV_RESERVED,
    ATARIX_BIB_E_UNKNOWN_MANDATORY,
    ATARIX_BIB_E_REQUIRED_RECORD,
    ATARIX_BIB_E_REFERENCE_OVERFLOW,
    ATARIX_BIB_E_REFERENCE_AUTHORITY,
    ATARIX_BIB_E_REFERENCE_INTEGRITY
} atarix_bib_error_t;

#pragma pack(push, 1)

typedef struct atarix_bib_header_v1 {
    uint8_t magic[8];
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t header_length;
    uint16_t flags;
    uint32_t total_length;
    uint32_t crc32c;
    uint64_t boot_id;
    uint64_t producer_id;
    uint64_t created_counter;
    uint32_t tlv_offset;
    uint32_t tlv_count;
    uint64_t reserved;
} atarix_bib_header_v1_t;

typedef struct atarix_bib_tlv_header_v1 {
    uint16_t type;
    uint16_t flags;
    uint32_t length;
    uint32_t payload_length;
    uint32_t reserved;
} atarix_bib_tlv_header_v1_t;

typedef struct atarix_bib_reference_fixed_v1 {
    atarix_bib_object_id_t object_id;
    atarix_bib_address_t address;
    uint64_t length;
    uint32_t format_major;
    uint32_t format_minor;
    uint32_t integrity_kind;
    uint32_t integrity_length;
} atarix_bib_reference_fixed_v1_t;

typedef struct atarix_bib_identity_fixed_v1 {
    atarix_bib_object_id_t object_id;
    uint32_t identity_kind;
    uint32_t identity_length;
} atarix_bib_identity_fixed_v1_t;

#pragma pack(pop)

_Static_assert(sizeof(atarix_bib_header_v1_t) == ATARIX_BIB_HEADER_SIZE_V1,
               "ATARIX BIB header v1 must be 64 bytes");
_Static_assert(offsetof(atarix_bib_header_v1_t, magic) == 0,
               "ATARIX BIB magic offset must be 0");
_Static_assert(offsetof(atarix_bib_header_v1_t, version_major) == 8,
               "ATARIX BIB major-version offset must be 8");
_Static_assert(offsetof(atarix_bib_header_v1_t, version_minor) == 10,
               "ATARIX BIB minor-version offset must be 10");
_Static_assert(offsetof(atarix_bib_header_v1_t, header_length) == 12,
               "ATARIX BIB header-length offset must be 12");
_Static_assert(offsetof(atarix_bib_header_v1_t, flags) == 14,
               "ATARIX BIB flags offset must be 14");
_Static_assert(offsetof(atarix_bib_header_v1_t, total_length) == 16,
               "ATARIX BIB total-length offset must be 16");
_Static_assert(offsetof(atarix_bib_header_v1_t, crc32c) == 20,
               "ATARIX BIB CRC offset must be 20");
_Static_assert(offsetof(atarix_bib_header_v1_t, boot_id) == 24,
               "ATARIX BIB boot-id offset must be 24");
_Static_assert(offsetof(atarix_bib_header_v1_t, producer_id) == 32,
               "ATARIX BIB producer-id offset must be 32");
_Static_assert(offsetof(atarix_bib_header_v1_t, created_counter) == 40,
               "ATARIX BIB created-counter offset must be 40");
_Static_assert(offsetof(atarix_bib_header_v1_t, tlv_offset) == 48,
               "ATARIX BIB TLV-offset offset must be 48");
_Static_assert(offsetof(atarix_bib_header_v1_t, tlv_count) == 52,
               "ATARIX BIB TLV-count offset must be 52");
_Static_assert(offsetof(atarix_bib_header_v1_t, reserved) == 56,
               "ATARIX BIB reserved offset must be 56");

_Static_assert(sizeof(atarix_bib_tlv_header_v1_t) == ATARIX_BIB_TLV_HEADER_SIZE_V1,
               "ATARIX BIB TLV header v1 must be 16 bytes");
_Static_assert(offsetof(atarix_bib_tlv_header_v1_t, type) == 0,
               "ATARIX BIB TLV type offset must be 0");
_Static_assert(offsetof(atarix_bib_tlv_header_v1_t, flags) == 2,
               "ATARIX BIB TLV flags offset must be 2");
_Static_assert(offsetof(atarix_bib_tlv_header_v1_t, length) == 4,
               "ATARIX BIB TLV length offset must be 4");
_Static_assert(offsetof(atarix_bib_tlv_header_v1_t, payload_length) == 8,
               "ATARIX BIB TLV payload-length offset must be 8");
_Static_assert(offsetof(atarix_bib_tlv_header_v1_t, reserved) == 12,
               "ATARIX BIB TLV reserved offset must be 12");

_Static_assert(sizeof(atarix_bib_reference_fixed_v1_t) == ATARIX_BIB_REFERENCE_FIXED_SIZE_V1,
               "ATARIX BIB reference fixed payload v1 must be 40 bytes");
_Static_assert(sizeof(atarix_bib_identity_fixed_v1_t) == ATARIX_BIB_IDENTITY_FIXED_SIZE_V1,
               "ATARIX BIB identity fixed payload v1 must be 16 bytes");

static inline int atarix_bib_magic_is_valid(const uint8_t magic[8]) {
    return magic != NULL &&
           magic[0] == ATARIX_BIB_MAGIC_BYTE_0 &&
           magic[1] == ATARIX_BIB_MAGIC_BYTE_1 &&
           magic[2] == ATARIX_BIB_MAGIC_BYTE_2 &&
           magic[3] == ATARIX_BIB_MAGIC_BYTE_3 &&
           magic[4] == ATARIX_BIB_MAGIC_BYTE_4 &&
           magic[5] == ATARIX_BIB_MAGIC_BYTE_5 &&
           magic[6] == ATARIX_BIB_MAGIC_BYTE_6 &&
           magic[7] == ATARIX_BIB_MAGIC_BYTE_7;
}

static inline int atarix_bib_length_is_aligned(uint32_t length) {
    return (length & (ATARIX_BIB_ALIGNMENT_V1 - 1u)) == 0u;
}

static inline int atarix_bib_header_flags_are_known(uint16_t flags) {
    return (flags & (uint16_t)~ATARIX_BIB_HEADER_KNOWN_FLAGS_V1) == 0u;
}

static inline int atarix_bib_tlv_flags_are_known(uint16_t flags) {
    return (flags & (uint16_t)~ATARIX_BIB_TLV_KNOWN_FLAGS_V1) == 0u;
}

static inline int atarix_bib_tlv_type_is_experimental(uint16_t type) {
    return type >= ATARIX_BIB_TLV_EXPERIMENTAL_BEGIN;
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_BIB_H */
