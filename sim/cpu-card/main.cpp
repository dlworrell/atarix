// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// ATARIX CPU card software test bench.
//
// The supervisor first runs the boot authority chain for the simulated
// chassis (identity -> enumeration -> capability policy -> binding ->
// transport activation -> reset release, supervisor.hpp). Then the test ROM
// boots on the emulated CPU card, binds services by service type through
// Discovery Services, and runs its tests. The bench checks, independently of
// anything the firmware reports about itself:
//   1. the CPU reached STP (the firmware ran to completion);
//   2. the UART transcript ends with the firmware's PASS line;
//   3. the Northbridge audit log equals the expected decision sequence,
//      including the authority (capability) used for each allowed request;
//   4. the only bus faults are the ones the firmware provokes on purpose;
//   5. vector pulls; 6. supervisor events (boot chain and gateway requests)
//      and gateway results; 7. GPU service results.
//
// Usage: atarix-cpu-card --rom FILE [--trace FILE] [--report FILE] [--max-cycles N]
// Exit status: 0 pass, 1 fail, 2 usage or setup error.

#include <cstdio>
#include <exception>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cpu_card.hpp"

extern "C" {
#include "atarix/operations.h"
#include "atarix/service_ids.h"
#include "atarix/sim_types.h"
}

namespace {

using atarix::sim::AuditRecord;
using atarix::sim::BusFault;
using atarix::sim::CpuCard;
using atarix::sim::Endpoint;
using atarix::sim::Northbridge;

constexpr uint16_t CpuNode = ATARIX_SIM_NODE_CPU_BASE;          // 0x0100, slot 0
constexpr uint16_t EchoNode = ATARIX_SIM_NODE_TEST_BASE;        // 0xFF00, test card: echo service
constexpr uint16_t Echo2Node = ATARIX_SIM_NODE_TEST_BASE + 1;   // 0xFF01, echo service, firmware not ready
constexpr uint16_t UnknownNode = 0x0404;                        // no endpoint
constexpr uint16_t GpuNode = ATARIX_SIM_NODE_DISPLAY_BASE;      // 0x0300, display card: GPU compute
constexpr uint16_t GpuNode2 = ATARIX_SIM_NODE_DISPLAY_BASE + 1; // 0x0301, homebrew GPU: identified only
constexpr uint16_t StorageNode = ATARIX_SIM_NODE_STORAGE_BASE;  // 0x0200, storage card: identity fails
constexpr uint16_t SupervisorNode = ATARIX_SIM_NODE_SUPERVISOR;
constexpr uint16_t FabricNode = ATARIX_SIM_NODE_FABRIC_BASE;
constexpr const char *PassLine = "ATARIX CPU CARD TEST: PASS";

// Capability ids the supervisor's policy issues, in chassis order.
constexpr uint32_t CapGpu = 1;
constexpr uint32_t CapEcho = 2;
constexpr uint32_t CapEcho2 = 3;

struct Expected {
	uint16_t destination;
	uint16_t type;
	uint32_t sequence;
	atarix_mailbox_status_t status;
	uint32_t authority;   // capability id used; 0 when none was established
	const char *why;
};

constexpr uint16_t Ping = ATARIX_MAILBOX_MESSAGE_PING;
constexpr uint16_t GpuRequest = ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST;

// The firmware's mailbox test plan, as the Northbridge must decide it.
const Expected ExpectedAudit[] = {
	{EchoNode, Ping, 1, ATARIX_MAILBOX_STATUS_OK, CapEcho, "PING to granted echo service -> PONG"},
	{SupervisorNode, Ping, 2, ATARIX_MAILBOX_STATUS_RING_DENIED, 0, "kernel ring cannot request supervisor ring"},
	{FabricNode, Ping, 3, ATARIX_MAILBOX_STATUS_RING_DENIED, 0, "kernel ring cannot request fabric ring"},
	{EchoNode, ATARIX_MAILBOX_MESSAGE_WRITE_REGISTER, 4, ATARIX_MAILBOX_STATUS_CAPABILITY_DENIED, 0, "READ capability does not grant WRITE"},
	{EchoNode, Ping, 5, ATARIX_MAILBOX_STATUS_CRC_MISMATCH, 0, "corrupted payload CRC rejected before sequence"},
	{EchoNode, Ping, 5, ATARIX_MAILBOX_STATUS_OK, CapEcho, "same sequence accepted once CRC is valid"},
	{EchoNode, Ping, 5, ATARIX_MAILBOX_STATUS_DUPLICATE, 0, "replay of sequence 5 rejected"},
	{UnknownNode, Ping, 6, ATARIX_MAILBOX_STATUS_POLICY_DENIED, 0, "unknown endpoint denied by default"},
	{EchoNode, Ping, 8, ATARIX_MAILBOX_STATUS_SEQUENCE_ERROR, 0, "sequence gap rejected"},
	{EchoNode, 0x0099, 7, ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE, 0, "unknown message type rejected"},
	{EchoNode, Ping, 7, ATARIX_MAILBOX_STATUS_OK, CapEcho, "PING whose reply raises the mailbox interrupt (I2)"},
	{GpuNode, GpuRequest, 8, ATARIX_MAILBOX_STATUS_OK, CapGpu, "GPU transform G1"},
	{GpuNode, GpuRequest, 9, ATARIX_MAILBOX_STATUS_OK, CapGpu, "GPU transform G2"},
	{GpuNode, GpuRequest, 10, ATARIX_MAILBOX_STATUS_OK, CapGpu, "GPU bad operation (service-level error)"},
	{GpuNode, GpuRequest, 11, ATARIX_MAILBOX_STATUS_OK, CapGpu, "GPU bad length (service-level error)"},
	{GpuNode2, GpuRequest, 12, ATARIX_MAILBOX_STATUS_CAPABILITY_DENIED, 0, "homebrew GPU: policy withheld the capability"},
	{StorageNode, Ping, 13, ATARIX_MAILBOX_STATUS_CAPABILITY_DENIED, 0, "listed in the directory but quarantined at boot: lookup is not access"},
	{Echo2Node, Ping, 14, ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE, CapEcho2, "capability held, endpoint SUSPENDED"},
	{EchoNode, Ping, 15, ATARIX_MAILBOX_STATUS_OK, CapEcho, "reply left unacknowledged"},
	{EchoNode, Ping, 16, ATARIX_MAILBOX_STATUS_BUSY, CapEcho, "reply slot occupied: resource unavailable"},
	{GpuNode, GpuRequest, 17, ATARIX_MAILBOX_STATUS_QUARANTINED, CapGpu, "GPU quarantined by the supervisor after a fault report"},
};

// Supervisor audit (ATX-SPEC-016): the boot authority chain, then every
// gateway request as received and decided.
struct ExpectedEvent {
	const char *event;
	uint16_t subject;
};
const ExpectedEvent ExpectedEvents[] = {
	{"power-on", 0},
	{"identity-validated", GpuNode}, {"identity-validated", GpuNode2}, {"identity-failed", StorageNode},
	{"identity-validated", EchoNode}, {"identity-validated", Echo2Node},
	{"enumeration-complete", 0},
	{"capability-granted", GpuNode}, {"capability-withheld", GpuNode2}, {"capability-withheld", StorageNode},
	{"capability-granted", EchoNode}, {"capability-granted", Echo2Node},
	{"service-binding", 0},
	{"transport-activated", 0},
	{"reset-released", 0},
	{"request-received", CpuNode}, {"request-executed", CpuNode},           // I3 Read Board ID
	{"request-received", CpuNode}, {"request-requires-approval", CpuNode},  // I6 Reset Request
	{"request-received", CpuNode}, {"request-denied", CpuNode},             // Recovery Mode
	{"request-received", CpuNode}, {"request-unsupported", CpuNode},        // Read Fault Log
	{"request-received", CpuNode}, {"request-denied", CpuNode},             // FPGA Reload
	{"request-received", CpuNode}, {"request-unknown", CpuNode},            // command $7E
	{"request-received", CpuNode}, {"request-denied", CpuNode},             // Report Fault: supervisor node
	{"request-received", CpuNode}, {"endpoint-quarantined", GpuNode}, {"request-executed", CpuNode},  // Report Fault: GPU
};

// Gateway requests and the ATX-SPEC-016 result each must get.
struct ExpectedRequest {
	uint8_t command;
	uint16_t argument;
	atarix::sim::GatewayResult result;
};
using GR = atarix::sim::GatewayResult;
const ExpectedRequest ExpectedRequests[] = {
	{0x03, 0x0000, GR::Executed},          // Read Board ID
	{0x01, 0x0000, GR::RequiresApproval},  // Reset Request
	{0x02, 0x0000, GR::Denied},            // Recovery Mode
	{0x04, 0x0000, GR::Unsupported},       // Read Fault Log
	{0x05, 0x0000, GR::Denied},            // FPGA Reload
	{0x7E, 0x0000, GR::Unknown},           // not in the command table
	{0x06, SupervisorNode, GR::Denied},    // Report Fault on a more privileged ring
	{0x06, GpuNode, GR::Executed},         // Report Fault on the GPU: quarantined
};

// GPU service decisions (reply status) for the requests it received.
const uint32_t ExpectedGpuStatuses[] = {0, 0, 2, 1};

// Every vector fetch (VPB active) and its address after Vector Pull Rewrite:
// the reset vector at boot, then the supervisor IRQ at level 0 and the
// mailbox IRQ rewritten to level 2.
struct ExpectedPull {
	uint32_t original;
	uint32_t rewritten;
};
const ExpectedPull ExpectedPulls[] = {
	{0x00FFFC, 0x00FFFC}, {0x00FFFD, 0x00FFFD},
	{0x00FFEE, 0x00FFEE}, {0x00FFEF, 0x00FFEF},
	{0x00FFEE, 0x00FFF2}, {0x00FFEF, 0x00FFF3},
};

// Faults the firmware provokes deliberately (ROM write-protect check).
struct ExpectedFault {
	uint32_t address;
	bool write;
};
const ExpectedFault ExpectedFaults[] = {
	{0x00E000, true},
};

std::string json_escape(std::string_view s) {
	std::string out;
	for(const char c: s) {
		switch(c) {
			case '"': out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			default:
				if(static_cast<unsigned char>(c) < 0x20) {
					static constexpr char hex[] = "0123456789abcdef";
					const auto byte = static_cast<unsigned char>(c);
					out += "\\u00";
					out.push_back(hex[byte >> 4]);
					out.push_back(hex[byte & 0x0F]);
				} else {
					out.push_back(c);
				}
		}
	}
	return out;
}

int usage() {
	std::fprintf(stderr,
		"usage: atarix-cpu-card --rom FILE [--trace FILE] [--report FILE] [--max-cycles N]\n"
		"                       [--inject skip-capability|skip-ring|accept-bad-crc|corrupt-pong|\n"
		"                                 skip-lifecycle|skip-resource|list-platform-rings|\n"
		"                                 no-vector-rewrite|ignore-mask|mailbox-level7|supervisor-accepts-reset|\n"
		"                                 release-before-enumeration|grant-by-discovery|skip-identity-validation|\n"
		"                                 gpu-wrong-math|gpu-off-by-3]\n");
	return 2;
}

}  // namespace

