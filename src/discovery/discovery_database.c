#include "atarix/discovery_database.h"

void atarix_discovery_database_init(atarix_discovery_database_t *db) {
    if (!db) {
        return;
    }

    db->resources = 0;
    db->resource_capacity = 0;
    db->resource_count = 0;

    db->services = 0;
    db->service_capacity = 0;
    db->service_count = 0;

    db->dependencies = 0;
    db->dependency_capacity = 0;
    db->dependency_count = 0;

    db->instrumentation = 0;
    db->instrumentation_capacity = 0;
    db->instrumentation_count = 0;
}

const atarix_resource_entry_t *atarix_discovery_find_resource(
    const atarix_discovery_database_t *db,
    atarix_discovery_handle_t handle) {
    size_t i;

    if (!db || !db->resources) {
        return 0;
    }

    for (i = 0; i < db->resource_count; ++i) {
        if (db->resources[i].handle == handle) {
            return &db->resources[i];
        }
    }

    return 0;
}

const atarix_service_entry_t *atarix_discovery_find_service(
    const atarix_discovery_database_t *db,
    atarix_discovery_handle_t handle) {
    size_t i;

    if (!db || !db->services) {
        return 0;
    }

    for (i = 0; i < db->service_count; ++i) {
        if (db->services[i].handle == handle) {
            return &db->services[i];
        }
    }

    return 0;
}

int atarix_discovery_database_contains_handle(
    const atarix_discovery_database_t *db,
    atarix_discovery_handle_t handle) {
    size_t i;

    if (!db) {
        return 0;
    }

    if (db->resources) {
        for (i = 0; i < db->resource_count; ++i) {
            if (db->resources[i].handle == handle) {
                return 1;
            }
        }
    }

    if (db->services) {
        for (i = 0; i < db->service_count; ++i) {
            if (db->services[i].handle == handle) {
                return 1;
            }
        }
    }

    if (db->instrumentation) {
        for (i = 0; i < db->instrumentation_count; ++i) {
            if (db->instrumentation[i].handle == handle) {
                return 1;
            }
        }
    }

    return 0;
}
