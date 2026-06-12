#ifndef ATARIX_DISCOVERY_PARSER_H
#define ATARIX_DISCOVERY_PARSER_H

#include <stddef.h>
#include <stdint.h>
#include "atarix/discovery.h"
#include "atarix/discovery_database.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum atarix_discovery_result {
    ATARIX_DISCOVERY_OK = 0,
    ATARIX_DISCOVERY_ERROR_NULL = -1,
    ATARIX_DISCOVERY_ERROR_TOO_SMALL = -2,
    ATARIX_DISCOVERY_ERROR_BAD_MAGIC = -3,
    ATARIX_DISCOVERY_ERROR_UNSUPPORTED_VERSION = -4,
    ATARIX_DISCOVERY_ERROR_BAD_LENGTH = -5,
    ATARIX_DISCOVERY_ERROR_BAD_ALIGNMENT = -6,
    ATARIX_DISCOVERY_ERROR_TRUNCATED_RECORD = -7,
    ATARIX_DISCOVERY_ERROR_MISSING_END = -8,
    ATARIX_DISCOVERY_ERROR_DUPLICATE_HANDLE = -9,
    ATARIX_DISCOVERY_ERROR_CAPACITY = -10,
    ATARIX_DISCOVERY_ERROR_BAD_CRC = -11
} atarix_discovery_result_t;

int atarix_discovery_validate(const void *image, size_t image_size);

const atarix_discovery_record_header_v1_t *atarix_discovery_first_record(
    const atarix_discovery_header_v1_t *header,
    size_t image_size);

const atarix_discovery_record_header_v1_t *atarix_discovery_next_record(
    const atarix_discovery_header_v1_t *header,
    size_t image_size,
    const atarix_discovery_record_header_v1_t *record);

int atarix_discovery_parse(
    const void *image,
    size_t image_size,
    atarix_discovery_database_t *db);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_DISCOVERY_PARSER_H */