int main(int argc, char **argv) {
	std::string rom_path, trace_path, report_path;
	uint64_t max_cycles = 20'000'000;
	atarix::sim::Injection injection = atarix::sim::Injection::None;
	atarix::sim::InterruptInjection interrupt_injection = atarix::sim::InterruptInjection::None;
	atarix::sim::SupervisorInjection supervisor_injection = atarix::sim::SupervisorInjection::None;
	for(int i = 1; i < argc; ++i) {
		const std::string_view arg(argv[i]);
		if(i + 1 >= argc) return usage();
		if(arg == "--rom") rom_path = argv[++i];
		else if(arg == "--trace") trace_path = argv[++i];
		else if(arg == "--report") report_path = argv[++i];
		else if(arg == "--max-cycles") max_cycles = std::stoull(argv[++i]);
		else if(arg == "--inject") {
			// Negative-control runs only; see sim/cpu-card/negative_control.sh.
			const std::string_view which(argv[++i]);
			using atarix::sim::Injection;
			if(which == "skip-capability") injection = Injection::SkipCapability;
			else if(which == "skip-ring") injection = Injection::SkipRing;
			else if(which == "accept-bad-crc") injection = Injection::AcceptBadCrc;
			else if(which == "corrupt-pong") injection = Injection::CorruptPong;
			else if(which == "gpu-wrong-math") injection = Injection::GpuWrongMath;
			else if(which == "gpu-off-by-3") injection = Injection::GpuOffBy3;
			else if(which == "skip-lifecycle") injection = Injection::SkipLifecycle;
			else if(which == "skip-resource") injection = Injection::SkipResource;
			else if(which == "list-platform-rings") injection = Injection::ListPlatformRings;
			else if(which == "supervisor-accepts-reset") supervisor_injection = atarix::sim::SupervisorInjection::ExecutesReset;
			else if(which == "release-before-enumeration") supervisor_injection = atarix::sim::SupervisorInjection::ReleaseBeforeEnumeration;
			else if(which == "grant-by-discovery") supervisor_injection = atarix::sim::SupervisorInjection::GrantByDiscovery;
			else if(which == "skip-identity-validation") supervisor_injection = atarix::sim::SupervisorInjection::SkipIdentityValidation;
			else if(which == "no-vector-rewrite") interrupt_injection = atarix::sim::InterruptInjection::NoVectorRewrite;
			else if(which == "ignore-mask") interrupt_injection = atarix::sim::InterruptInjection::IgnoreMask;
			else if(which == "mailbox-level7") interrupt_injection = atarix::sim::InterruptInjection::MailboxLevel7;
			else return usage();
		}
		else return usage();
	}
	if(rom_path.empty()) return usage();

	std::ifstream rom_file(rom_path, std::ios::binary);
	if(!rom_file) {
		std::fprintf(stderr, "cannot open ROM %s\n", rom_path.c_str());
		return 2;
	}
	std::vector<uint8_t> rom((std::istreambuf_iterator<char>(rom_file)), std::istreambuf_iterator<char>());

	// The CPU card runs the kernel (ring 0) in slot 0. Its identity and ring
	// are assigned by its slot, never claimed by the CPU.
	Northbridge northbridge(CpuNode, ATARIX_RING_KERNEL, ATARIX_TRUST_IDENTIFIED);
	northbridge.inject(injection);
	atarix::sim::InterruptController interrupts;
	interrupts.inject(interrupt_injection);
	northbridge.set_interrupts(&interrupts);
	const atarix::sim::BoardIdentity identity{};
	atarix::sim::Supervisor supervisor(interrupts, northbridge, CpuNode, ATARIX_RING_KERNEL, {
		uint8_t(identity.vendor), uint8_t(identity.vendor >> 8), uint8_t(identity.board), uint8_t(identity.board >> 8)});
	supervisor.inject(supervisor_injection);
	if(injection != atarix::sim::Injection::None || interrupt_injection != atarix::sim::InterruptInjection::None ||
		supervisor_injection != atarix::sim::SupervisorInjection::None) {
		std::printf("NEGATIVE CONTROL: deliberate defect injected\n");
	}

	// Platform endpoints (rings -2 and -1): part of the platform, not cards.
	Endpoint supervisor_endpoint{SupervisorNode, ATARIX_RING_SUPERVISOR, ATARIX_SERVICE_RECOVERY, false, "supervisor"};
	supervisor_endpoint.state = ATARIX_MAILBOX_ENDPOINT_ACTIVE;
	northbridge.add_endpoint(supervisor_endpoint);
	Endpoint fabric_endpoint{FabricNode, ATARIX_RING_FABRIC, ATARIX_SERVICE_MAILBOX, false, "fabric-control"};
	fabric_endpoint.state = ATARIX_MAILBOX_ENDPOINT_ACTIVE;
	northbridge.add_endpoint(fabric_endpoint);

	// The chassis: each card's Identity EEPROM and the service its discovery
	// ROM declares. Nothing here grants anything: the supervisor's policy
	// decides from validated identity (supervisor.cpp).
	using atarix::sim::Card;
	using atarix::sim::make_identity;
	constexpr uint32_t Vendor = 0x0000A7A1u;                         // simulator vendor id, not registered
	constexpr uint32_t Homebrew = ATARIX_VENDOR_HOMEBREW_BASE + 0x42u;
	std::vector<Card> cards;
	{
		// Display card, factory identity: GPU compute at the driver ring.
		Card c{"display-gpu", make_identity(Vendor, 0x2350, ATARIX_BOARD_CLASS_PROTOTYPE_CARD,
			ATARIX_TRUST_CLASS_FACTORY, "display-gpu"), GpuNode, ATARIX_SERVICE_GPU_COMPUTE, ATARIX_RING_DRIVER};
		c.gpu_compute = true;
		cards.push_back(c);
	}
	{
		// Homebrew display card: valid identity, but only "identified".
		Card c{"homebrew-gpu", make_identity(Homebrew, 0x2350, ATARIX_BOARD_CLASS_HOMEBREW_CARD,
			ATARIX_TRUST_CLASS_HOMEBREW, "homebrew-gpu"), GpuNode2, ATARIX_SERVICE_GPU_COMPUTE, ATARIX_RING_DRIVER, 1};
		c.gpu_compute = true;
		cards.push_back(c);
	}
	{
		// Storage card whose Identity EEPROM is corrupted (one bit, CRC stale).
		Card c{"storage", make_identity(Vendor, 0x0400, ATARIX_BOARD_CLASS_STORAGE_CARD,
			ATARIX_TRUST_CLASS_FACTORY, "storage"), StorageNode, ATARIX_SERVICE_BLOCK_STORAGE, ATARIX_RING_DRIVER};
		c.identity.board_name[0] ^= 0x01;
		cards.push_back(c);
	}
	{
		// Test card: echo validation service at the user-service ring.
		Card c{"test-echo", make_identity(Vendor, 0x0E00, ATARIX_BOARD_CLASS_DEVELOPMENT_CARD,
			ATARIX_TRUST_CLASS_DEVELOPMENT, "test-echo"), EchoNode, ATARIX_SERVICE_VALIDATION, ATARIX_RING_USER_SERVICE};
		c.echo_service = true;
		cards.push_back(c);
	}
	{
		// Second echo service whose firmware is not ready: enumerated and
		// granted, but left SUSPENDED by transport activation.
		Card c{"test-echo-2", make_identity(Vendor, 0x0E00, ATARIX_BOARD_CLASS_DEVELOPMENT_CARD,
			ATARIX_TRUST_CLASS_DEVELOPMENT, "test-echo-2"), Echo2Node, ATARIX_SERVICE_VALIDATION,
			ATARIX_RING_USER_SERVICE, 1};
		c.echo_service = true;
		c.firmware_ready = false;
		cards.push_back(c);
	}
	supervisor.boot(cards);

	FILE *trace = nullptr;
	if(!trace_path.empty()) {
		trace = std::fopen(trace_path.c_str(), "w");
		if(!trace) {
			std::fprintf(stderr, "cannot write trace %s\n", trace_path.c_str());
			return 2;
		}
	}

	uint64_t cycles = 0;
	std::vector<std::string> failures;
	std::string uart;
	std::vector<BusFault> faults;
	try {
		CpuCard card(std::move(rom), northbridge, interrupts, supervisor, identity);
		card.set_trace(trace);
		cycles = card.run(max_cycles);
		uart = card.uart();
		faults = card.faults();
		if(!card.stopped()) failures.push_back("CPU did not reach STP within the cycle limit");
	} catch(const std::exception &e) {
		std::fprintf(stderr, "error: %s\n", e.what());
		if(trace) std::fclose(trace);
		return 2;
	}
	if(trace) std::fclose(trace);

	// 2. UART transcript.
	if(uart.find(PassLine) == std::string::npos) failures.push_back("UART transcript has no PASS line");
	if(uart.find("FAIL") != std::string::npos) failures.push_back("firmware reported FAIL");

	// 3. Audit log must equal the expected decisions exactly, in order.
	const auto &audit = northbridge.audit();
	const size_t expected_count = sizeof ExpectedAudit / sizeof ExpectedAudit[0];
	if(audit.size() != expected_count) {
		failures.push_back("audit has " + std::to_string(audit.size()) + " decisions, expected " + std::to_string(expected_count));
	}
	for(size_t i = 0; i < audit.size() && i < expected_count; ++i) {
		const auto &got = audit[i];
		const auto &want = ExpectedAudit[i];
		if(got.destination != want.destination || got.type != want.type || got.sequence != want.sequence ||
			got.status != want.status || got.source != CpuNode || got.authority != want.authority ||
			got.actor_ring != ATARIX_RING_KERNEL || got.allowed != (want.authority != 0)) {
			std::ostringstream s;
			s << "audit[" << i << "] (" << want.why << "): got dst=" << got.destination << " type=" << got.type
				<< " seq=" << got.sequence << " status=" << got.status << " stage=" << got.stage
				<< " authority=" << got.authority << ", want dst=" << want.destination << " type=" << want.type
				<< " seq=" << want.sequence << " status=" << want.status << " authority=" << want.authority;
			failures.push_back(s.str());
		}
	}

	// 7. GPU service: exactly the expected requests reached it, with these statuses.
	const auto &gpu_log = northbridge.gpu_log();
	const size_t expected_gpu = sizeof ExpectedGpuStatuses / sizeof ExpectedGpuStatuses[0];
	if(gpu_log.size() != expected_gpu) {
		failures.push_back(std::to_string(gpu_log.size()) + " GPU requests served, expected " + std::to_string(expected_gpu));
	}
	for(size_t i = 0; i < gpu_log.size() && i < expected_gpu; ++i) {
		if(gpu_log[i].status != ExpectedGpuStatuses[i] || gpu_log[i].node != GpuNode) {
			failures.push_back("GPU request " + std::to_string(i) + " returned status " + std::to_string(gpu_log[i].status));
		}
	}

	// 5. Vector pulls: exact sequence, and never the RESET vector via rewrite.
	const auto &pulls = interrupts.vector_pulls();
	const size_t expected_pulls = sizeof ExpectedPulls / sizeof ExpectedPulls[0];
	if(pulls.size() != expected_pulls) {
		failures.push_back(std::to_string(pulls.size()) + " vector pulls, expected " + std::to_string(expected_pulls));
	}
	for(size_t i = 0; i < pulls.size(); ++i) {
		if(pulls[i].level >= 0 && pulls[i].rewritten >= 0x00FFFC) {
			failures.push_back("Vector Pull Rewrite redirected an IRQ to the RESET vector");
		}
		if(i < expected_pulls &&
			(pulls[i].original != ExpectedPulls[i].original || pulls[i].rewritten != ExpectedPulls[i].rewritten)) {
			std::ostringstream s;
			s << "vector pull " << i << ": " << std::hex << pulls[i].original << "->" << pulls[i].rewritten
				<< ", want " << ExpectedPulls[i].original << "->" << ExpectedPulls[i].rewritten;
			failures.push_back(s.str());
		}
	}

	// 6. Supervisor: boot chain and gateway events in order, then each
	// gateway request's result. A reset request must never execute.
	const auto &events = supervisor.events();
	const size_t expected_events = sizeof ExpectedEvents / sizeof ExpectedEvents[0];
	if(events.size() != expected_events) {
		failures.push_back(std::to_string(events.size()) + " supervisor events, expected " + std::to_string(expected_events));
	}
	for(size_t i = 0; i < events.size() && i < expected_events; ++i) {
		if(events[i].event != ExpectedEvents[i].event || events[i].subject != ExpectedEvents[i].subject) {
			failures.push_back("supervisor event " + std::to_string(i) + ": got " + events[i].event + " (" +
				events[i].detail + "), want " + ExpectedEvents[i].event);
			break;
		}
	}
	const auto &requests = supervisor.requests();
	const size_t expected_requests = sizeof ExpectedRequests / sizeof ExpectedRequests[0];
	if(requests.size() != expected_requests) {
		failures.push_back(std::to_string(requests.size()) + " gateway requests, expected " + std::to_string(expected_requests));
	}
	for(size_t i = 0; i < requests.size() && i < expected_requests; ++i) {
		const auto &got = requests[i];
		const auto &want = ExpectedRequests[i];
		if(got.command != want.command || got.argument != want.argument || got.result != want.result ||
			got.caller != CpuNode || got.caller_ring != ATARIX_RING_KERNEL) {
			failures.push_back("gateway request " + std::to_string(i) + " (command " + std::to_string(got.command) +
				"): result " + atarix::sim::gateway_result_name(got.result) + ", want " +
				atarix::sim::gateway_result_name(want.result));
		}
	}
	if(supervisor.reset_executed()) failures.push_back("CPU reset the system through the supervisor gateway");

	// 4. Faults: exactly the provoked ones, from both the card and the Northbridge.
	std::vector<BusFault> all_faults = faults;
	all_faults.insert(all_faults.end(), northbridge.faults().begin(), northbridge.faults().end());
	const size_t expected_faults = sizeof ExpectedFaults / sizeof ExpectedFaults[0];
	if(all_faults.size() != expected_faults) {
		failures.push_back(std::to_string(all_faults.size()) + " bus faults, expected " + std::to_string(expected_faults));
	}
	for(size_t i = 0; i < all_faults.size() && i < expected_faults; ++i) {
		if(all_faults[i].address != ExpectedFaults[i].address || all_faults[i].write != ExpectedFaults[i].write) {
			failures.push_back("unexpected fault at " + std::to_string(all_faults[i].address) + ": " + all_faults[i].reason);
		}
	}

	std::printf("%s", uart.c_str());
	std::printf("cycles=%llu audit=%zu faults=%zu result=%s\n",
		static_cast<unsigned long long>(cycles), audit.size(), all_faults.size(), failures.empty() ? "PASS" : "FAIL");
	for(const auto &f: failures) std::printf("  %s\n", f.c_str());

	if(!report_path.empty()) {
		std::ofstream out(report_path);
		out << "{\n  \"result\": \"" << (failures.empty() ? "pass" : "fail") << "\",\n"
			<< "  \"cycles\": " << cycles << ",\n"
			<< "  \"uart\": \"" << json_escape(uart) << "\",\n  \"audit\": [\n";
		for(size_t i = 0; i < audit.size(); ++i) {
			const auto &a = audit[i];
			out << "    {\"cycle\": " << a.cycle << ", \"source\": " << a.source << ", \"destination\": " << a.destination
				<< ", \"type\": " << a.type << ", \"sequence\": " << a.sequence << ", \"status\": " << a.status
				<< ", \"stage\": \"" << a.stage << "\", \"actor_ring\": \"" << atarix_ring_name(a.actor_ring)
				<< "\", \"target_ring\": \"" << atarix_ring_name(a.target_ring) << "\", \"operation\": " << a.operation
				<< ", \"authority\": " << a.authority << ", \"policy\": \"" << (a.allowed ? "allow" : "deny")
				<< "\", \"reason\": \"" << json_escape(a.reason) << "\"}" << (i + 1 < audit.size() ? ",\n" : "\n");
		}
		out << "  ],\n  \"vector_pulls\": [\n";
		for(size_t i = 0; i < pulls.size(); ++i) {
			const auto &v = pulls[i];
			out << "    {\"cycle\": " << v.cycle << ", \"original\": " << v.original << ", \"rewritten\": " << v.rewritten
				<< ", \"level\": " << v.level << ", \"emulation\": " << (v.emulation ? "true" : "false") << "}"
				<< (i + 1 < pulls.size() ? ",\n" : "\n");
		}
		out << "  ],\n  \"supervisor_events\": [\n";
		for(size_t i = 0; i < events.size(); ++i) {
			out << "    {\"cycle\": " << events[i].cycle << ", \"event\": \"" << events[i].event << "\", \"subject\": "
				<< events[i].subject << ", \"detail\": \"" << json_escape(events[i].detail) << "\"}"
				<< (i + 1 < events.size() ? ",\n" : "\n");
		}
		out << "  ],\n  \"gateway_requests\": [\n";
		for(size_t i = 0; i < requests.size(); ++i) {
			const auto &r = requests[i];
			out << "    {\"cycle\": " << r.cycle << ", \"command\": " << unsigned(r.command) << ", \"caller\": "
				<< r.caller << ", \"caller_ring\": \"" << atarix_ring_name(r.caller_ring) << "\", \"argument\": "
				<< r.argument << ", \"result\": \"" << atarix::sim::gateway_result_name(r.result)
				<< "\", \"reason\": \"" << json_escape(r.reason) << "\"}" << (i + 1 < requests.size() ? ",\n" : "\n");
		}
		out << "  ],\n  \"faults\": [\n";
		for(size_t i = 0; i < all_faults.size(); ++i) {
			const auto &f = all_faults[i];
			out << "    {\"cycle\": " << f.cycle << ", \"address\": " << f.address << ", \"write\": "
				<< (f.write ? "true" : "false") << ", \"reason\": \"" << json_escape(f.reason) << "\"}"
				<< (i + 1 < all_faults.size() ? ",\n" : "\n");
		}
		out << "  ],\n  \"failures\": [\n";
		for(size_t i = 0; i < failures.size(); ++i) {
			out << "    \"" << json_escape(failures[i]) << '"' << (i + 1 < failures.size() ? ",\n" : "\n");
		}
		out << "  ]\n}\n";
	}
	return failures.empty() ? 0 : 1;
}
