#ifndef ATARIX_DISCOVERY_TEST_UTILS_H
#define ATARIX_DISCOVERY_TEST_UTILS_H

#include <stdint.h>
#include <string.h>

#include "atarix/discovery.h"
#include "atarix/resource_types.h"
#include "atarix/service_ids.h"

#define ATARIX_TEST_IMAGE_SIZE 512u

static uint32_t test_crc32_iso_hdlc(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;

    for (i = 0; i < length; ++i) {
        int bit;
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

static void test_patch_discovery_crc(uint8_t *image, size_t image_size) {
    atarix_discovery_header_v1_t *header = (atarix_discovery_header_v1_t *)image;
    header->header_crc32 = 0;
    header->image_crc32 = 0;
    header->header_crc32 = test_crc32_iso_hdlc(image, ATARIX_DISCOVERY_HEADER_SIZE_V1);
    header->image_crc32 = test_crc32_iso_hdlc(image, image_size);
}

static void test_init_header(uint8_t *image, size_t total_length, uint32_t record_count) {
    atarix_discovery_header_v1_t *header = (atarix_discovery_header_v1_t *)image;
    memset(image, 0, ATARIX_TEST_IMAGE_SIZE);
    header->magic = ATARIX_DISCOVERY_MAGIC_ATDX;
    header->version_major = ATARIX_DISCOVERY_VERSION_MAJOR_V1;
    header->version_minor = ATARIX_DISCOVERY_VERSION_MINOR_V0;
    header->total_length = (uint32_t)total_length;
    header->record_count = record_count;
    header->provider_id = 3;
}

static size_t test_make_minimal_discovery(uint8_t *image) {
    atarix_discovery_record_header_v1_t *end_record;
    size_t total = ATARIX_DISCOVERY_HEADER_SIZE_V1 + ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1;
    test_init_header(image, total, 1);
    end_record = (atarix_discovery_record_header_v1_t *)(image + ATARIX_DISCOVERY_HEADER_SIZE_V1);
    end_record->type = ATARIX_DISCOVERY_RECORD_END;
    end_record->version = 1;
    end_record->length = ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1;
    test_patch_discovery_crc(image, total);
    return total;
}

static size_t test_make_cpu_discovery(uint8_t *image) {
    size_t offset = ATARIX_DISCOVERY_HEADER_SIZE_V1;
    atarix_discovery_resource_record_v1_t *resource;
    atarix_discovery_service_record_v1_t *service;
    atarix_discovery_record_header_v1_t *end_record;
    size_t total;

    test_init_header(image, 0, 3);

    resource = (atarix_discovery_resource_record_v1_t *)(image + offset);
    memset(resource, 0, sizeof(*resource));
    resource->header.type = ATARIX_DISCOVERY_RECORD_RESOURCE;
    resource->header.version = 1;
    resource->header.length = sizeof(*resource);
    resource->resource_type_id = ATARIX_RESOURCE_CPU;
    resource->resource_version = 1;
    resource->resource_handle = atarix_discovery_make_handle(3, ATARIX_RESOURCE_CPU, 1);
    resource->provider_id = 3;
    resource->instance_id = 1;
    offset += sizeof(*resource);

    service = (atarix_discovery_service_record_v1_t *)(image + offset);
    memset(service, 0, sizeof(*service));
    service->header.type = ATARIX_DISCOVERY_RECORD_SERVICE;
    service->header.version = 1;
    service->header.length = sizeof(*service);
    service->service_id = ATARIX_SERVICE_CPU_HEALTH;
    service->service_version = 1;
    service->service_handle = atarix_discovery_make_handle(3, ATARIX_SERVICE_CPU_HEALTH, 1);
    service->provider_id = 3;
    service->instance_id = 1;
    offset += sizeof(*service);

    end_record = (atarix_discovery_record_header_v1_t *)(image + offset);
    end_record->type = ATARIX_DISCOVERY_RECORD_END;
    end_record->version = 1;
    end_record->length = ATARIX_DISCOVERY_RECORD_HEADER_SIZE_V1;
    offset += sizeof(*end_record);

    total = offset;
    ((atarix_discovery_header_v1_t *)image)->total_length = (uint32_t)total;
    test_patch_discovery_crc(image, total);
    return total;
}

#endif /* ATARIX_DISCOVERY_TEST_UTILS_H */
