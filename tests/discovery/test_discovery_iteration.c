#include <stdint.h>
#include <stdio.h>

#include "atarix/discovery_parser.h"
#include "discovery_test_utils.h"

int main(void) {
    uint8_t image[ATARIX_TEST_IMAGE_SIZE];
    size_t image_size;
    atarix_discovery_header_v1_t *header;
    const atarix_discovery_record_header_v1_t *record;

    image_size = test_make_minimal_discovery(image);
    header = (atarix_discovery_header_v1_t *)image;
    record = atarix_discovery_first_record(header, image_size);

    if (!record) {
        printf("first record missing\n");
        return 1;
    }

    if (record->type != ATARIX_DISCOVERY_RECORD_END) {
        printf("expected END record\n");
        return 1;
    }

    return 0;
}
