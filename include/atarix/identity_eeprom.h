#ifndef ATARIX_IDENTITY_EEPROM_H
#define ATARIX_IDENTITY_EEPROM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_IDENTITY_MAGIC_ATIX 0x58495441u
#define ATARIX_IDENTITY_FORMAT_VERSION_V1 0x0001u
#define ATARIX_IDENTITY_BLOCK_SIZE_V1 512u

#define ATARIX_FACTORY_TEST_SIGNATURE_NONE 0x0000000000000000ull
#define ATARIX_FACTORY_TEST_SIGNATURE_PASS 0x4154495854455354ull

#define ATARIX_VENDOR_INVALID 0x00000000u
#define ATARIX_VENDOR_RESERVED 0x00000001u
#define ATARIX_VENDOR_HOMEBREW_BASE 0xFFF00000u
#define ATARIX_VENDOR_HOMEBREW_END  0xFFFFFFFFu

typedef enum atarix_board_class {
    ATARIX_BOARD_CLASS_INVALID = 0x0000,
    ATARIX_BOARD_CLASS_CPU_CARD = 0x0001,
    ATARIX_BOARD_CLASS_MEMORY_CARD = 0x0002,
    ATARIX_BOARD_CLASS_NETWORK_CARD = 0x0003,
    ATARIX_BOARD_CLASS_STORAGE_CARD = 0x0004,
    ATARIX_BOARD_CLASS_SUPERVISOR_CARD = 0x0005,
    ATARIX_BOARD_CLASS_BACKPLANE_CONTROLLER = 0x0006,
    ATARIX_BOARD_CLASS_DEVELOPMENT_CARD = 0x0007,
    ATARIX_BOARD_CLASS_HOMEBREW_CARD = 0x0008,
    ATARIX_BOARD_CLASS_PROTOTYPE_CARD = 0x0009,
    ATARIX_BOARD_CLASS_UNPROVISIONED_CARD = 0x000A,
    ATARIX_BOARD_CLASS_VENDOR_SPECIFIC = 0xFFFF
} atarix_board_class_t;

typedef enum atarix_trust_class {
    ATARIX_TRUST_CLASS_INVALID = 0x0000,
    ATARIX_TRUST_CLASS_UNKNOWN = 0x0001,
    ATARIX_TRUST_CLASS_HOMEBREW = 0x0002,
    ATARIX_TRUST_CLASS_DEVELOPMENT = 0x0003,
    ATARIX_TRUST_CLASS_FACTORY = 0x0004,
    ATARIX_TRUST_CLASS_PLATFORM_CRITICAL = 0x0005,
    ATARIX_TRUST_CLASS_VENDOR_SPECIFIC = 0xFFFF
} atarix_trust_class_t;

typedef enum atarix_identity_status_flags {
    ATARIX_ID_STATUS_PRESENT = 1u << 0,
    ATARIX_ID_STATUS_READABLE = 1u << 1,
    ATARIX_ID_STATUS_HEADER_VALID = 1u << 2,
    ATARIX_ID_STATUS_CRC_VALID = 1u << 3,
    ATARIX_ID_STATUS_IDENTITY_VALID = 1u << 4,
    ATARIX_ID_STATUS_POLICY_ACCEPTED = 1u << 5,
    ATARIX_ID_STATUS_VALID = 1u << 6,
    ATARIX_ID_STATUS_ERROR = 1u << 7
} atarix_identity_status_flags_t;

typedef enum atarix_identity_capability_flags {
    ATARIX_ID_CAP_MAILBOX_ENDPOINT = 1ull << 0,
    ATARIX_ID_CAP_DISCOVERY_ROM = 1ull << 1,
    ATARIX_ID_CAP_DMA_HARDWARE = 1ull << 2,
    ATARIX_ID_CAP_INTERRUPT_HARDWARE = 1ull << 3,
    ATARIX_ID_CAP_BOOT_CAPABLE = 1ull << 4,
    ATARIX_ID_CAP_RECOVERY_CAPABLE = 1ull << 5,
    ATARIX_ID_CAP_SUPERVISOR_FACING = 1ull << 6,
    ATARIX_ID_CAP_FABRIC_MEMORY = 1ull << 7,
    ATARIX_ID_CAP_NETWORK = 1ull << 8,
    ATARIX_ID_CAP_STORAGE = 1ull << 9,
    ATARIX_ID_CAP_INSTRUMENTATION = 1ull << 10,
    ATARIX_ID_CAP_DEVELOPMENT_EXPERIMENTAL = 1ull << 11
} atarix_identity_capability_flags_t;

#pragma pack(push, 1)
typedef struct atarix_identity_eeprom_v1 {
    uint32_t magic;
    uint16_t format_version;
    uint16_t header_length;
    uint32_t total_record_length;
    uint32_t vendor_id;
    uint32_t device_id;
    uint64_t serial_number;
    uint16_t board_revision;
    uint16_t board_class;
    uint16_t trust_class;
    uint16_t reserved_alignment;
    uint64_t manufacture_timestamp;
    uint32_t boot_firmware_revision;
    uint32_t provisioning_tool_revision;
    uint64_t capability_flags;
    char board_name[32];
    uint32_t provisioning_revision;
    uint64_t provisioning_timestamp;
    uint64_t factory_test_signature;
    uint32_t crc32;
    uint8_t reserved[396];
} atarix_identity_eeprom_v1_t;
#pragma pack(pop)

_Static_assert(sizeof(atarix_identity_eeprom_v1_t) == ATARIX_IDENTITY_BLOCK_SIZE_V1,
               "ATARIX Identity EEPROM v1 block must be 512 bytes");

static inline int atarix_identity_vendor_is_homebrew(uint32_t vendor_id) {
    return vendor_id >= ATARIX_VENDOR_HOMEBREW_BASE;
}

static inline int atarix_identity_magic_is_valid(uint32_t magic) {
    return magic == ATARIX_IDENTITY_MAGIC_ATIX;
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_IDENTITY_EEPROM_H */
