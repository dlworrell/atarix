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
// Every submitted frame is checked in the ATX-SPEC-005 validation order using
// the production libatarix code:
//   header/CRC -> sequence -> endpoint -> ring -> capability ->
//   lifecycle state -> resource availability -> dispatch.
// The source identity of a frame is assigned by the Northbridge from the slot
// the CPU card occupies; the CPU never supplies it.
//
// The Northbridge is also the Enumeration Fabric (ring -1) for the bench: it
// holds the endpoints the cards' discovery records describe and publishes the
// kernel's ring-filtered view of them at Discovery Services ($00D500).
// Enumeration describes; it grants nothing. Capabilities come only from the
// supervisor's policy (supervisor.hpp).

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "interrupts.hpp"

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

// Discovery Services ($00D500, docs/register-map-v1.md). +00..+06 keep the
// documented names; +06 and +07..+0D are the simulator profile (see the
// test-bench doc, spec gap register).
constexpr uint32_t DiscoveryBase = 0x00D500;
constexpr uint32_t DiscoveryLimit = 0x00D50D;
enum DiscoveryRegister: uint32_t {
	DiscCount = 0x00,        // R: entries in the caller's ring-filtered view
	DiscSelect = 0x01,       // R/W: entry index
	DiscVendorLo = 0x02,     // R: provider card vendor (low 16 bits of the identity vendor)
	DiscVendorHi = 0x03,
	DiscDeviceLo = 0x04,     // R: provider card device id (low 16 bits)
	DiscDeviceHi = 0x05,
	DiscCapability = 0x06,   // R: capability id granted to the caller for this entry, 0 = none
	DiscRing = 0x07,         // R: provider ring, wire value low byte
	DiscServiceLo = 0x08,    // R: service type
	DiscServiceHi = 0x09,
	DiscNodeLo = 0x0A,       // R: node id (transport address; not identity, not authority)
	DiscNodeHi = 0x0B,
	DiscState = 0x0C,        // R: endpoint lifecycle state (atarix_mailbox_endpoint_state_t)
	DiscInstance = 0x0D,     // R: service instance
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
	bool gpu_compute = false;  // SERVICE_GPU_COMPUTE: answers GPU_TRANSFORM_REQUEST (gpu_service.hpp)
	atarix_mailbox_endpoint_state_t state = ATARIX_MAILBOX_ENDPOINT_CREATED;
	uint32_t vendor = 0;       // from the provider card's validated identity
	uint32_t device = 0;
	uint8_t instance = 0;
};

struct GpuServiceRecord {
	uint64_t cycle;
	uint16_t node;
	uint32_t sequence;
	uint32_t status;     // GpuStatus returned in the reply
};

// One mailbox decision, with the ATX-SPEC-010 audit fields the bench can
// supply: actor (source node, assigned by the fabric), actor ring, operation,
// target and its ring, authority used (capability id, 0 = none), policy
// result, result, reason, and provenance (cycle, deciding stage).
struct AuditRecord {
	uint64_t cycle;
	uint16_t source;
	uint16_t destination;
	uint16_t type;
	uint32_t sequence;
	atarix_mailbox_status_t status;
	std::string stage;
	atarix_ring_t actor_ring = ATARIX_RING_QUARANTINE;
	atarix_ring_t target_ring = ATARIX_RING_QUARANTINE;  // QUARANTINE when there is no target
	uint32_t operation = 0;
	uint32_t authority = 0;
	bool allowed = false;   // policy result: ring and capability both allowed
	std::string reason;
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
	SkipLifecycle,       // deliver to endpoints that are not ACTIVE
	SkipResource,        // overwrite an unacknowledged reply
	ListPlatformRings,   // publish supervisor/fabric endpoints in the kernel's view
	GpuWrongMath,        // GPU service rotates the wrong way
	GpuOffBy3,           // GPU result 3 LSB off (outside the +/-2 tolerance)
};

class Northbridge {
public:
	Northbridge(uint16_t cpu_node, atarix_ring_t cpu_ring, atarix_trust_level_t cpu_trust);

	void inject(Injection injection) { injection_ = injection; }

	// RX-ready drives interrupt source 3 (Mailbox), level-triggered.
	void set_interrupts(InterruptController *interrupts) { interrupts_ = interrupts; }

	void add_endpoint(const Endpoint &endpoint);
	void grant(const atarix_capability_record_v1_t &capability);

	// Lifecycle control. Only the supervisor calls these (boot activation,
	// quarantine); there is no CPU-visible path to them.
	bool set_state(uint16_t node, atarix_mailbox_endpoint_state_t state);
	// Publish the ring-filtered directory view for the CPU card's ring:
	// endpoints in rings at or below it in privilege are not listed.
	void publish_directory();

	const std::vector<Endpoint> &endpoints() const { return endpoints_; }
	const std::vector<atarix_capability_record_v1_t> &capabilities() const { return capabilities_; }
	const Endpoint *find(uint16_t node) const;
	// Capability id granted to the CPU card for this endpoint (READ), or 0.
	uint32_t granted_capability(const Endpoint &endpoint) const;

	// Register-block access from the CPU card bus. Returns false if the address
	// is outside the block. Faults are recorded, never silent.
	bool read(uint32_t address, uint8_t &value, uint64_t cycle);
	bool write(uint32_t address, uint8_t value, uint64_t cycle);

	const std::vector<AuditRecord> &audit() const { return audit_; }
	const std::vector<BusFault> &faults() const { return faults_; }
	const std::vector<GpuServiceRecord> &gpu_log() const { return gpu_log_; }

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
	InterruptController *interrupts_ = nullptr;

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
	std::vector<GpuServiceRecord> gpu_log_;

	std::vector<uint16_t> directory_;   // node ids in the published view
	uint8_t select_ = 0;

	void submit(uint64_t cycle);
	void deliver_reply(uint16_t source, uint16_t type, uint32_t sequence, const uint8_t *payload, uint16_t length);
	void fault(uint64_t cycle, uint32_t address, bool write, std::string reason);
	bool read_discovery(uint32_t offset, uint8_t &value, uint64_t cycle);
	bool write_discovery(uint32_t offset, uint8_t value, uint64_t cycle);
};

}  // namespace atarix::sim
