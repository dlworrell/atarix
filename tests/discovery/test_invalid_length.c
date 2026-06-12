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
    header->total_length = 7u;

    rc = atarix_discovery_validate(image, image_size);
    if (rc != ATARIX_DISCOVERY_ERROR_BAD_LENGTH) {
        puts("invalid length was not rejected");
        return 1;
    }

    return 0;
}
