#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atarix/bib_reader.h"

static const char *error_name(atarix_bib_error_t error) {
    switch (error) {
    case ATARIX_BIB_OK: return "ok";
    case ATARIX_BIB_E_WINDOW: return "invalid-window";
    case ATARIX_BIB_E_MAGIC: return "invalid-magic";
    case ATARIX_BIB_E_VERSION: return "unsupported-version";
    case ATARIX_BIB_E_HEADER_LENGTH: return "invalid-header-length";
    case ATARIX_BIB_E_TOTAL_LENGTH: return "invalid-total-length";
    case ATARIX_BIB_E_CRC: return "crc-mismatch";
    case ATARIX_BIB_E_HEADER_RESERVED: return "invalid-header-reserved";
    case ATARIX_BIB_E_TLV_BOUNDS: return "invalid-tlv-bounds";
    case ATARIX_BIB_E_TLV_RESERVED: return "invalid-tlv-reserved";
    case ATARIX_BIB_E_UNKNOWN_MANDATORY: return "unknown-mandatory-record";
    case ATARIX_BIB_E_REQUIRED_RECORD: return "required-record-error";
    case ATARIX_BIB_E_REFERENCE_OVERFLOW: return "reference-overflow";
    case ATARIX_BIB_E_REFERENCE_AUTHORITY: return "reference-authority-error";
    case ATARIX_BIB_E_REFERENCE_INTEGRITY: return "reference-integrity-error";
    }
    return "unknown-error";
}

static const char *type_name(uint16_t type) {
    switch (type) {
    case ATARIX_BIB_TLV_RESERVED: return "reserved";
    case ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE: return "service-directory-reference";
    case ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE: return "memory-map-reference";
    case ATARIX_BIB_TLV_BOOT_DEVICE_IDENTITY: return "boot-device-identity";
    case ATARIX_BIB_TLV_BOOT_IMAGE_IDENTITY: return "boot-image-identity";
    case ATARIX_BIB_TLV_BOOT_ARGUMENTS: return "boot-arguments";
    case ATARIX_BIB_TLV_CONSOLE_SERVICE_IDENTITY: return "console-service-identity";
    case ATARIX_BIB_TLV_SUPERVISOR_SERVICE_IDENTITY: return "supervisor-service-identity";
    case ATARIX_BIB_TLV_AUDIT_LOG_REFERENCE: return "audit-log-reference";
    case ATARIX_BIB_TLV_FIRMWARE_BUILD_IDENTITY: return "firmware-build-identity";
    case ATARIX_BIB_TLV_ENTROPY_SEED_REFERENCE: return "entropy-seed-reference";
    default: return atarix_bib_tlv_type_is_experimental(type) ? "experimental" : "unknown";
    }
}

static int is_reference_type(uint16_t type) {
    return type == ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE ||
           type == ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE ||
           type == ATARIX_BIB_TLV_AUDIT_LOG_REFERENCE ||
           type == ATARIX_BIB_TLV_ENTROPY_SEED_REFERENCE;
}

static int read_file(const char *path, uint8_t **buffer, size_t *length) {
    FILE *stream;
    long size;
    uint8_t *data;

    stream = fopen(path, "rb");
    if (stream == NULL) {
        fprintf(stderr, "atarix-bibdump: cannot open %s: %s\n", path, strerror(errno));
        return 1;
    }
    if (fseek(stream, 0, SEEK_END) != 0 || (size = ftell(stream)) < 0 ||
        fseek(stream, 0, SEEK_SET) != 0) {
        fprintf(stderr, "atarix-bibdump: cannot size %s\n", path);
        fclose(stream);
        return 1;
    }
    if ((unsigned long)size > ATARIX_BIB_RECOMMENDED_MAX_SIZE_V1) {
        fprintf(stderr, "atarix-bibdump: %s exceeds the BIB v1 diagnostic limit\n", path);
        fclose(stream);
        return 1;
    }
    data = (uint8_t *)malloc((size_t)size == 0u ? 1u : (size_t)size);
    if (data == NULL) {
        fprintf(stderr, "atarix-bibdump: allocation failed\n");
        fclose(stream);
        return 1;
    }
    if ((size_t)size != 0u && fread(data, 1u, (size_t)size, stream) != (size_t)size) {
        fprintf(stderr, "atarix-bibdump: cannot read %s\n", path);
        free(data);
        fclose(stream);
        return 1;
    }
    fclose(stream);
    *buffer = data;
    *length = (size_t)size;
    return 0;
}

