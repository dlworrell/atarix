#include <stdint.h>
#include <stdio.h>

#include "atarix/discovery_parser.h"
#include "discovery_test_utils.h"

static int expect_result(const char *name, int got, int expected) {
    if (got != expected) {
        printf("%s: got %d expected %d\n", name, got, expected);
        return 1;
    }
    return 0;
}

int main(void) {
    uint8_t image[ATARIX_TEST_IMAGE_SIZE];
    size_t image_size;
    atarix_discovery_header_v1_t *header;
    int failures = 0;

    image_size = test_make_minimal_discovery(image);
    failures += expect_result("valid minimal", atarix_discovery_validate(image, image_size), ATARIX_DISCOVERY_OK);

    image_size = test_make_minimal_discovery(image);
    header = (atarix_discovery_header_v1_t *)image;
    header->magic = 0x11111111u;
    failures += expect_result("bad magic", atarix_discovery_validate(image, image_size), ATARIX_DISCOVERY_ERROR_BAD_MAGIC);

    image_size = test_make_minimal_discovery(image);
    header = (atarix_discovery_header_v1_t *)image;
    header->version_major = 99u;
    failures += expect_result("bad version", atarix_discovery_validate(image, image_size), ATARIX_DISCOVERY_ERROR_UNSUPPORTED_VERSION);

    return failures ? 1 : 0;
}
