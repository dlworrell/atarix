#include "atarix/sim_bib.h"

atarix_bib_builder_result_t atarix_sim_bib_capture(
    const atarix_sim_bib_capture_config_t *config,
    void *buffer,
    size_t capacity,
    uint32_t *serialized_length) {
    atarix_bib_builder_t builder;
    atarix_bib_builder_result_t result;

    if (config == NULL || buffer == NULL || serialized_length == NULL) {
        return ATARIX_BIB_BUILDER_E_ARGUMENT;
    }

    result = atarix_bib_builder_init(
        &builder,
        buffer,
        capacity,
        config->header_flags,
        config->boot_id,
        config->producer_id,
        config->created_counter);
    if (result != ATARIX_BIB_BUILDER_OK) {
        return result;
    }

    result = atarix_bib_builder_add_reference(
        &builder,
        ATARIX_BIB_TLV_SERVICE_DIRECTORY_REFERENCE,
        ATARIX_BIB_TLV_FLAG_MANDATORY,
        config->service_object_id,
        config->service_address,
        config->service_length,
        1u,
        0u);
    if (result != ATARIX_BIB_BUILDER_OK) {
        return result;
    }

    result = atarix_bib_builder_add_reference(
        &builder,
        ATARIX_BIB_TLV_MEMORY_MAP_REFERENCE,
        ATARIX_BIB_TLV_FLAG_MANDATORY,
        config->memory_object_id,
        config->memory_address,
        config->memory_length,
        1u,
        0u);
    if (result != ATARIX_BIB_BUILDER_OK) {
        return result;
    }

    return atarix_bib_builder_seal(&builder, serialized_length);
}
