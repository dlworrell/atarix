#include <stdint.h>
#include <stdio.h>

#include "atarix/discovery_parser.h"
#include "discovery_test_utils.h"

int main(void) {
    uint8_t image[ATARIX_TEST_IMAGE_SIZE];
    atarix_discovery_header_v1_t *header;
    size_t image_size;
    int rc;

    image_size = test_make_minimal_discovery(image);
    header = (atarix_discovery_header_v1_t *)image;
    header->total_length = ATARIX_DISCOVERY_HEADER_SIZE_V1;
    header->record_count = 0;
    test_patch_discovery_crc(image, header->total_length);

    rc = atarix_discovery_validate(image, image_size);
    if (rc != ATARIX_DISCOVERY_ERROR_MISSING_END) {
        puts("missing END was not rejected");
        return 1;
    }

    return 0;
}
