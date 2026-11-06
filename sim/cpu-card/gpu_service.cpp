// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Simulator wrapper around the display card firmware's GPU core
// (src/firmware/display-card/gpu_core.c): the bench runs the same C code the
// RP2350 runs (ADR-0016). Negative-control faults are applied outside the
// core, so the core itself is never modified for testing.

#include "gpu_service.hpp"

#include <algorithm>

extern "C" {
#include "gpu_core.h"
}

namespace atarix::sim {

GpuReply gpu_transform(const uint8_t *payload, uint16_t length, GpuFault fault) {
	std::array<uint8_t, GpuRequestLength> request{};
	std::copy(payload, payload + std::min<size_t>(length, request.size()), request.begin());

	if(fault == GpuFault::NegateAngle && length == GpuRequestLength) {
		// Negate the s16.16 angle field (+28): the service rotates the wrong way.
		uint32_t angle = uint32_t(request[28]) | (uint32_t(request[29]) << 8) |
			(uint32_t(request[30]) << 16) | (uint32_t(request[31]) << 24);
		angle = uint32_t(-int32_t(angle));
		for(int i = 0; i < 4; ++i) request[28 + i] = uint8_t(angle >> (8 * i));
	}

	GpuReply reply{};
	reply.status = GpuStatus(atarix_gpu_transform(request.data(), length, reply.payload.data()));

	if(fault == GpuFault::BiasX3 && reply.status == GpuOk) {
		uint32_t x = uint32_t(reply.payload[4]) | (uint32_t(reply.payload[5]) << 8) |
			(uint32_t(reply.payload[6]) << 16) | (uint32_t(reply.payload[7]) << 24);
		x += 3u;
		for(int i = 0; i < 4; ++i) reply.payload[4 + i] = uint8_t(x >> (8 * i));
	}
	return reply;
}

}  // namespace atarix::sim
