// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0

#include "supervisor.hpp"

#include <cstddef>
#include <utility>

extern "C" {
#include "atarix/operations.h"
#include "atarix/service_ids.h"
#include "atarix/sim_types.h"
}

namespace atarix::sim {

namespace {

std::string hex16(uint16_t v) {
	static constexpr char digits[] = "0123456789ABCDEF";
	std::string s = "0x";
	for(int shift = 12; shift >= 0; shift -= 4) s.push_back(digits[(v >> shift) & 0xF]);
	return s;
}

// Supervisor policy, bench profile: which provider trust a service type needs
// before the kernel is granted READ on it. Anything not listed gets nothing
// (default deny).
bool required_trust(uint16_t service_type, atarix_trust_level_t &trust) {
	switch(service_type) {
		case ATARIX_SERVICE_VALIDATION: trust = ATARIX_TRUST_IDENTIFIED; return true;
		case ATARIX_SERVICE_GPU_COMPUTE: trust = ATARIX_TRUST_VALIDATED; return true;
		default: return false;
	}
}

// Provider trust from a structurally valid identity: homebrew and unknown
// cards are identified, not validated (ADR-RING "Relationship to Homebrew Cards").
atarix_trust_level_t trust_from_identity(const atarix_identity_eeprom_v1_t &id) {
	if(atarix_identity_vendor_is_homebrew(id.vendor_id) || id.trust_class == ATARIX_TRUST_CLASS_HOMEBREW ||
		id.trust_class == ATARIX_TRUST_CLASS_UNKNOWN) {
		return ATARIX_TRUST_IDENTIFIED;
	}
	return ATARIX_TRUST_VALIDATED;
}

const char *trust_name(atarix_trust_level_t trust) {
	switch(trust) {
		case ATARIX_TRUST_IDENTIFIED: return "identified";
		case ATARIX_TRUST_VALIDATED: return "validated";
		case ATARIX_TRUST_TRUSTED: return "trusted";
		default: return "untrusted";
	}
}

}  // namespace

const char *gateway_result_name(GatewayResult result) {
	switch(result) {
		case GatewayResult::Executed: return "executed";
		case GatewayResult::Denied: return "denied";
		case GatewayResult::Deferred: return "deferred";
		case GatewayResult::RequiresApproval: return "requires-approval";
		case GatewayResult::Quarantined: return "quarantined";
		case GatewayResult::Unsupported: return "unsupported";
		case GatewayResult::Unknown: return "unknown";
		case GatewayResult::Failed: return "failed";
	}
	return "unknown";
}

atarix_identity_eeprom_v1_t make_identity(uint32_t vendor, uint32_t device, uint16_t board_class,
	uint16_t trust_class, const char *name) {
	atarix_identity_eeprom_v1_t id{};
	id.magic = ATARIX_IDENTITY_MAGIC_ATIX;
	id.format_version = ATARIX_IDENTITY_FORMAT_VERSION_V1;
	id.header_length = 0x80;
	id.total_record_length = ATARIX_IDENTITY_BLOCK_SIZE_V1;
	id.vendor_id = vendor;
	id.device_id = device;
	id.board_class = board_class;
	id.trust_class = trust_class;
	for(size_t i = 0; name[i] != '\0' && i + 1 < sizeof id.board_name; ++i) id.board_name[i] = name[i];
	id.crc32 = atarix_identity_crc32_v1(&id);
	return id;
}

Supervisor::Supervisor(InterruptController &interrupts, Northbridge &northbridge, uint16_t cpu_node,
	atarix_ring_t cpu_ring, std::array<uint8_t, 4> board_id)
	: interrupts_(interrupts), northbridge_(northbridge), cpu_node_(cpu_node), cpu_ring_(cpu_ring),
	  board_id_(board_id) {}

void Supervisor::record(uint64_t cycle, std::string event, uint16_t subject, std::string detail) {
	events_.push_back({cycle, std::move(event), subject, std::move(detail)});
}

void Supervisor::boot(const std::vector<Card> &cards) {
	record(0, "power-on", 0, "CPU card held in reset");

	// 1. Identity validation (identity-eeprom-v1 "Supervisor Responsibilities").
	struct Validated {
		const Card *card;
		bool valid;
		atarix_trust_level_t trust;
	};
	std::vector<Validated> validated;
	for(const auto &card: cards) {
		const uint32_t flags = atarix_identity_validate_v1(&card.identity);
		const bool valid = injection_ == SupervisorInjection::SkipIdentityValidation ||
			((flags & ATARIX_ID_STATUS_IDENTITY_VALID) && !(flags & ATARIX_ID_STATUS_ERROR));
		const atarix_trust_level_t trust = valid ? trust_from_identity(card.identity) : ATARIX_TRUST_UNTRUSTED;
		validated.push_back({&card, valid, trust});
		if(valid) {
			record(0, "identity-validated", card.node, card.name + ": " + trust_name(trust));
		} else {
			record(0, "identity-failed", card.node, card.name + ": card quarantined");
		}
	}

	if(injection_ == SupervisorInjection::ReleaseBeforeEnumeration) {
		record(0, "reset-released", 0, "CPU card released");
		return;
	}

	// 2. Enumeration (fabric, ring -1): one endpoint per declared service.
	// Describes only. A card that failed identity is listed as quarantined.
	for(const auto &v: validated) {
		Endpoint e{v.card->node, v.card->ring, v.card->service_type, v.card->echo_service, v.card->name,
			v.card->gpu_compute};
		e.state = v.valid ? ATARIX_MAILBOX_ENDPOINT_CREATED : ATARIX_MAILBOX_ENDPOINT_QUARANTINED;
		e.vendor = v.card->identity.vendor_id;
		e.device = v.card->identity.device_id;
		e.instance = v.card->instance;
		northbridge_.add_endpoint(e);
	}
	record(0, "enumeration-complete", 0, std::to_string(validated.size()) + " service endpoints");

	// 3. Capability evaluation: supervisor policy, not discovery, grants.
	uint32_t next_id = 1;
	for(const auto &v: validated) {
		const Card &card = *v.card;
		atarix_trust_level_t needed{};
		const bool listed = required_trust(card.service_type, needed);
		std::string why;
		if(injection_ != SupervisorInjection::GrantByDiscovery) {
			if(!v.valid) why = "identity failed";
			else if(!listed) why = "no policy for service type " + hex16(card.service_type);
			else if(v.trust < needed) why = std::string("provider is ") + trust_name(v.trust) + ", policy requires " +
				trust_name(needed);
		}
		if(!why.empty()) {
			record(0, "capability-withheld", card.node, why);
			continue;
		}
		atarix_capability_record_v1_t cap{};
		cap.capability_id = next_id++;
		cap.operation_id = ATARIX_OPERATION_READ;
		cap.target_handle = atarix_discovery_make_handle(card.node, card.service_type, 0);
		cap.minimum_ring = ATARIX_RING_WIRE_KERNEL;
		cap.maximum_ring = ATARIX_RING_WIRE_APPLICATION;
		cap.trust_requirement = ATARIX_TRUST_IDENTIFIED;
		cap.issuer_handle = atarix_discovery_make_handle(ATARIX_SIM_NODE_SUPERVISOR, ATARIX_SERVICE_PROVISIONING, 0);
		northbridge_.grant(cap);
		record(0, "capability-granted", card.node, "id " + std::to_string(cap.capability_id) + " READ to " +
			atarix_ring_name(cpu_ring_));
	}

	// 4. Service binding: publish the kernel's ring-filtered directory view.
	northbridge_.publish_directory();
	record(0, "service-binding", 0, "directory published for ring " + std::string(atarix_ring_name(cpu_ring_)));

	// 5. Transport activation: lifecycle CREATED -> ACTIVE, or SUSPENDED when
	// the card's service firmware is not ready. Quarantined stays quarantined.
	for(const auto &v: validated) {
		if(!v.valid) continue;
		northbridge_.set_state(v.card->node, v.card->firmware_ready ? ATARIX_MAILBOX_ENDPOINT_ACTIVE
		                                                           : ATARIX_MAILBOX_ENDPOINT_SUSPENDED);
	}
	record(0, "transport-activated", 0, "endpoints activated");

	record(0, "reset-released", 0, "CPU card released");
}

bool Supervisor::read(uint32_t address, uint8_t &value) {
	if(address < SupervisorBase || address > SupervisorLimit) return false;
	const uint32_t offset = address - SupervisorBase;
	if(offset == 0) {
		value = 0;  // COMMAND is write-only
	} else if(offset == 1) {
		value = status_;
		// Reading STATUS consumes the completion and deasserts the interrupt.
		status_ &= uint8_t(~0x02);
		interrupts_.set_source(IrqSupervisor, false);
	} else if(offset == 6) {
		value = uint8_t(result_);
	} else {
		value = args_[offset - 2];
	}
	return true;
}

GatewayResult Supervisor::evaluate(uint8_t command, uint16_t argument, uint64_t cycle, std::string &reason) {
	// Command table (ATX-SPEC-016 "Command Model"): caller class is the CPU
	// card at its slot's ring; no command is an open-ended interface.
	switch(command) {
		case CmdResetRequest:
			if(injection_ == SupervisorInjection::ExecutesReset) {
				reset_executed_ = true;
				reason = "executed (injected defect)";
				return GatewayResult::Executed;
			}
			reason = "system reset from the operational fabric requires operator approval";
			return GatewayResult::RequiresApproval;
		case CmdRecoveryMode:
			reason = "recovery entry is not available to the operational fabric";
			return GatewayResult::Denied;
		case CmdReadBoardId:
			args_ = board_id_;
			reason = "observation";
			return GatewayResult::Executed;
		case CmdReadFaultLog:
			reason = "fault log record format is not specified";
			return GatewayResult::Unsupported;
		case CmdFpgaReload:
			reason = "firmware and bitstream state are supervisor-only";
			return GatewayResult::Denied;
		case CmdReportFault: {
			const Endpoint *e = northbridge_.find(argument);
			if(!e) {
				reason = "no endpoint at " + hex16(argument);
				return GatewayResult::Denied;
			}
			if(e->ring <= cpu_ring_) {
				reason = std::string("a ") + atarix_ring_name(cpu_ring_) + " caller may not report a " +
					atarix_ring_name(e->ring) + "-ring endpoint";
				return GatewayResult::Denied;
			}
			northbridge_.set_state(argument, ATARIX_MAILBOX_ENDPOINT_QUARANTINED);
			record(cycle, "endpoint-quarantined", argument, "fault reported by " + hex16(cpu_node_));
			reason = "endpoint quarantined";
			return GatewayResult::Executed;
		}
		default:
			reason = "command not in the supervisor command table";
			return GatewayResult::Unknown;
	}
}

void Supervisor::complete(uint64_t cycle, uint8_t command, uint16_t argument, GatewayResult result, std::string reason) {
	result_ = result;
	status_ = uint8_t(0x02 | (result == GatewayResult::Executed ? 0 : 0x80));
	record(cycle, std::string("request-") + gateway_result_name(result), cpu_node_,
		"command " + hex16(command) + ": " + reason);
	requests_.push_back({cycle, command, cpu_node_, cpu_ring_, argument, result, std::move(reason)});
	// Every completion is signalled, whatever the result.
	interrupts_.set_source(IrqSupervisor, true);
}

bool Supervisor::write(uint32_t address, uint8_t value, uint64_t cycle) {
	if(address < SupervisorBase || address > SupervisorLimit) return false;
	const uint32_t offset = address - SupervisorBase;
	if(offset >= 2 && offset <= 5) {
		args_[offset - 2] = value;
		return true;
	}
	if(offset != 0) {
		record(cycle, "request-received", cpu_node_, "write to read-only gateway register");
		complete(cycle, 0xFF, 0, GatewayResult::Unknown, "write to read-only gateway register");
		return true;
	}
	// Only commands that define an argument take one; ARG0-ARG3 may still
	// hold an earlier result.
	const uint16_t argument = value == CmdReportFault ? uint16_t(args_[0] | (args_[1] << 8)) : 0;
	record(cycle, "request-received", cpu_node_, "command " + hex16(value));
	std::string reason;
	const GatewayResult result = evaluate(value, argument, cycle, reason);
	complete(cycle, value, argument, result, std::move(reason));
	return true;
}

}  // namespace atarix::sim
