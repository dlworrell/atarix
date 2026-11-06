/* SPDX-FileCopyrightText: Donovan Worrell */
/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>

#include "atarix/identity_eeprom.h"

static void make_valid(atarix_identity_eeprom_v1_t *id) {
    static const atarix_identity_eeprom_v1_t zero;
    *id = zero;
    id->magic = ATARIX_IDENTITY_MAGIC_ATIX;
    id->format_version = ATARIX_IDENTITY_FORMAT_VERSION_V1;
    id->header_length = 0x80;
    id->total_record_length = ATARIX_IDENTITY_BLOCK_SIZE_V1;
    id->vendor_id = 0x0000A7A1u;
    id->device_id = 0x0816u;
    id->board_class = ATARIX_BOARD_CLASS_CPU_CARD;
    id->trust_class = ATARIX_TRUST_CLASS_FACTORY;
    id->crc32 = atarix_identity_crc32_v1(id);
}

static int expect(const char *what, uint32_t got, int want_valid) {
    const int valid = (got & ATARIX_ID_STATUS_IDENTITY_VALID) && !(got & ATARIX_ID_STATUS_ERROR);
    if (valid != want_valid) {
        printf("%s: status 0x%02x, expected %s\n", what, (unsigned)got, want_valid ? "valid" : "invalid");
        return 1;
    }
    if (got & (ATARIX_ID_STATUS_POLICY_ACCEPTED | ATARIX_ID_STATUS_VALID)) {
        printf("%s: structural validation must not grant policy acceptance\n", what);
        return 1;
    }
    return 0;
}

int main(void) {
    atarix_identity_eeprom_v1_t id;
    int failures = 0;

    make_valid(&id);
    failures += expect("valid record", atarix_identity_validate_v1(&id), 1);

    make_valid(&id);
    id.board_name[3] ^= 0x01; /* one bit, CRC not updated */
    failures += expect("corrupted body", atarix_identity_validate_v1(&id), 0);
    if (atarix_identity_validate_v1(&id) & ATARIX_ID_STATUS_CRC_VALID) {
        printf("corrupted body: CRC reported valid\n");
        failures++;
    }

    make_valid(&id);
    id.magic ^= 1u;
    id.crc32 = atarix_identity_crc32_v1(&id);
    failures += expect("bad magic", atarix_identity_validate_v1(&id), 0);

    make_valid(&id);
    id.vendor_id = ATARIX_VENDOR_INVALID;
    id.crc32 = atarix_identity_crc32_v1(&id);
    failures += expect("invalid vendor", atarix_identity_validate_v1(&id), 0);

    make_valid(&id);
    id.vendor_id = ATARIX_VENDOR_HOMEBREW_BASE + 7u;
    id.crc32 = atarix_identity_crc32_v1(&id);
    failures += expect("homebrew vendor is structurally valid", atarix_identity_validate_v1(&id), 1);

    failures += expect("null", atarix_identity_validate_v1(NULL), 0);
    return failures ? 1 : 0;
}
