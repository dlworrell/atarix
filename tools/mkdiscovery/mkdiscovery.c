#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "atarix/discovery.h"
#include "atarix/resource_types.h"
#include "atarix/service_ids.h"

#define MKDISCOVERY_MAX_IMAGE 4096u

static uint32_t crc32_iso_hdlc(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;

    for (i = 0; i < length; ++i) {
        uint32_t byte = data[i];
        int bit;
        crc ^= byte;
        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

static void patch_crc32(uint8_t *image, size_t image_length) {
    atarix_discovery_header_v1_t *header = (atarix_discovery_header_v1_t *)image;

    header->header_crc32 = 0;
    header->image_crc32 = 0;
    header->header_crc32 = crc32_iso_hdlc(image, ATARIX_DISCOVERY_HEADER_SIZE_V1);
    header->image_crc32 = crc32_iso_hdlc(image, image_length);
}

static int append_bytes(uint8_t *image, size_t image_capacity, size_t *offset, const void *src, size_t length) {
    if (length > image_capacity - *offset) {
        return 1;
    }
    memcpy(image + *offset, src, length); /* aes-sec-001: allow bounded append after explicit remaining-capacity check */
    *offset += length;
    return 0;
}

static int append_end(uint8_t *image, size_t image_capacity, size_t *offset) {
    atarix_discovery_record_header_v1_t end_record;
    memset(&end_record, 0, sizeof(end_record));
    end_record.type = ATARIX_DISCOVERY_RECORD_END;
    end_record.version = 1;
    end_record.length = sizeof(end_record);
    return append_bytes(image, image_capacity, offset, &end_record, sizeof(end_record));
}

static int append_resource(uint8_t *image, size_t image_capacity, size_t *offset, uint16_t type, uint32_t provider_id, uint32_t instance_id) {
    atarix_discovery_resource_record_v1_t record;
    memset(&record, 0, sizeof(record));
    record.header.type = ATARIX_DISCOVERY_RECORD_RESOURCE;
    record.header.version = 1;
    record.header.length = sizeof(record);
    record.resource_type_id = type;
    record.resource_version = 1;
    record.resource_handle = atarix_discovery_make_handle((uint16_t)provider_id, type, instance_id);
    record.provider_id = provider_id;
    record.instance_id = instance_id;
    return append_bytes(image, image_capacity, offset, &record, sizeof(record));
}

static int append_service(uint8_t *image, size_t image_capacity, size_t *offset, uint16_t service_id, uint32_t provider_id, uint32_t instance_id) {
    atarix_discovery_service_record_v1_t record;
    memset(&record, 0, sizeof(record));
    record.header.type = ATARIX_DISCOVERY_RECORD_SERVICE;
    record.header.version = 1;
    record.header.length = sizeof(record);
    record.service_id = service_id;
    record.service_version = 1;
    record.service_handle = atarix_discovery_make_handle((uint16_t)provider_id, service_id, instance_id);
    record.provider_id = provider_id;
    record.instance_id = instance_id;
    return append_bytes(image, image_capacity, offset, &record, sizeof(record));
}

static int build_profile(const char *profile, uint8_t *image, size_t image_capacity, size_t *image_length) {
    atarix_discovery_header_v1_t *header = (atarix_discovery_header_v1_t *)image;
    size_t offset = ATARIX_DISCOVERY_HEADER_SIZE_V1;
    uint32_t provider_id = 3;
    uint32_t record_count = 0;
    int append_status = 0;

    memset(image, 0, image_capacity);
    header->magic = ATARIX_DISCOVERY_MAGIC_ATDX;
    header->version_major = ATARIX_DISCOVERY_VERSION_MAJOR_V1;
    header->version_minor = ATARIX_DISCOVERY_VERSION_MINOR_V0;
    header->provider_id = provider_id;

    if (strcmp(profile, "minimal") == 0) {
        /* END only. */
    } else if (strcmp(profile, "cpu-card") == 0) {
        append_status |= append_resource(image, image_capacity, &offset, ATARIX_RESOURCE_CPU, provider_id, 1); record_count++;
        append_status |= append_resource(image, image_capacity, &offset, ATARIX_RESOURCE_CPU_HEALTH, provider_id, 1); record_count++;
        append_status |= append_service(image, image_capacity, &offset, ATARIX_SERVICE_CPU_DIAGNOSTICS, provider_id, 1); record_count++;
        append_status |= append_service(image, image_capacity, &offset, ATARIX_SERVICE_CPU_HEALTH, provider_id, 1); record_count++;
    } else if (strcmp(profile, "supervisor") == 0) {
        provider_id = 1;
        header->provider_id = provider_id;
        append_status |= append_resource(image, image_capacity, &offset, ATARIX_RESOURCE_SUPERVISOR, provider_id, 1); record_count++;
        append_status |= append_service(image, image_capacity, &offset, ATARIX_SERVICE_HEALTH, provider_id, 1); record_count++;
    } else if (strcmp(profile, "instrumentation") == 0) {
        provider_id = 5;
        header->provider_id = provider_id;
        append_status |= append_resource(image, image_capacity, &offset, ATARIX_RESOURCE_LOGIC_ANALYZER, provider_id, 1); record_count++;
        append_status |= append_service(image, image_capacity, &offset, ATARIX_SERVICE_INSTRUMENTATION, provider_id, 1); record_count++;
    } else {
        fprintf(stderr, "unknown profile: %s\n", profile);
        return 1;
    }

    append_status |= append_end(image, image_capacity, &offset); record_count++;
    if (append_status != 0) {
        fprintf(stderr, "profile does not fit discovery image capacity\n");
        return 1;
    }
    header->total_length = (uint32_t)offset;
    header->record_count = record_count;
    patch_crc32(image, offset);
    *image_length = offset;
    return 0;
}

int main(int argc, char **argv) {
    const char *profile = 0;
    const char *output = 0;
    uint8_t image[MKDISCOVERY_MAX_IMAGE];
    size_t image_length = 0;
    FILE *fp;
    int i;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--profile") == 0 && i + 1 < argc) {
            profile = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output = argv[++i];
        }
    }

    if (!profile || !output) {
        fprintf(stderr, "usage: %s --profile <minimal|cpu-card|supervisor|instrumentation> --output <file>\n", argv[0]);
        return 2;
    }

    if (build_profile(profile, image, sizeof(image), &image_length) != 0) {
        return 1;
    }

    fp = fopen(output, "wb");
    if (!fp) {
        return 1;
    }

    if (fwrite(image, 1, image_length, fp) != image_length) {
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}
