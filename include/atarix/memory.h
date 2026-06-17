#ifndef ATARIX_MEMORY_H
#define ATARIX_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_MEMORY_DESCRIPTOR_MAGIC_V1 0x584D454Du
#define ATARIX_MEMORY_DESCRIPTOR_VERSION_V1 1u
#define ATARIX_MEMORY_DESCRIPTOR_MAX_LENGTH_V1 0x00100000u

/*
 * ATX-SPEC-021: Data movement is not authority.
 * A valid descriptor describes a requested movement. It does not authorize it
 * unless an explicit capability and policy path validates the request.
 */
typedef enum atarix_memory_status {
    ATARIX_MEMORY_STATUS_OK = 0,
    ATARIX_MEMORY_STATUS_NULL_DESCRIPTOR = 1,
    ATARIX_MEMORY_STATUS_BAD_MAGIC = 2,
    ATARIX_MEMORY_STATUS_BAD_VERSION = 3,
    ATARIX_MEMORY_STATUS_ZERO_LENGTH = 4,
    ATARIX_MEMORY_STATUS_MISSING_CAPABILITY = 5,
    ATARIX_MEMORY_STATUS_MISSING_SOURCE = 6,
    ATARIX_MEMORY_STATUS_MISSING_DESTINATION = 7,
    ATARIX_MEMORY_STATUS_BAD_FLAGS = 8,
    ATARIX_MEMORY_STATUS_LENGTH_EXCEEDED = 9
} atarix_memory_status_t;

typedef enum atarix_memory_move_flags {
    ATARIX_MEMORY_MOVE_COPY = 0x00000001u,
    ATARIX_MEMORY_MOVE_MOVE = 0x00000002u,
    ATARIX_MEMORY_MOVE_MAP = 0x00000004u,
    ATARIX_MEMORY_MOVE_SHARE = 0x00000008u,
    ATARIX_MEMORY_MOVE_FABRIC = 0x00000010u,
    ATARIX_MEMORY_MOVE_ZERO_COPY = 0x00000020u
} atarix_memory_move_flags_t;

#define ATARIX_MEMORY_MOVE_FORM_MASK \
    (ATARIX_MEMORY_MOVE_COPY | \
     ATARIX_MEMORY_MOVE_MOVE | \
     ATARIX_MEMORY_MOVE_MAP | \
     ATARIX_MEMORY_MOVE_SHARE | \
     ATARIX_MEMORY_MOVE_FABRIC | \
     ATARIX_MEMORY_MOVE_ZERO_COPY)

#pragma pack(push, 1)
typedef struct atarix_memory_descriptor_v1 {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t flags;
    uint32_t source_object;
    uint32_t source_offset;
    uint32_t destination_object;
    uint32_t destination_offset;
    uint32_t length;
    uint32_t capability_id;
    uint32_t generation;
    uint32_t audit_hint;
} atarix_memory_descriptor_v1_t;
#pragma pack(pop)

_Static_assert(sizeof(atarix_memory_descriptor_v1_t) == 44u,
               "ATARIX Memory Descriptor v1 must be 44 bytes");

atarix_memory_status_t atarix_memory_descriptor_validate(
    const atarix_memory_descriptor_v1_t *descriptor);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_MEMORY_H */
