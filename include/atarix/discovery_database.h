#ifndef ATARIX_DISCOVERY_DATABASE_H
#define ATARIX_DISCOVERY_DATABASE_H

#include <stddef.h>
#include <stdint.h>
#include "atarix/discovery.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct atarix_resource_entry {
    atarix_discovery_handle_t handle;
    uint16_t resource_type;
    uint16_t resource_version;
    uint32_t resource_flags;
    uint32_t provider_id;
    uint32_t instance_id;
} atarix_resource_entry_t;

typedef struct atarix_service_entry {
    atarix_discovery_handle_t handle;
    uint16_t service_id;
    uint16_t service_version;
    uint32_t service_flags;
    uint32_t provider_id;
    uint32_t instance_id;
} atarix_service_entry_t;

typedef struct atarix_dependency_entry {
    atarix_discovery_handle_t consumer_handle;
    atarix_discovery_handle_t provider_handle;
    uint16_t dependency_type;
    uint16_t dependency_version;
    uint32_t dependency_flags;
} atarix_dependency_entry_t;

typedef struct atarix_instrumentation_entry {
    atarix_discovery_handle_t handle;
    atarix_discovery_handle_t observed_handle;
    uint16_t instrumentation_type;
    uint16_t instrumentation_version;
    uint32_t instrumentation_flags;
} atarix_instrumentation_entry_t;

typedef struct atarix_discovery_database {
    atarix_resource_entry_t *resources;
    size_t resource_capacity;
    size_t resource_count;

    atarix_service_entry_t *services;
    size_t service_capacity;
    size_t service_count;

    atarix_dependency_entry_t *dependencies;
    size_t dependency_capacity;
    size_t dependency_count;

    atarix_instrumentation_entry_t *instrumentation;
    size_t instrumentation_capacity;
    size_t instrumentation_count;
} atarix_discovery_database_t;

void atarix_discovery_database_init(atarix_discovery_database_t *db);

const atarix_resource_entry_t *atarix_discovery_find_resource(
    const atarix_discovery_database_t *db,
    atarix_discovery_handle_t handle);

const atarix_service_entry_t *atarix_discovery_find_service(
    const atarix_discovery_database_t *db,
    atarix_discovery_handle_t handle);

int atarix_discovery_database_contains_handle(
    const atarix_discovery_database_t *db,
    atarix_discovery_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_DISCOVERY_DATABASE_H */
