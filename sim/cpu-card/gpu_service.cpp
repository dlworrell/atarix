// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0

#include "gpu_service.hpp"

#include <cmath>

#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"

namespace atarix::sim {

namespace {

int32_t le32s(const uint8_t *p) {
	return int32_t(uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24));
}

uint16_t le16(const uint8_t *p) {
	return uint16_t(p[0] | (p[1] << 8));
}

void put32(uint8_t *p, uint32_t v) {
	for(int i = 0; i < 4; ++i) p[i] = uint8_t(v >> (8 * i));
}

float from_fixed(const uint8_t *p) {
	return float(le32s(p)) / 65536.0f;
}

GpuReply reply_status(GpuStatus status) {
	GpuReply reply{status, {}};
	put32(reply.payload.data(), status);
	return reply;
}

}  // namespace

GpuReply gpu_transform(const uint8_t *payload, uint16_t length, GpuFault fault) {
	if(length != GpuRequestLength) return reply_status(GpuBadLength);
	if(le16(payload) != GpuOpTransformPoint || le16(payload + 2) != 0) return reply_status(GpuBadOperation);

	vec3 point = {from_fixed(payload + 4), from_fixed(payload + 8), from_fixed(payload + 12)};
	vec3 axis = {from_fixed(payload + 16), from_fixed(payload + 20), from_fixed(payload + 24)};
	float angle = from_fixed(payload + 28);
	vec3 translate = {from_fixed(payload + 32), from_fixed(payload + 36), from_fixed(payload + 40)};
	if(axis[0] == 0.0f && axis[1] == 0.0f && axis[2] == 0.0f) return reply_status(GpuZeroAxis);
	if(fault == GpuFault::NegateAngle) angle = -angle;

	// M = T * R; result = M * point (cglm normalizes the axis).
	mat4 m;
	glm_translate_make(m, translate);
	glm_rotate(m, angle, axis);
	vec3 result;
	glm_mat4_mulv3(m, point, 1.0f, result);

	GpuReply reply{GpuOk, {}};
	for(int i = 0; i < 3; ++i) {
		const double scaled = double(result[i]) * 65536.0;
		if(!std::isfinite(scaled) || std::fabs(scaled) >= 2147483647.0) return reply_status(GpuOutOfRange);
		int32_t fixed = int32_t(std::lround(scaled));
		if(fault == GpuFault::BiasX3 && i == 0) fixed += 3;
		put32(reply.payload.data() + 4 + 4 * i, uint32_t(fixed));
	}
	put32(reply.payload.data(), GpuOk);
	return reply;
}

}  // namespace atarix::sim
