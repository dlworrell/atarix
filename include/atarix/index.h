#ifndef ATARIX_INDEX_H
#define ATARIX_INDEX_H

/*
 * Atarix index acceleration ABI scaffold.
 *
 * This header defines mailbox payloads for ATX-SPEC-020 lookup acceleration.
 * It deliberately uses registry/table identifiers rather than raw physical
 * addresses. Capability-like values in this ABI are references only and must
 * be validated through native mailbox, capability, policy, lifecycle, and
 * recovery state before use.
 */

#include <stdint.h>

#include "atarix/mailbox.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_INDEX_ABI_VERSION_1 1u
#define ATARIX_INDEX_QUERY_PAYLOAD_SIZE_V1 16u
#define ATARIX_INDEX_REPLY_PAYLOAD_SIZE_V1 16u

typedef uint32_t atarix_index_registry_id_t;
typedef uint32_t atarix_index_key_hash_t;
typedef uint32_t atarix_index_generation_t;
typedef uint32_t atarix_index_resolved_id_t;
typedef uint16_t atarix_index_audit_window_id_t;

typedef enum atarix_index_query_type {
    ATARIX_INDEX_QUERY_EXACT = 0x00u,
    ATARIX_INDEX_QUERY_RANGE = 0x01u,
    ATARIX_INDEX_QUERY_FILTER = 0x02u,
    ATARIX_INDEX_QUERY_BITMAP = 0x03u,
    ATARIX_INDEX_QUERY_ORDERED = 0x04u
} atarix_index_query_type_t;

typedef enum atarix_index_projection_flags {
    ATARIX_INDEX_PROJECTION_NONE = 0x00u,
    ATARIX_INDEX_PROJECTION_AUDIT_WINDOW = 0x01u,
    ATARIX_INDEX_PROJECTION_HUMAN_READABLE = 0x02u,
    ATARIX_INDEX_PROJECTION_DIAGNOSTIC = 0x04u
} atarix_index_projection_flags_t;

#pragma pack(push, 1)

/*
 * Public index query payload.
 *
 * This is a declarative request. The caller names a registry/table and query
 * type; the receiving service chooses the implementation. The payload does
 * not contain raw table_base addresses and does not confer authority.
 */
typedef struct atarix_payload_index_query_v1 {
    uint32_t registry_id;
    uint32_t key_hash;
    uint32_t expected_generation;
    uint8_t query_type;
    uint8_t probe_limit;
    uint8_t projection_flags;
    uint8_t reserved;
} atarix_payload_index_query_v1_t;

/*
 * Public index reply payload.
 *
 * resolved_id is meaningful only when status is OK and the authority/policy
 * decision path has confirmed the lookup. audit_window_id may be zero when no
 * audit window was requested or required.
 */
typedef struct atarix_payload_index_reply_v1 {
    uint32_t resolved_id;
    uint32_t resolved_generation;
    uint8_t status;
    uint8_t match_offset;
    uint16_t audit_window_id;
    uint32_t reserved;
} atarix_payload_index_reply_v1_t;

#pragma pack(pop)

_Static_assert(sizeof(atarix_payload_index_query_v1_t) == ATARIX_INDEX_QUERY_PAYLOAD_SIZE_V1,
               "ATARIX index query payload v1 must be 16 bytes");

_Static_assert(sizeof(atarix_payload_index_reply_v1_t) == ATARIX_INDEX_REPLY_PAYLOAD_SIZE_V1,
               "ATARIX index reply payload v1 must be 16 bytes");

/*
 * Helper predicates for ABI users.
 */
static inline int atarix_index_query_type_known(uint8_t query_type)
{
    switch ((atarix_index_query_type_t)query_type) {
    case ATARIX_INDEX_QUERY_EXACT:
    case ATARIX_INDEX_QUERY_RANGE:
    case ATARIX_INDEX_QUERY_FILTER:
    case ATARIX_INDEX_QUERY_BITMAP:
    case ATARIX_INDEX_QUERY_ORDERED:
        return 1;
    default:
        return 0;
    }
}

static inline int atarix_index_projection_flags_valid(uint8_t flags)
{
    const uint8_t known =
        (uint8_t)(ATARIX_INDEX_PROJECTION_AUDIT_WINDOW |
                  ATARIX_INDEX_PROJECTION_HUMAN_READABLE |
                  ATARIX_INDEX_PROJECTION_DIAGNOSTIC);
    return (flags & (uint8_t)~known) == 0u;
}

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_INDEX_H */
