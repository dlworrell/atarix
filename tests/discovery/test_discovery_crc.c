#include <stdint.h>
#include <stdio.h>

#include "atarix/discovery_parser.h"
#include "discovery_test_utils.h"

int main(void) {
    uint8_t image[ATARIX_TEST_IMAGE_SIZE];
    size_t image_size;
    int result;

    image_size = test_make_minimal_discovery(image);
    result = atarix_discovery_validate(image, image_size);
    if (result != ATARIX_DISCOVERY_OK) {
        printf("valid CRC image rejected: %d\n", result);
        return 1;
    }

    image[image_size - 1u] ^= 0x01u;
    result = atarix_discovery_validate(image, image_size);
    if (result != ATARIX_DISCOVERY_ERROR_BAD_CRC) {
        printf("corrupt CRC image returned: %d\n", result);
        return 1;
    }

    return 0;
}
