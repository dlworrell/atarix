// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Supervisor model: ring -2 (docs/ADR-RING-SECURITY-MODEL.md), the RP2350
// supervisor of ATX-SPEC-016.
//
// Two jobs in the bench:
//
// 1. Boot authority chain (ADR-RING-SECURITY-MODEL "Boot-Time Authority
//    Chain", ADR-THREE-FABRIC-ARCHITECTURE "Boot Sequence"), run while the
//    CPU card is held in reset:
//      power-on -> identity validation (each card's Identity EEPROM v1) ->
//      enumeration (fabric, ring -1: endpoints from discovery records) ->
//      capability evaluation (supervisor policy) -> service binding (ring-
//      filtered directory published at $00D500) -> transport activation
//      (endpoint lifecycle) -> CPU reset released.
//    Every step is a supervisor audit event.
//
// 2. Supervisor Request Gateway (ATX-SPEC-016 "Operational Requests"): the
//    register block at $00C200 is the CPU's only path to the supervisor. A
//    request is not a command: each one is checked against the command
//    policy below and answered with an ATX-SPEC-016 result code.
//      +00 COMMAND   W   writing submits the request with ARG0-ARG3
//      +01 STATUS    R   bit1 done, bit7 not executed; reading clears done
//                        and deasserts interrupt source 1 (Supervisor)
//      +02..+05 ARG  R/W request arguments in, results out
//      +06 RESULT    R   GatewayResult of the last request
//    The caller's identity and ring come from the slot, never from the CPU.

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "interrupts.hpp"
#include "northbridge.hpp"

extern "C" {
#include "atarix/capability.h"
#include "atarix/identity_eeprom.h"
#include "atarix/rings.h"
}

namespace atarix::sim {

constexpr uint32_t SupervisorBase = 0x00C200;
constexpr uint32_t SupervisorLimit = 0x00C206;

// ATX-SPEC-016 "Request Results". Values are the simulator profile.
enum class GatewayResult: uint8_t {
	Executed = 0,
	Denied = 1,
	Deferred = 2,
	RequiresApproval = 3,
	Quarantined = 4,
	Unsupported = 5,
	Unknown = 6,
	Failed = 7,
};
const char *gateway_result_name(GatewayResult result);

// Command IDs: 01-05 from docs/register-map-v1.md; 06 is proposed.
enum GatewayCommand: uint8_t {
	CmdResetRequest = 0x01,
	CmdRecoveryMode = 0x02,
	CmdReadBoardId = 0x03,
	CmdReadFaultLog = 0x04,
	CmdFpgaReload = 0x05,
	CmdReportFault = 0x06,   // ARG0/ARG1 = node the kernel reports as faulty
};

enum class SupervisorInjection {
	None,
	ExecutesReset,             // gateway executes a CPU reset request
	ReleaseBeforeEnumeration,  // CPU leaves reset before enumeration and binding
	GrantByDiscovery,          // every enumerated service gets a capability
	SkipIdentityValidation,    // cards are trusted without checking identity
};

// A card as the supervisor finds it on the backplane: its Identity EEPROM
// and the one service its discovery ROM declares.
struct Card {
	std::string name;
	atarix_identity_eeprom_v1_t identity;
	uint16_t node;
	uint16_t service_type;
	atarix_ring_t ring;
	uint8_t instance = 0;
	bool firmware_ready = true;  // false: enumerated but not activated (SUSPENDED)
	bool echo_service = false;
	bool gpu_compute = false;
};

// Identity EEPROM v1 record with a correct CRC.
atarix_identity_eeprom_v1_t make_identity(uint32_t vendor, uint32_t device, uint16_t board_class,
	uint16_t trust_class, const char *name);

struct SupervisorEvent {
	uint64_t cycle;
	std::string event;
	uint16_t subject;     // node the event is about (0 = the platform)
	std::string detail;
};

struct GatewayRequest {
	uint64_t cycle;
	uint8_t command;
	uint16_t caller;          // assigned from the slot
	atarix_ring_t caller_ring;
	uint16_t argument;        // ARG1:ARG0 at submission
	GatewayResult result;
	std::string reason;
};

class Supervisor {
public:
	Supervisor(InterruptController &interrupts, Northbridge &northbridge, uint16_t cpu_node,
		atarix_ring_t cpu_ring, std::array<uint8_t, 4> board_id);

	void inject(SupervisorInjection injection) { injection_ = injection; }

	// Run the boot authority chain for these cards. Afterwards the CPU card
	// may leave reset (the bench then runs it).
	void boot(const std::vector<Card> &cards);

	bool read(uint32_t address, uint8_t &value);
	bool write(uint32_t address, uint8_t value, uint64_t cycle);

	const std::vector<SupervisorEvent> &events() const { return events_; }
	const std::vector<GatewayRequest> &requests() const { return requests_; }
	bool reset_executed() const { return reset_executed_; }

private:
	InterruptController &interrupts_;
	Northbridge &northbridge_;
	uint16_t cpu_node_;
	atarix_ring_t cpu_ring_;
	std::array<uint8_t, 4> board_id_;
	std::array<uint8_t, 4> args_{};
	uint8_t status_ = 0;
	GatewayResult result_ = GatewayResult::Unknown;
	bool reset_executed_ = false;
	SupervisorInjection injection_ = SupervisorInjection::None;
	std::vector<SupervisorEvent> events_;
	std::vector<GatewayRequest> requests_;

	void record(uint64_t cycle, std::string event, uint16_t subject, std::string detail);
	GatewayResult evaluate(uint8_t command, uint16_t argument, uint64_t cycle, std::string &reason);
	void complete(uint64_t cycle, uint8_t command, uint16_t argument, GatewayResult result, std::string reason);
};

}  // namespace atarix::sim
