#include "atarix/memory.h"

static int atarix_memory_exactly_one_move_form(uint32_t flags)
{
    uint32_t forms = flags & ATARIX_MEMORY_MOVE_FORM_MASK;

    return forms != 0u &&
           (forms & (forms - 1u)) == 0u &&
           (flags & ~ATARIX_MEMORY_MOVE_FORM_MASK) == 0u;
}

atarix_memory_status_t atarix_memory_descriptor_validate(
    const atarix_memory_descriptor_v1_t *descriptor)
{
    if (descriptor == NULL) {
        return ATARIX_MEMORY_STATUS_NULL_DESCRIPTOR;
    }

    if (descriptor->magic != ATARIX_MEMORY_DESCRIPTOR_MAGIC_V1) {
        return ATARIX_MEMORY_STATUS_BAD_MAGIC;
    }

    if (descriptor->version != ATARIX_MEMORY_DESCRIPTOR_VERSION_V1) {
        return ATARIX_MEMORY_STATUS_BAD_VERSION;
    }

    if (descriptor->length == 0u) {
        return ATARIX_MEMORY_STATUS_ZERO_LENGTH;
    }

    if (descriptor->length > ATARIX_MEMORY_DESCRIPTOR_MAX_LENGTH_V1) {
        return ATARIX_MEMORY_STATUS_LENGTH_EXCEEDED;
    }

    if (descriptor->capability_id == 0u) {
        return ATARIX_MEMORY_STATUS_MISSING_CAPABILITY;
    }

    if (descriptor->source_object == 0u) {
        return ATARIX_MEMORY_STATUS_MISSING_SOURCE;
    }

    if (descriptor->destination_object == 0u) {
        return ATARIX_MEMORY_STATUS_MISSING_DESTINATION;
    }

    if (!atarix_memory_exactly_one_move_form(descriptor->flags)) {
        return ATARIX_MEMORY_STATUS_BAD_FLAGS;
    }

    return ATARIX_MEMORY_STATUS_OK;
}

atarix_memory_status_t atarix_validate_descriptor(
    const atarix_memory_descriptor_v1_t *descriptor)
{
    return atarix_memory_descriptor_validate(descriptor);
}
