#ifndef ATARIX_DIRECTORY_H
#define ATARIX_DIRECTORY_H

#include <stdint.h>
#include "atarix/discovery.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_DIRECTORY_MAGIC_ATDR 0x52445441u
#define ATARIX_DIRECTORY_VERSION_MAJOR_V1 1u
#define ATARIX_DIRECTORY_VERSION_MINOR_V0 0u
#define ATARIX_DIRECTORY_HEADER_SIZE_V1 32u
#define ATARIX_DIRECTORY_ENTRY_SIZE_V1 56u

typedef enum atarix_directory_binding_type {
    ATARIX_DIRECTORY_BINDING_INVALID = 0x00000000u,
    ATARIX_DIRECTORY_BINDING_MAILBOX = 0x00000001u,
    ATARIX_DIRECTORY_BINDING_TRANSPORT_HANDLE = 0x00000002u,
    ATARIX_DIRECTORY_BINDING_SHARED_MEMORY = 0x00000003u,
    ATARIX_DIRECTORY_BINDING_SUPERVISOR_CALL = 0x00000004u,
    ATARIX_DIRECTORY_BINDING_FABRIC_SERVICE = 0x00000005u,
    ATARIX_DIRECTORY_BINDING_NETWORK_ENDPOINT = 0x00000006u,
    ATARIX_DIRECTORY_BINDING_STORAGE_ENDPOINT = 0x00000007u
} atarix_directory_binding_type_t;

typedef enum atarix_directory_entry_flags {
    ATARIX_DIRECTORY_FLAG_AVAILABLE = 1u << 0,
    ATARIX_DIRECTORY_FLAG_AUTHORIZED = 1u << 1,
    ATARIX_DIRECTORY_FLAG_RESTRICTED = 1u << 2,
    ATARIX_DIRECTORY_FLAG_RECOVERY_ONLY = 1u << 3,
    ATARIX_DIRECTORY_FLAG_DEVELOPMENT_ONLY = 1u << 4,
    ATARIX_DIRECTORY_FLAG_HOTPLUG_CAPABLE = 1u << 5,
    ATARIX_DIRECTORY_FLAG_INSTRUMENTED = 1u << 6
} atarix_directory_entry_flags_t;

#pragma pack(push, 1)
typedef struct atarix_directory_header_v1 {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t total_length;
    uint32_t entry_count;
    uint32_t flags;
    uint32_t header_crc32;
    uint32_t directory_crc32;
    uint32_t reserved;
} atarix_directory_header_v1_t;

typedef struct atarix_directory_entry_v1 {
    atarix_discovery_handle_t service_handle;
    atarix_discovery_handle_t provider_handle;
    uint32_t service_id;
    uint32_t provider_id;
    uint32_t flags;
    uint32_t binding_type;
    atarix_discovery_handle_t binding_handle;
    atarix_discovery_handle_t required_capability_handle;
    uint32_t minimum_ring;
    uint32_t reserved;
} atarix_directory_entry_v1_t;
#pragma pack(pop)

_Static_assert(sizeof(atarix_directory_header_v1_t) == ATARIX_DIRECTORY_HEADER_SIZE_V1,
               "ATARIX Directory header v1 must be 32 bytes");

_Static_assert(sizeof(atarix_directory_entry_v1_t) == ATARIX_DIRECTORY_ENTRY_SIZE_V1,
               "ATARIX Directory entry v1 must be 56 bytes");

static inline int atarix_directory_magic_is_valid(uint32_t magic) {
    return magic == ATARIX_DIRECTORY_MAGIC_ATDR;
}

static inline int atarix_directory_entry_is_available(uint32_t flags) {
    return (flags & ATARIX_DIRECTORY_FLAG_AVAILABLE) != 0;
}

static inline int atarix_directory_entry_is_authorized(uint32_t flags) {
    return (flags & ATARIX_DIRECTORY_FLAG_AUTHORIZED) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_DIRECTORY_H */
