#ifndef ATARIX_MAILBOX_H
#define ATARIX_MAILBOX_H

#include <stddef.h>
#include <stdint.h>

#include "atarix/ring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_MAILBOX_MAGIC_V1 0x58424D41u
#define ATARIX_MAILBOX_HEADER_SIZE_V1 16u
#define ATARIX_MAILBOX_MAX_PAYLOAD_V1 4096u

typedef enum atarix_mailbox_status {
    ATARIX_MAILBOX_STATUS_OK = 0,
    ATARIX_MAILBOX_STATUS_NULL = 1,
    ATARIX_MAILBOX_STATUS_INVALID_HEADER = 2,
    ATARIX_MAILBOX_STATUS_INVALID_LENGTH = 3,
    ATARIX_MAILBOX_STATUS_CRC_MISMATCH = 4,
    ATARIX_MAILBOX_STATUS_SEQUENCE_ERROR = 5,
    ATARIX_MAILBOX_STATUS_DUPLICATE = 6,
    ATARIX_MAILBOX_STATUS_OUT_OF_ORDER = 7,
    ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE = 8,
    ATARIX_MAILBOX_STATUS_RING_DENIED = 9
} atarix_mailbox_status_t;

typedef enum atarix_mailbox_message_type {
    ATARIX_MAILBOX_MESSAGE_INVALID = 0x0000,
    ATARIX_MAILBOX_MESSAGE_PING = 0x0001,
    ATARIX_MAILBOX_MESSAGE_PONG = 0x0002,
    ATARIX_MAILBOX_MESSAGE_VERSION_QUERY = 0x0003,
    ATARIX_MAILBOX_MESSAGE_VERSION_REPLY = 0x0004,
    ATARIX_MAILBOX_MESSAGE_STATUS_QUERY = 0x0005,
    ATARIX_MAILBOX_MESSAGE_STATUS_REPLY = 0x0006,
    ATARIX_MAILBOX_MESSAGE_READ_REGISTER = 0x0007,
    ATARIX_MAILBOX_MESSAGE_WRITE_REGISTER = 0x0008,
    ATARIX_MAILBOX_MESSAGE_RESET_REQUEST = 0x0009,
    ATARIX_MAILBOX_MESSAGE_FAULT_REPORT = 0x000A,
    ATARIX_MAILBOX_MESSAGE_DISCOVERY_RECORD_PUSH = 0x000B,
    ATARIX_MAILBOX_MESSAGE_CAPABILITY_EVALUATE = 0x000C
} atarix_mailbox_message_type_t;

#pragma pack(push, 1)
typedef struct atarix_mailbox_header_v1 {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
} atarix_mailbox_header_v1_t;
#pragma pack(pop)

_Static_assert(sizeof(atarix_mailbox_header_v1_t) == ATARIX_MAILBOX_HEADER_SIZE_V1,
               "ATARIX Mailbox Header v1 must be 16 bytes");

typedef struct atarix_mailbox_sequence_window {
    uint32_t last_sequence;
    int initialized;
} atarix_mailbox_sequence_window_t;

uint32_t atarix_mailbox_crc32(
    const void *data,
    size_t length);

atarix_mailbox_status_t atarix_mailbox_build_header(
    atarix_mailbox_header_v1_t *header,
    uint16_t type,
    uint32_t sequence,
    const void *payload,
    uint16_t payload_length);

atarix_mailbox_status_t atarix_mailbox_validate_header(
    const atarix_mailbox_header_v1_t *header,
    const void *payload,
    size_t payload_length);

atarix_mailbox_status_t atarix_mailbox_sequence_accept(
    atarix_mailbox_sequence_window_t *window,
    uint32_t sequence);

atarix_mailbox_status_t atarix_mailbox_ring_authorize(
    atarix_ring_t source_ring,
    atarix_ring_t target_ring,
    uint16_t message_type);

int atarix_mailbox_message_type_known(uint16_t type);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_MAILBOX_H */
