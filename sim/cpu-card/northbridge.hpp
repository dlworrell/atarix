// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Fabric Northbridge model: the CPU-card-facing Mailbox Engine register block.
//
// The register block lives at $00D200 (docs/register-map-v1.md, "Mailbox
// Engine"). Offsets +00..+05 keep their documented names; the remaining
// offsets and the frame windows are the simulator profile proposed in
// docs/verification/cpu-card-software-testbench.md and are not yet a hardware
// contract.
//
// Every submitted frame is checked in the order required by
// docs/mailbox-protocol-v1.md using the production libatarix code:
//   header/CRC -> sequence -> endpoint -> ring -> capability -> dispatch.
// The source identity of a frame is assigned by the Northbridge from the slot
// the CPU card occupies; the CPU never supplies it.

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

extern "C" {
#include "atarix/capability_engine.h"
#include "atarix/mailbox.h"
#include "atarix/rings.h"
}

namespace atarix::sim {

constexpr uint32_t MailboxBase = 0x00D200;
constexpr uint32_t MailboxLimit = 0x00D2FF;
constexpr uint32_t TxWindow = 0x00D280;
constexpr uint32_t RxWindow = 0x00D2C0;
constexpr uint32_t WindowSize = 0x40;

// Register offsets from MailboxBase.
enum MailboxRegister: uint32_t {
	RegStatus = 0x00,         // R: bit1 RX ready, bit7 fault latched
	RegControl = 0x01,        // W: 0x01 submit TX frame, 0x02 acknowledge RX frame
	RegLength = 0x02,         // R: RX payload length, low byte
	RegSource = 0x03,         // R: RX source node, low byte
	RegDestination = 0x04,    // R/W: TX destination node, low byte
	RegType = 0x05,           // R: RX message type, low byte
	RegResult = 0x06,         // R: atarix_mailbox_status_t of the last submit
	RegDestinationHi = 0x07,  // R/W: TX destination node, high byte
	RegSourceHi = 0x08,       // R: RX source node, high byte
	RegTypeHi = 0x09,         // R: RX message type, high byte
	RegLengthHi = 0x0A,       // R: RX payload length, high byte
};

constexpr uint8_t ControlSubmit = 0x01;
constexpr uint8_t ControlAckRx = 0x02;
constexpr uint8_t StatusRxReady = 0x02;
constexpr uint8_t StatusFault = 0x80;

struct Endpoint {
	uint16_t node_id;
	atarix_ring_t ring;
	uint16_t service_type;
	bool echo_service;   // replies PONG to PING with the same sequence and payload
	std::string name;
};

struct AuditRecord {
	uint64_t cycle;
	uint16_t source;
	uint16_t destination;
	uint16_t type;
	uint32_t sequence;
	atarix_mailbox_status_t status;
	std::string stage;
};

struct BusFault {
	uint64_t cycle;
	uint32_t address;
	bool write;
	std::string reason;
};

// Deliberate defects for negative-control runs only: each must make the test
// bench fail, proving the bench can detect a broken security check.
enum class Injection {
	None,
	SkipCapability,
	SkipRing,
	AcceptBadCrc,
	CorruptPong,
};

class Northbridge {
public:
	Northbridge(uint16_t cpu_node, atarix_ring_t cpu_ring, atarix_trust_level_t cpu_trust);

	void inject(Injection injection) { injection_ = injection; }

	void add_endpoint(const Endpoint &endpoint);
	void grant(const atarix_capability_record_v1_t &capability);

	// Register-block access from the CPU card bus. Returns false if the address
	// is outside the block. Faults are recorded, never silent.
	bool read(uint32_t address, uint8_t &value, uint64_t cycle);
	bool write(uint32_t address, uint8_t value, uint64_t cycle);

	const std::vector<AuditRecord> &audit() const { return audit_; }
	const std::vector<BusFault> &faults() const { return faults_; }

	static atarix_discovery_handle_t handle_for(const Endpoint &endpoint);
	static uint32_t operation_for(uint16_t message_type);

private:
	uint16_t cpu_node_;
	atarix_ring_t cpu_ring_;
	atarix_trust_level_t cpu_trust_;
	std::vector<Endpoint> endpoints_;
	std::vector<atarix_capability_record_v1_t> capabilities_;
	atarix_mailbox_sequence_window_t rx_window_{};
	Injection injection_ = Injection::None;

	std::array<uint8_t, WindowSize> tx_{};
	std::array<uint8_t, WindowSize> rx_{};
	uint16_t destination_ = 0;
	uint16_t rx_source_ = 0;
	uint16_t rx_type_ = 0;
	uint16_t rx_length_ = 0;
	bool rx_ready_ = false;
	bool fault_ = false;
	uint8_t result_ = 0;

	std::vector<AuditRecord> audit_;
	std::vector<BusFault> faults_;

	void submit(uint64_t cycle);
	void deliver_reply(uint16_t source, uint16_t type, uint32_t sequence, const uint8_t *payload, uint16_t length);
	void fault(uint64_t cycle, uint32_t address, bool write, std::string reason);
	const Endpoint *find(uint16_t node) const;
};

}  // namespace atarix::sim
