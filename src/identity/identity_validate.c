/* SPDX-FileCopyrightText: Donovan Worrell */
/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Identity EEPROM v1 structural validation (docs/identity-eeprom-v1.md,
 * "Supervisor Responsibilities" steps 3-6, without the policy decision).
 */
#include "atarix/identity_eeprom.h"
#include "atarix/mailbox.h"

uint32_t atarix_identity_crc32_v1(const atarix_identity_eeprom_v1_t *identity) {
    atarix_identity_eeprom_v1_t copy;

    if (!identity) {
        return 0;
    }
    /* "CRC field is treated as zero during calculation", over 0x000-0x1FF,
     * CRC-32/ISO-HDLC (the same algorithm as the mailbox CRC). */
    copy = *identity;
    copy.crc32 = 0;
    return atarix_mailbox_crc32(&copy, sizeof(copy));
}

uint32_t atarix_identity_validate_v1(const atarix_identity_eeprom_v1_t *identity) {
    uint32_t status = ATARIX_ID_STATUS_PRESENT | ATARIX_ID_STATUS_READABLE;

    if (!identity) {
        return ATARIX_ID_STATUS_ERROR;
    }

    if (!atarix_identity_magic_is_valid(identity->magic) ||
        identity->format_version != ATARIX_IDENTITY_FORMAT_VERSION_V1 ||
        identity->total_record_length < ATARIX_IDENTITY_BLOCK_SIZE_V1 ||
        identity->header_length == 0 ||
        identity->header_length > identity->total_record_length) {
        return status | ATARIX_ID_STATUS_ERROR;
    }
    status |= ATARIX_ID_STATUS_HEADER_VALID;

    if (atarix_identity_crc32_v1(identity) != identity->crc32) {
        return status | ATARIX_ID_STATUS_ERROR;
    }
    status |= ATARIX_ID_STATUS_CRC_VALID;

    if (identity->vendor_id == ATARIX_VENDOR_INVALID ||
        identity->vendor_id == ATARIX_VENDOR_RESERVED ||
        identity->device_id == 0 ||
        identity->board_class == ATARIX_BOARD_CLASS_INVALID ||
        identity->trust_class == ATARIX_TRUST_CLASS_INVALID) {
        return status | ATARIX_ID_STATUS_ERROR;
    }
    status |= ATARIX_ID_STATUS_IDENTITY_VALID;

    /* POLICY_ACCEPTED and VALID are the supervisor's decision, not this
     * function's: identity is not authority. */
    return status;
}
