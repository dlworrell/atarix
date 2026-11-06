/*
 * SPDX-FileCopyrightText: Donovan Worrell
 * SPDX-License-Identifier: Apache-2.0
 */

#include "gpu_core.h"

#include <math.h>

#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"

/* Largest float strictly below 2^31: the s16.16 range limit, in float. */
#define GPU_FIXED_LIMIT 2147483520.0f

static int32_t le32s(const uint8_t *p) {
    return (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                     ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static uint16_t le16(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

static void put32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static float from_fixed(const uint8_t *p) {
    return (float)le32s(p) * (1.0f / 65536.0f);
}

static atarix_gpu_status_t reply_status(uint8_t *reply, atarix_gpu_status_t status) {
    unsigned i;
    for (i = 0; i < ATARIX_GPU_REPLY_LENGTH; ++i) {
        reply[i] = 0u;
    }
    put32(reply, (uint32_t)status);
    return status;
}

atarix_gpu_status_t atarix_gpu_transform(const uint8_t *request,
                                         uint16_t length,
                                         uint8_t reply[ATARIX_GPU_REPLY_LENGTH]) {
    vec3 point;
    vec3 axis;
    vec3 translate;
    vec3 result;
    mat4 m;
    float angle;
    int i;

    if (!request || length != ATARIX_GPU_REQUEST_LENGTH) {
        return reply_status(reply, ATARIX_GPU_BAD_LENGTH);
    }
    if (le16(request) != ATARIX_GPU_OP_TRANSFORM_POINT || le16(request + 2) != 0u) {
        return reply_status(reply, ATARIX_GPU_BAD_OPERATION);
    }

    for (i = 0; i < 3; ++i) {
        point[i] = from_fixed(request + 4 + 4 * i);
        axis[i] = from_fixed(request + 16 + 4 * i);
        translate[i] = from_fixed(request + 32 + 4 * i);
    }
    angle = from_fixed(request + 28);
    if (axis[0] == 0.0f && axis[1] == 0.0f && axis[2] == 0.0f) {
        return reply_status(reply, ATARIX_GPU_ZERO_AXIS);
    }

    /* M = T * R; result = M * point (cglm normalizes the axis). */
    glm_translate_make(m, translate);
    glm_rotate(m, angle, axis);
    glm_mat4_mulv3(m, point, 1.0f, result);

    reply_status(reply, ATARIX_GPU_OK);
    for (i = 0; i < 3; ++i) {
        const float scaled = result[i] * 65536.0f;
        if (!isfinite(scaled) || fabsf(scaled) > GPU_FIXED_LIMIT) {
            return reply_status(reply, ATARIX_GPU_OUT_OF_RANGE);
        }
        put32(reply + 4 + 4 * i, (uint32_t)(int32_t)lroundf(scaled));
    }
    return ATARIX_GPU_OK;
}
