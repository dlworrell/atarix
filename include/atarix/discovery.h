#ifndef ATARIX_DISCOVERY_H
#define ATARIX_DISCOVERY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_DISCOVERY_MAGIC_ATDX 0x58445441u
#define ATARIX_DISCOVERY_VERSION_MAJOR_V1 1u
#define ATARIX_DISCOVERY_VERSION_MINOR_V0 0u

#define ATARIX_DISCOVERY_HEADER_SIZE_V1 32u
#define ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1 8u
#define ATARIX_DISCOVERY_RECORD_ALIGNMENT_V1 8u

typedef uint64_t atarix_discovery_handle_t;

typedef enum atarix_discovery_header_flags {
    ATARIX_DISCOVERY_FLAG_BOOT_REQUIRED = 1u << 0,
    ATARIX_DISCOVERY_FLAG_RUNTIME_PROVIDER = 1u << 1,
    ATARIX_DISCOVERY_FLAG_INSTRUMENTED = 1u << 2,
    ATARIX_DISCOVERY_FLAG_CPU_RESOURCE_PROVIDER = 1u << 3,
    ATARIX_DISCOVERY_FLAG_HOTPLUG_CAPABLE = 1u << 4,
    ATARIX_DISCOVERY_FLAG_EXPERIMENTAL = 1u << 5
} atarix_discovery_header_flags_t;

typedef enum atarix_discovery_record_type {
    ATARIX_DISCOVERY_RECORD_RESERVED = 0x0000,
    ATARIX_DISCOVERY_RECORD_RESOURCE = 0x0001,
    ATARIX_DISCOVERY_RECORD_SERVICE = 0x0002,
    ATARIX_DISCOVERY_RECORD_PROVIDER = 0x0003,
    ATARIX_DISCOVERY_RECORD_DEPENDENCY = 0x0004,
    ATARIX_DISCOVERY_RECORD_INSTRUMENTATION = 0x0005,
    ATARIX_DISCOVERY_RECORD_VENDOR_BEGIN = 0x8000,
    ATARIX_DISCOVERY_RECORD_VENDOR_END = 0xFFFE,
    ATARIX_DISCOVERY_RECORD_END = 0xFFFF
} atarix_discovery_record_type_t;

typedef enum atarix_discovery_dependency_type {
    ATARIX_DISCOVERY_DEPENDENCY_UNSPECIFIED = 0x0000,
    ATARIX_DISCOVERY_DEPENDENCY_REQUIRES = 0x0001,
    ATARIX_DISCOVERY_DEPENDENCY_OPTIONAL = 0x0002,
    ATARIX_DISCOVERY_DEPENDENCY_CONFLICTS = 0x0003
} atarix_discovery_dependency_type_t;

#pragma pack(push, 1)

typedef struct atarix_discovery_header_v1 {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t total_length;
    uint32_t record_count;
    uint32_t provider_id;
    uint32_t flags;
    uint32_t header_crc32;
    uint32_t image_crc32;
} atarix_discovery_header_v1_t;

typedef struct atarix_discovery_record_header_v1 {
    uint16_t type;
    uint16_t version;
    uint32_t length;
} atarix_discovery_record_header_v1_t;

typedef struct atarix_discovery_resource_record_v1 {
    atarix_discovery_record_header_v1_t header;
    uint16_t resource_type_id;
    uint16_t resource_version;
    uint32_t resource_flags;
    atarix_discovery_handle_t resource_handle;
    uint32_t provider_id;
    uint32_t instance_id;
} atarix_discovery_resource_record_v1_t;

typedef struct atarix_discovery_service_record_v1 {
    atarix_discovery_record_header_v1_t header;
    uint16_t service_id;
    uint16_t service_version;
    uint32_t service_flags;
    atarix_discovery_handle_t service_handle;
    uint32_t provider_id;
    uint32_t instance_id;
} atarix_discovery_service_record_v1_t;

typedef struct atarix_discovery_provider_record_fixed_v1 {
    atarix_discovery_record_header_v1_t header;
    uint32_t provider_id;
    uint32_t provider_version;
    uint32_t provider_flags;
    uint32_t name_length;
} atarix_discovery_provider_record_fixed_v1_t;

typedef struct atarix_discovery_dependency_record_v1 {
    atarix_discovery_record_header_v1_t header;
    atarix_discovery_handle_t consumer_handle;
    atarix_discovery_handle_t provider_handle;
    uint16_t dependency_type;
    uint16_t dependency_version;
    uint32_t dependency_flags;
} atarix_discovery_dependency_record_v1_t;

typedef struct atarix_discovery_instrumentation_record_v1 {
    atarix_discovery_record_header_v1_t header;
    uint16_t instrumentation_type;
    uint16_t instrumentation_version;
    uint32_t instrumentation_flags;
    atarix_discovery_handle_t instrumentation_handle;
    atarix_discovery_handle_t observed_handle;
} atarix_discovery_instrumentation_record_v1_t;

#pragma pack(pop)

_Static_assert(sizeof(atarix_discovery_header_v1_t) == ATARIX_DISCOVERY_HEADER_SIZE_V1,
               "ATARIX Discovery header v1 must be 32 bytes");

_Static_assert(sizeof(atarix_discovery_record_header_v1_t) == ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1,
               "ATARIX Discovery record header v1 must be 8 bytes");

_Static_assert(sizeof(atarix_discovery_resource_record_v1_t) == 32,
               "ATARIX Discovery resource record v1 must be 32 bytes");

_Static_assert(sizeof(atarix_discovery_service_record_v1_t) == 32,
               "ATARIX Discovery service record v1 must be 32 bytes");

_Static_assert(sizeof(atarix_discovery_dependency_record_v1_t) == 32,
               "ATARIX Discovery dependency record v1 must be 32 bytes");

_Static_assert(sizeof(atarix_discovery_instrumentation_record_v1_t) == 32,
               "ATARIX Discovery instrumentation record v1 must be 32 bytes");

static inline uint16_t atarix_discovery_handle_provider16(atarix_discovery_handle_t handle) {
    return (uint16_t)((handle >> 48) & 0xFFFFu);
}

static inline uint16_t atarix_discovery_handle_type(atarix_discovery_handle_t handle) {
    return (uint16_t)((handle >> 32) & 0xFFFFu);
}

static inline uint32_t atarix_discovery_handle_instance(atarix_discovery_handle_t handle) {
    return (uint32_t)(handle & 0xFFFFFFFFu);
}

static inline atarix_discovery_handle_t atarix_discovery_make_handle(uint16_t provider_id_low16,
                                                                      uint16_t type_id,
                                                                      uint32_t instance_id) {
    return ((uint64_t)provider_id_low16 << 48) |
           ((uint64_t)type_id << 32) |
           (uint64_t)instance_id;
}

static inline int atarix_discovery_magic_is_valid(uint32_t magic) {
    return magic == ATARIX_DISCOVERY_MAGIC_ATDX;
}

static inline int atarix_discovery_record_is_vendor_specific(uint16_t type) {
    return type >= ATARIX_DISCOVERY_RECORD_VENDOR_BEGIN &&
           type <= ATARIX_DISCOVERY_RECORD_VENDOR_END;
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_DISCOVERY_H */
