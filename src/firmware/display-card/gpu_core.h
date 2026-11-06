/*
 * SPDX-FileCopyrightText: Donovan Worrell
 * SPDX-License-Identifier: Apache-2.0
 *
 * Display card GPU compute core (SERVICE_GPU_COMPUTE, 0x6000).
 *
 * Portable C11, single-precision only, built on the vendored cglm. The same
 * source is linked into the CPU card simulator (sim/cpu-card/gpu_service.cpp)
 * and cross-compiled for the display card's RP2350 Cortex-M33 (FPv5-SP),
 * per ADR-0016.
 *
 * GPU_TRANSFORM_REQUEST payload (44 bytes, little-endian, signed 16.16):
 *   +0 u16 operation (1 = TRANSFORM_POINT)   +2 u16 flags (0)
 *   +4 point x,y,z   +16 axis x,y,z   +28 angle (radians)   +32 translation x,y,z
 * The point is rotated about the normalized axis, then translated.
 * GPU_TRANSFORM_REPLY payload (16 bytes): u32 status, s16.16 x,y,z.
 */

#ifndef ATARIX_GPU_CORE_H
#define ATARIX_GPU_CORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ATARIX_GPU_REQUEST_LENGTH 44u
#define ATARIX_GPU_REPLY_LENGTH 16u
#define ATARIX_GPU_OP_TRANSFORM_POINT 1u

typedef enum atarix_gpu_status {
    ATARIX_GPU_OK = 0,
    ATARIX_GPU_BAD_LENGTH = 1,
    ATARIX_GPU_BAD_OPERATION = 2,
    ATARIX_GPU_OUT_OF_RANGE = 3,
    ATARIX_GPU_ZERO_AXIS = 4
} atarix_gpu_status_t;

/*
 * Validate and execute one request. Always fills all 16 reply bytes (status
 * first; coordinates zero on error) and returns the status.
 */
atarix_gpu_status_t atarix_gpu_transform(const uint8_t *request,
                                         uint16_t length,
                                         uint8_t reply[ATARIX_GPU_REPLY_LENGTH]);

#ifdef __cplusplus
}
#endif

#endif /* ATARIX_GPU_CORE_H */
