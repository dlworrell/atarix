#include "atarix/discovery_parser.h"

static int add_resource(
    atarix_discovery_database_t *db,
    const atarix_discovery_resource_record_v1_t *record) {
    if (atarix_discovery_database_contains_handle(db, record->resource_handle)) {
        return ATARIX_DISCOVERY_ERROR_DUPLICATE_HANDLE;
    }

    if (db->resource_count >= db->resource_capacity || !db->resources) {
        return ATARIX_DISCOVERY_ERROR_CAPACITY;
    }

    db->resources[db->resource_count].handle = record->resource_handle;
    db->resources[db->resource_count].resource_type = record->resource_type_id;
    db->resources[db->resource_count].resource_version = record->resource_version;
    db->resources[db->resource_count].resource_flags = record->resource_flags;
    db->resources[db->resource_count].provider_id = record->provider_id;
    db->resources[db->resource_count].instance_id = record->instance_id;
    db->resource_count++;

    return ATARIX_DISCOVERY_OK;
}

static int add_service(
    atarix_discovery_database_t *db,
    const atarix_discovery_service_record_v1_t *record) {
    if (atarix_discovery_database_contains_handle(db, record->service_handle)) {
        return ATARIX_DISCOVERY_ERROR_DUPLICATE_HANDLE;
    }

    if (db->service_count >= db->service_capacity || !db->services) {
        return ATARIX_DISCOVERY_ERROR_CAPACITY;
    }

    db->services[db->service_count].handle = record->service_handle;
    db->services[db->service_count].service_id = record->service_id;
    db->services[db->service_count].service_version = record->service_version;
    db->services[db->service_count].service_flags = record->service_flags;
    db->services[db->service_count].provider_id = record->provider_id;
    db->services[db->service_count].instance_id = record->instance_id;
    db->service_count++;

    return ATARIX_DISCOVERY_OK;
}

static int add_dependency(
    atarix_discovery_database_t *db,
    const atarix_discovery_dependency_record_v1_t *record) {
    if (db->dependency_count >= db->dependency_capacity || !db->dependencies) {
        return ATARIX_DISCOVERY_ERROR_CAPACITY;
    }

    db->dependencies[db->dependency_count].consumer_handle = record->consumer_handle;
    db->dependencies[db->dependency_count].provider_handle = record->provider_handle;
    db->dependencies[db->dependency_count].dependency_type = record->dependency_type;
    db->dependencies[db->dependency_count].dependency_version = record->dependency_version;
    db->dependencies[db->dependency_count].dependency_flags = record->dependency_flags;
    db->dependency_count++;

    return ATARIX_DISCOVERY_OK;
}

static int add_instrumentation(
    atarix_discovery_database_t *db,
    const atarix_discovery_instrumentation_record_v1_t *record) {
    if (atarix_discovery_database_contains_handle(db, record->instrumentation_handle)) {
        return ATARIX_DISCOVERY_ERROR_DUPLICATE_HANDLE;
    }

    if (db->instrumentation_count >= db->instrumentation_capacity || !db->instrumentation) {
        return ATARIX_DISCOVERY_ERROR_CAPACITY;
    }

    db->instrumentation[db->instrumentation_count].handle = record->instrumentation_handle;
    db->instrumentation[db->instrumentation_count].observed_handle = record->observed_handle;
    db->instrumentation[db->instrumentation_count].instrumentation_type = record->instrumentation_type;
    db->instrumentation[db->instrumentation_count].instrumentation_version = record->instrumentation_version;
    db->instrumentation[db->instrumentation_count].instrumentation_flags = record->instrumentation_flags;
    db->instrumentation_count++;

    return ATARIX_DISCOVERY_OK;
}

int atarix_discovery_parse(
    const void *image,
    size_t image_size,
    atarix_discovery_database_t *db) {
    const atarix_discovery_header_v1_t *header;
    const atarix_discovery_record_header_v1_t *record;
    int result;

    if (!db) {
        return ATARIX_DISCOVERY_ERROR_NULL;
    }

    result = atarix_discovery_validate(image, image_size);
    if (result != ATARIX_DISCOVERY_OK) {
        return result;
    }

    header = (const atarix_discovery_header_v1_t *)image;
    record = atarix_discovery_first_record(header, image_size);

    while (record) {
        if (record->type == ATARIX_DISCOVERY_RECORD_END) {
            return ATARIX_DISCOVERY_OK;
        }

        switch (record->type) {
        case ATARIX_DISCOVERY_RECORD_RESOURCE:
            result = add_resource(db, (const atarix_discovery_resource_record_v1_t *)record);
            break;
        case ATARIX_DISCOVERY_RECORD_SERVICE:
            result = add_service(db, (const atarix_discovery_service_record_v1_t *)record);
            break;
        case ATARIX_DISCOVERY_RECORD_DEPENDENCY:
            result = add_dependency(db, (const atarix_discovery_dependency_record_v1_t *)record);
            break;
        case ATARIX_DISCOVERY_RECORD_INSTRUMENTATION:
            result = add_instrumentation(db, (const atarix_discovery_instrumentation_record_v1_t *)record);
            break;
        default:
            result = ATARIX_DISCOVERY_OK;
            break;
        }

        if (result != ATARIX_DISCOVERY_OK) {
            return result;
        }

        record = atarix_discovery_next_record(header, image_size, record);
    }

    return ATARIX_DISCOVERY_ERROR_MISSING_END;
}
