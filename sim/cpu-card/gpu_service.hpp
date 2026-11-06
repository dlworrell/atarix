// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Simulated GPU compute service (SERVICE_GPU_COMPUTE, 0x6000) on a display
// card node. It stands in for firmware the display card's own processor would
// run from its boot ROM, and uses the vendored cglm (third_party/cglm) as the
// reference math.
//
// GPU_TRANSFORM_REQUEST payload (44 bytes, little-endian), 16.16 fixed point so
// the W65C816 can build and read it with integer arithmetic:
//   +0  u16 operation (1 = TRANSFORM_POINT)   +2 u16 flags (must be 0)
//   +4  point x, y, z   +16 axis x, y, z   +28 angle (radians)   +32 translation x, y, z
// The point is rotated about the (normalized) axis, then translated.
// GPU_TRANSFORM_REPLY payload (16 bytes): u32 status, s16.16 x, y, z.

#pragma once

#include <array>
#include <cstdint>

namespace atarix::sim {

enum GpuStatus: uint32_t {
	GpuOk = 0,
	GpuBadLength = 1,
	GpuBadOperation = 2,
	GpuOutOfRange = 3,
	GpuZeroAxis = 4,
};

constexpr uint16_t GpuRequestLength = 44;
constexpr uint16_t GpuReplyLength = 16;
constexpr uint16_t GpuOpTransformPoint = 1;

struct GpuReply {
	GpuStatus status;
	std::array<uint8_t, GpuReplyLength> payload;
};

// Deliberate defects, for negative-control runs only.
enum class GpuFault {
	None,
	NegateAngle,   // rotates the wrong way
	BiasX3,        // x off by 3 LSB: just outside the firmware's +/-2 tolerance
};

GpuReply gpu_transform(const uint8_t *payload, uint16_t length, GpuFault fault = GpuFault::None);

}  // namespace atarix::sim
