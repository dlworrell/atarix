#include <stdint.h>
#include <stdio.h>

#include "atarix/discovery_parser.h"
#include "discovery_test_utils.h"

int main(void) {
    uint8_t image[ATARIX_TEST_IMAGE_SIZE];
    size_t image_size;
    atarix_discovery_database_t db;
    int rc;

    atarix_discovery_database_init(&db);
    image_size = test_make_minimal_discovery(image);
    rc = atarix_discovery_parse(image, image_size, &db);

    if (rc != ATARIX_DISCOVERY_OK) {
        puts("minimal parse failed");
        return 1;
    }

    if (db.resource_count != 0u || db.service_count != 0u) {
        puts("minimal database not empty");
        return 1;
    }

    return 0;
}
