// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0

#include "northbridge.hpp"

#include <utility>

#include "gpu_service.hpp"

extern "C" {
#include "atarix/operations.h"
#include "atarix/ring.h"
}

namespace atarix::sim {

namespace {

uint16_t le16(const uint8_t *p) {
	return uint16_t(p[0] | (p[1] << 8));
}

uint32_t le32(const uint8_t *p) {
	return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

void put16(uint8_t *p, uint16_t v) {
	p[0] = uint8_t(v);
	p[1] = uint8_t(v >> 8);
}

void put32(uint8_t *p, uint32_t v) {
	for(int i = 0; i < 4; ++i) p[i] = uint8_t(v >> (8 * i));
}

}  // namespace

Northbridge::Northbridge(uint16_t cpu_node, atarix_ring_t cpu_ring, atarix_trust_level_t cpu_trust)
	: cpu_node_(cpu_node), cpu_ring_(cpu_ring), cpu_trust_(cpu_trust) {}

void Northbridge::add_endpoint(const Endpoint &endpoint) {
	endpoints_.push_back(endpoint);
}

void Northbridge::grant(const atarix_capability_record_v1_t &capability) {
	capabilities_.push_back(capability);
}

atarix_discovery_handle_t Northbridge::handle_for(const Endpoint &endpoint) {
	return atarix_discovery_make_handle(endpoint.node_id, endpoint.service_type, 0);
}

// Test-fixture mapping from message type to the operation a capability must
// grant. Query-style messages need READ; state-changing messages need WRITE or
// CONTROL. Recorded as an open architecture decision in the test-bench doc.
uint32_t Northbridge::operation_for(uint16_t message_type) {
	switch(message_type) {
		case ATARIX_MAILBOX_MESSAGE_PING:
		case ATARIX_MAILBOX_MESSAGE_VERSION_QUERY:
		case ATARIX_MAILBOX_MESSAGE_STATUS_QUERY:
		case ATARIX_MAILBOX_MESSAGE_READ_REGISTER:
		case ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST:  // computes a result, changes no state
			return ATARIX_OPERATION_READ;
		case ATARIX_MAILBOX_MESSAGE_WRITE_REGISTER:
		case ATARIX_MAILBOX_MESSAGE_DISCOVERY_RECORD_PUSH:
			return ATARIX_OPERATION_WRITE;
		case ATARIX_MAILBOX_MESSAGE_RESET_REQUEST:
			return ATARIX_OPERATION_RESET;
		default:
			return ATARIX_OPERATION_CONTROL;
	}
}

const Endpoint *Northbridge::find(uint16_t node) const {
	for(const auto &e: endpoints_) {
		if(e.node_id == node) return &e;
	}
	return nullptr;
}

void Northbridge::fault(uint64_t cycle, uint32_t address, bool write, std::string reason) {
	fault_ = true;
	faults_.push_back({cycle, address, write, std::move(reason)});
}

bool Northbridge::read(uint32_t address, uint8_t &value, uint64_t cycle) {
	if(address < MailboxBase || address > MailboxLimit) return false;

	if(address >= RxWindow && address < RxWindow + WindowSize) {
		value = rx_[address - RxWindow];
		return true;
	}
	if(address >= TxWindow && address < TxWindow + WindowSize) {
		value = tx_[address - TxWindow];
		return true;
	}

	switch(address - MailboxBase) {
		case RegStatus: value = uint8_t((rx_ready_ ? StatusRxReady : 0) | (fault_ ? StatusFault : 0)); break;
		case RegLength: value = uint8_t(rx_length_); break;
		case RegLengthHi: value = uint8_t(rx_length_ >> 8); break;
		case RegSource: value = uint8_t(rx_source_); break;
		case RegSourceHi: value = uint8_t(rx_source_ >> 8); break;
		case RegDestination: value = uint8_t(destination_); break;
		case RegDestinationHi: value = uint8_t(destination_ >> 8); break;
		case RegType: value = uint8_t(rx_type_); break;
		case RegTypeHi: value = uint8_t(rx_type_ >> 8); break;
		case RegResult: value = result_; break;
		case RegControl:
			value = 0;
			fault(cycle, address, false, "read of write-only mailbox CONTROL");
		break;
		default:
			value = 0;
			fault(cycle, address, false, "read of unimplemented mailbox register");
		break;
	}
	return true;
}

bool Northbridge::write(uint32_t address, uint8_t value, uint64_t cycle) {
	if(address < MailboxBase || address > MailboxLimit) return false;

	if(address >= TxWindow && address < TxWindow + WindowSize) {
		tx_[address - TxWindow] = value;
		return true;
	}
	if(address >= RxWindow && address < RxWindow + WindowSize) {
		fault(cycle, address, true, "write to read-only mailbox RX window");
		return true;
	}

	switch(address - MailboxBase) {
		case RegDestination: destination_ = uint16_t((destination_ & 0xFF00) | value); break;
		case RegDestinationHi: destination_ = uint16_t((destination_ & 0x00FF) | (value << 8)); break;
		case RegControl:
			if(value == ControlSubmit) {
				submit(cycle);
			} else if(value == ControlAckRx) {
				rx_ready_ = false;
				if(interrupts_) interrupts_->set_source(IrqMailbox, false);
			} else {
				fault(cycle, address, true, "unknown mailbox CONTROL command");
			}
		break;
		default:
			fault(cycle, address, true, "write to read-only or unimplemented mailbox register");
		break;
	}
	return true;
}

void Northbridge::submit(uint64_t cycle) {
	const uint8_t *frame = tx_.data();
	atarix_mailbox_header_v1_t header{};
	header.magic = le32(frame + 0);
	header.type = le16(frame + 4);
	header.length = le16(frame + 6);
	header.sequence = le32(frame + 8);
	header.crc32 = le32(frame + 12);

	AuditRecord record{cycle, cpu_node_, destination_, header.type, header.sequence, ATARIX_MAILBOX_STATUS_OK, ""};
	const auto finish = [&](atarix_mailbox_status_t status, const char *stage) {
		record.status = status;
		record.stage = stage;
		result_ = uint8_t(status);
		audit_.push_back(record);
	};

	// The frame window bounds the payload buffer. Passing the buffer size (not
	// the claimed length) lets libatarix apply the protocol's validation order:
	// magic, type, then "length matches payload buffer".
	const size_t payload_capacity = WindowSize - ATARIX_MAILBOX_HEADER_SIZE_V1;
	const size_t payload_length = header.length <= payload_capacity ? header.length : payload_capacity;

	const uint8_t *payload = frame + ATARIX_MAILBOX_HEADER_SIZE_V1;
	atarix_mailbox_status_t status = atarix_mailbox_validate_header(&header, payload, payload_length);
	if(injection_ == Injection::AcceptBadCrc && status == ATARIX_MAILBOX_STATUS_CRC_MISMATCH) {
		status = ATARIX_MAILBOX_STATUS_OK;
	}
	if(status != ATARIX_MAILBOX_STATUS_OK) {
		finish(status, "header");
		return;
	}

	status = atarix_mailbox_sequence_accept(&rx_window_, header.sequence);
	if(status != ATARIX_MAILBOX_STATUS_OK) {
		finish(status, "sequence");
		return;
	}

	const Endpoint *target = find(destination_);
	if(!target) {
		// A known endpoint is not authority, and an unknown one is denied by default.
		finish(ATARIX_MAILBOX_STATUS_POLICY_DENIED, "endpoint");
		return;
	}

	status = atarix_mailbox_ring_authorize(cpu_ring_, target->ring, header.type);
	if(injection_ == Injection::SkipRing && status == ATARIX_MAILBOX_STATUS_RING_DENIED) {
		status = ATARIX_MAILBOX_STATUS_OK;
	}
	if(status != ATARIX_MAILBOX_STATUS_OK) {
		finish(status, "ring");
		return;
	}

	atarix_capability_request_t request{};
	request.target_handle = handle_for(*target);
	request.operation_id = operation_for(header.type);
	request.requester_ring = cpu_ring_;
	request.trust_level = cpu_trust_;
	atarix_capability_evaluation_t evaluation{};
	const bool capability_ok =
		atarix_capability_evaluate(capabilities_.data(), capabilities_.size(), &request, &evaluation) == 0 &&
		evaluation.decision == ATARIX_DECISION_ALLOW;
	if(!capability_ok && injection_ != Injection::SkipCapability) {
		finish(ATARIX_MAILBOX_STATUS_CAPABILITY_DENIED, "capability");
		return;
	}

	if(target->echo_service && header.type == ATARIX_MAILBOX_MESSAGE_PING) {
		deliver_reply(target->node_id, ATARIX_MAILBOX_MESSAGE_PONG, header.sequence, payload, header.length);
		finish(ATARIX_MAILBOX_STATUS_OK, "dispatch");
		return;
	}

	if(target->gpu_compute && header.type == ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST) {
		// Service-level errors (bad length, operation, axis or range) are
		// reported in the reply's status; the mailbox decision is dispatch OK.
		const GpuFault fault = injection_ == Injection::GpuWrongMath ? GpuFault::NegateAngle
			: injection_ == Injection::GpuOffBy3 ? GpuFault::BiasX3 : GpuFault::None;
		const GpuReply reply = gpu_transform(payload, header.length, fault);
		gpu_log_.push_back({cycle, target->node_id, header.sequence, reply.status});
		deliver_reply(target->node_id, ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REPLY, header.sequence,
			reply.payload.data(), GpuReplyLength);
		finish(ATARIX_MAILBOX_STATUS_OK, "dispatch");
		return;
	}

	// Authorized but no handler: explicit, observable failure.
	finish(ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE, "dispatch");
}

void Northbridge::deliver_reply(uint16_t source, uint16_t type, uint32_t sequence, const uint8_t *payload, uint16_t length) {
	atarix_mailbox_header_v1_t reply{};
	if(atarix_mailbox_build_header(&reply, type, sequence, payload, length) != ATARIX_MAILBOX_STATUS_OK) {
		return;
	}
	rx_.fill(0);
	put32(rx_.data() + 0, reply.magic);
	put16(rx_.data() + 4, reply.type);
	put16(rx_.data() + 6, reply.length);
	put32(rx_.data() + 8, reply.sequence);
	put32(rx_.data() + 12, reply.crc32);
	for(uint16_t i = 0; i < length; ++i) rx_[ATARIX_MAILBOX_HEADER_SIZE_V1 + i] = payload[i];
	if(injection_ == Injection::CorruptPong && length) rx_[ATARIX_MAILBOX_HEADER_SIZE_V1] ^= 0x01;
	rx_source_ = source;
	rx_type_ = type;
	rx_length_ = length;
	rx_ready_ = true;
	if(interrupts_) interrupts_->set_source(IrqMailbox, true);
}

}  // namespace atarix::sim
