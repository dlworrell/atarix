#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "atarix/discovery_parser.h"
#include "discovery_test_utils.h"

int main(void) {
    uint8_t image[ATARIX_TEST_IMAGE_SIZE];
    atarix_discovery_record_header_v1_t *unknown;
    atarix_discovery_record_header_v1_t *end_record;
    atarix_discovery_header_v1_t *header;
    size_t offset;
    size_t image_size;
    int rc;

    memset(image, 0, sizeof(image));
    header = (atarix_discovery_header_v1_t *)image;
    offset = ATARIX_DISCOVERY_HEADER_SIZE_V1;

    test_init_header(image, 0u, 2u);

    unknown = (atarix_discovery_record_header_v1_t *)(image + offset);
    unknown->type = 0x8000u;
    unknown->version = 1u;
    unknown->length = ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1;
    offset += unknown->length;

    end_record = (atarix_discovery_record_header_v1_t *)(image + offset);
    end_record->type = ATARIX_DISCOVERY_RECORD_END;
    end_record->version = 1u;
    end_record->length = ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1;
    offset += end_record->length;

    header->total_length = (uint32_t)offset;
    image_size = offset;
    test_patch_discovery_crc(image, image_size);

    rc = atarix_discovery_validate(image, image_size);
    if (rc != ATARIX_DISCOVERY_OK) {
        puts("unknown record was not skipped");
        return 1;
    }

    return 0;
}
