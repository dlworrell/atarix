#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "atarix/discovery_parser.h"
#include "discovery_test_utils.h"

int main(void) {
    uint8_t image[ATARIX_TEST_IMAGE_SIZE];
    atarix_discovery_resource_record_v1_t *first;
    atarix_discovery_resource_record_v1_t *second;
    atarix_discovery_record_header_v1_t *end_record;
    atarix_discovery_header_v1_t *header;
    atarix_resource_entry_t resources[4];
    atarix_discovery_database_t db;
    size_t offset;
    size_t image_size;
    atarix_discovery_handle_t handle;
    int rc;

    memset(image, 0, sizeof(image));
    header = (atarix_discovery_header_v1_t *)image;
    offset = ATARIX_DISCOVERY_HEADER_SIZE_V1;
    handle = atarix_discovery_make_handle(3, ATARIX_RESOURCE_CPU, 1);

    test_init_header(image, 0u, 3u);

    first = (atarix_discovery_resource_record_v1_t *)(image + offset);
    first->header.type = ATARIX_DISCOVERY_RECORD_RESOURCE;
    first->header.version = 1u;
    first->header.length = sizeof(*first);
    first->resource_type_id = ATARIX_RESOURCE_CPU;
    first->resource_version = 1u;
    first->resource_handle = handle;
    first->provider_id = 3u;
    first->instance_id = 1u;
    offset += sizeof(*first);

    second = (atarix_discovery_resource_record_v1_t *)(image + offset);
    second->header.type = ATARIX_DISCOVERY_RECORD_RESOURCE;
    second->header.version = 1u;
    second->header.length = sizeof(*second);
    second->resource_type_id = ATARIX_RESOURCE_CPU_HEALTH;
    second->resource_version = 1u;
    second->resource_handle = handle;
    second->provider_id = 3u;
    second->instance_id = 2u;
    offset += sizeof(*second);

    end_record = (atarix_discovery_record_header_v1_t *)(image + offset);
    end_record->type = ATARIX_DISCOVERY_RECORD_END;
    end_record->version = 1u;
    end_record->length = ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1;
    offset += end_record->length;

    header->total_length = (uint32_t)offset;
    image_size = offset;
    test_patch_discovery_crc(image, image_size);

    atarix_discovery_database_init(&db);
    db.resources = resources;
    db.resource_capacity = 4u;

    rc = atarix_discovery_parse(image, image_size, &db);
    if (rc != ATARIX_DISCOVERY_ERROR_DUPLICATE_HANDLE) {
        puts("duplicate handle was not rejected");
        return 1;
    }

    return 0;
}
