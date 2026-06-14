#include "atarix/mailbox.h"

static uint32_t crc32_update(uint32_t crc, const unsigned char *data, size_t length) {
    size_t i;
    int bit;

    for (i = 0; i < length; i++) {
        crc ^= (uint32_t)data[i];
        for (bit = 0; bit < 8; bit++) {
            if ((crc & 1u) != 0u) {
                crc = (crc >> 1) ^ 0xEDB88320u;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

uint32_t atarix_mailbox_crc32(
    const void *data,
    size_t length) {
    uint32_t crc = 0xFFFFFFFFu;

    if (!data && length != 0u) {
        return 0u;
    }

    crc = crc32_update(crc, (const unsigned char *)data, length);
    return crc ^ 0xFFFFFFFFu;
}

int atarix_mailbox_message_type_known(uint16_t type) {
    switch (type) {
    case ATARIX_MAILBOX_MESSAGE_PING:
    case ATARIX_MAILBOX_MESSAGE_PONG:
    case ATARIX_MAILBOX_MESSAGE_VERSION_QUERY:
    case ATARIX_MAILBOX_MESSAGE_VERSION_REPLY:
    case ATARIX_MAILBOX_MESSAGE_STATUS_QUERY:
    case ATARIX_MAILBOX_MESSAGE_STATUS_REPLY:
    case ATARIX_MAILBOX_MESSAGE_READ_REGISTER:
    case ATARIX_MAILBOX_MESSAGE_WRITE_REGISTER:
    case ATARIX_MAILBOX_MESSAGE_RESET_REQUEST:
    case ATARIX_MAILBOX_MESSAGE_FAULT_REPORT:
    case ATARIX_MAILBOX_MESSAGE_DISCOVERY_RECORD_PUSH:
    case ATARIX_MAILBOX_MESSAGE_CAPABILITY_EVALUATE:
        return 1;
    default:
        return 0;
    }
}

atarix_mailbox_status_t atarix_mailbox_build_header(
    atarix_mailbox_header_v1_t *header,
    uint16_t type,
    uint32_t sequence,
    const void *payload,
    uint16_t payload_length) {
    if (!header || (!payload && payload_length != 0u)) {
        return ATARIX_MAILBOX_STATUS_NULL;
    }

    if (!atarix_mailbox_message_type_known(type)) {
        return ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE;
    }

    if (payload_length > ATARIX_MAILBOX_MAX_PAYLOAD_V1) {
        return ATARIX_MAILBOX_STATUS_INVALID_LENGTH;
    }

    header->magic = ATARIX_MAILBOX_MAGIC_V1;
    header->type = type;
    header->length = payload_length;
    header->sequence = sequence;
    header->crc32 = atarix_mailbox_crc32(payload, payload_length);

    return ATARIX_MAILBOX_STATUS_OK;
}

atarix_mailbox_status_t atarix_mailbox_validate_header(
    const atarix_mailbox_header_v1_t *header,
    const void *payload,
    size_t payload_length) {
    uint32_t crc;

    if (!header || (!payload && payload_length != 0u)) {
        return ATARIX_MAILBOX_STATUS_NULL;
    }

    if (header->magic != ATARIX_MAILBOX_MAGIC_V1) {
        return ATARIX_MAILBOX_STATUS_INVALID_HEADER;
    }

    if (!atarix_mailbox_message_type_known(header->type)) {
        return ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE;
    }

    if (header->length > ATARIX_MAILBOX_MAX_PAYLOAD_V1 || header->length != payload_length) {
        return ATARIX_MAILBOX_STATUS_INVALID_LENGTH;
    }

    crc = atarix_mailbox_crc32(payload, payload_length);
    if (crc != header->crc32) {
        return ATARIX_MAILBOX_STATUS_CRC_MISMATCH;
    }

    return ATARIX_MAILBOX_STATUS_OK;
}

atarix_mailbox_status_t atarix_mailbox_sequence_accept(
    atarix_mailbox_sequence_window_t *window,
    uint32_t sequence) {
    if (!window) {
        return ATARIX_MAILBOX_STATUS_NULL;
    }

    if (!window->initialized) {
        window->last_sequence = sequence;
        window->initialized = 1;
        return ATARIX_MAILBOX_STATUS_OK;
    }

    if (sequence == window->last_sequence) {
        return ATARIX_MAILBOX_STATUS_DUPLICATE;
    }

    if (sequence < window->last_sequence) {
        return ATARIX_MAILBOX_STATUS_OUT_OF_ORDER;
    }

    if (sequence != window->last_sequence + 1u) {
        return ATARIX_MAILBOX_STATUS_SEQUENCE_ERROR;
    }

    window->last_sequence = sequence;
    return ATARIX_MAILBOX_STATUS_OK;
}

atarix_mailbox_status_t atarix_mailbox_ring_authorize(
    atarix_ring_t source_ring,
    atarix_ring_t target_ring,
    uint16_t message_type) {
    atarix_ring_status_t ring_status;

    if (!atarix_mailbox_message_type_known(message_type)) {
        return ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE;
    }

    ring_status = atarix_ring_can_access(source_ring,
                                         target_ring,
                                         ATARIX_RING_OPERATION_REQUEST);
    if (ring_status != ATARIX_RING_STATUS_ALLOW) {
        return ATARIX_MAILBOX_STATUS_RING_DENIED;
    }

    return ATARIX_MAILBOX_STATUS_OK;
}