static void print_header_flags(uint16_t flags) {
    printf("flags: 0x%04" PRIx16, flags);
    if (flags == 0u) {
        printf(" none");
    }
    if ((flags & ATARIX_BIB_FLAG_RECOVERY_BOOT) != 0u) printf(" recovery");
    if ((flags & ATARIX_BIB_FLAG_DEGRADED_BOOT) != 0u) printf(" degraded");
    if ((flags & ATARIX_BIB_FLAG_SECURE_POLICY_ACTIVE) != 0u) printf(" secure-policy");
    if ((flags & ATARIX_BIB_FLAG_DIAGNOSTIC_BOOT) != 0u) printf(" diagnostic");
    putchar('\n');
}

static int dump_bib(const uint8_t *buffer, size_t length) {
    atarix_bib_view_t view;
    atarix_bib_error_t error;
    uint32_t index;

    error = atarix_bib_reader_open(&view, buffer, length);
    if (error != ATARIX_BIB_OK) {
        fprintf(stderr, "atarix-bibdump: invalid BIB: %s (%d)\n", error_name(error), (int)error);
        return 2;
    }

    printf("format: ATARIX-BIB\n");
    printf("version: 1.0\n");
    printf("total-length: %" PRIu32 "\n", view.total_length);
    print_header_flags(view.flags);
    printf("boot-id: 0x%016" PRIx64 "\n", view.boot_id);
    printf("producer-id: 0x%016" PRIx64 "\n", view.producer_id);
    printf("created-counter: %" PRIu64 "\n", view.created_counter);
    printf("record-count: %" PRIu32 "\n", view.tlv_count);

    for (index = 0; index < view.tlv_count; ++index) {
        atarix_bib_record_view_t record;
        error = atarix_bib_reader_record(&view, index, &record);
        if (error != ATARIX_BIB_OK) {
            fprintf(stderr, "atarix-bibdump: record %" PRIu32 ": %s\n", index, error_name(error));
            return 2;
        }
        printf("record[%" PRIu32 "].type: 0x%04" PRIx16 " %s\n",
               index, record.type, type_name(record.type));
        printf("record[%" PRIu32 "].flags: 0x%04" PRIx16 "%s%s\n",
               index, record.flags,
               (record.flags & ATARIX_BIB_TLV_FLAG_MANDATORY) != 0u ? " mandatory" : "",
               (record.flags & ATARIX_BIB_TLV_FLAG_SENSITIVE) != 0u ? " sensitive" : "");
        printf("record[%" PRIu32 "].payload-length: %" PRIu32 "\n", index, record.payload_length);

        if (is_reference_type(record.type)) {
            atarix_bib_reference_view_t reference;
            error = atarix_bib_reader_find_reference(&view, record.type, &reference);
            if (error != ATARIX_BIB_OK) {
                fprintf(stderr, "atarix-bibdump: reference %" PRIu32 ": %s\n", index, error_name(error));
                return 2;
            }
            printf("record[%" PRIu32 "].object-id: 0x%016" PRIx64 "\n", index, reference.object_id);
            printf("record[%" PRIu32 "].address: 0x%016" PRIx64 "\n", index, reference.address);
            printf("record[%" PRIu32 "].length: %" PRIu64 "\n", index, reference.length);
            printf("record[%" PRIu32 "].format: %" PRIu32 ".%" PRIu32 "\n",
                   index, reference.format_major, reference.format_minor);
            printf("record[%" PRIu32 "].integrity-kind: %" PRIu32 "\n", index, reference.integrity_kind);
            printf("record[%" PRIu32 "].integrity-length: %" PRIu32 "\n", index, reference.integrity_length);
        } else if ((record.flags & ATARIX_BIB_TLV_FLAG_SENSITIVE) != 0u) {
            printf("record[%" PRIu32 "].payload: <redacted>\n", index);
        } else {
            printf("record[%" PRIu32 "].payload: <opaque>\n", index);
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    uint8_t *buffer;
    size_t length;
    int result;

    if (argc != 2) {
        fprintf(stderr, "usage: atarix-bibdump FILE\n");
        return 64;
    }
    if (read_file(argv[1], &buffer, &length) != 0) {
        return 1;
    }
    result = dump_bib(buffer, length);
    free(buffer);
    return result;
}
