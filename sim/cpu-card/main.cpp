// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// ATARIX CPU card software test bench.
//
// Boots the test ROM on the emulated CPU card and checks, independently of
// anything the firmware reports about itself:
//   1. the CPU reached STP (the firmware ran to completion);
//   2. the UART transcript ends with the firmware's PASS line;
//   3. the Northbridge audit log equals the expected decision sequence;
//   4. the only bus faults are the ones the firmware provokes on purpose.
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
constexpr uint16_t EchoNode = ATARIX_SIM_NODE_TEST_BASE;        // 0xFF00, test range
constexpr uint16_t UnknownNode = 0x0404;                        // no endpoint
constexpr uint16_t GpuNode = ATARIX_SIM_NODE_DISPLAY_BASE;      // 0x0300, display card: GPU compute
constexpr uint16_t GpuNode2 = ATARIX_SIM_NODE_DISPLAY_BASE + 1; // 0x0301, second GPU: no capability
constexpr const char *PassLine = "ATARIX CPU CARD TEST: PASS";

struct Expected {
	uint16_t destination;
	uint16_t type;
	uint32_t sequence;
	atarix_mailbox_status_t status;
	const char *why;
};

// The firmware's mailbox test plan, as the Northbridge must decide it.
const Expected ExpectedAudit[] = {
	{EchoNode, ATARIX_MAILBOX_MESSAGE_PING, 1, ATARIX_MAILBOX_STATUS_OK, "PING to granted echo service -> PONG"},
	{ATARIX_SIM_NODE_SUPERVISOR, ATARIX_MAILBOX_MESSAGE_PING, 2, ATARIX_MAILBOX_STATUS_RING_DENIED, "kernel ring cannot request supervisor ring"},
	{ATARIX_SIM_NODE_FABRIC_BASE, ATARIX_MAILBOX_MESSAGE_PING, 3, ATARIX_MAILBOX_STATUS_RING_DENIED, "kernel ring cannot request fabric ring"},
	{EchoNode, ATARIX_MAILBOX_MESSAGE_WRITE_REGISTER, 4, ATARIX_MAILBOX_STATUS_CAPABILITY_DENIED, "READ capability does not grant WRITE"},
	{EchoNode, ATARIX_MAILBOX_MESSAGE_PING, 5, ATARIX_MAILBOX_STATUS_CRC_MISMATCH, "corrupted payload CRC rejected before sequence"},
	{EchoNode, ATARIX_MAILBOX_MESSAGE_PING, 5, ATARIX_MAILBOX_STATUS_OK, "same sequence accepted once CRC is valid"},
	{EchoNode, ATARIX_MAILBOX_MESSAGE_PING, 5, ATARIX_MAILBOX_STATUS_DUPLICATE, "replay of sequence 5 rejected"},
	{UnknownNode, ATARIX_MAILBOX_MESSAGE_PING, 6, ATARIX_MAILBOX_STATUS_POLICY_DENIED, "unknown endpoint denied by default"},
	{EchoNode, ATARIX_MAILBOX_MESSAGE_PING, 8, ATARIX_MAILBOX_STATUS_SEQUENCE_ERROR, "sequence gap rejected"},
	{EchoNode, 0x0099, 7, ATARIX_MAILBOX_STATUS_UNKNOWN_TYPE, "unknown message type rejected"},
	{EchoNode, ATARIX_MAILBOX_MESSAGE_PING, 7, ATARIX_MAILBOX_STATUS_OK, "PING whose reply raises the mailbox interrupt (I2)"},
	{GpuNode, ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST, 8, ATARIX_MAILBOX_STATUS_OK, "GPU transform G1"},
	{GpuNode, ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST, 9, ATARIX_MAILBOX_STATUS_OK, "GPU transform G2"},
	{GpuNode, ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST, 10, ATARIX_MAILBOX_STATUS_OK, "GPU bad operation (service-level error)"},
	{GpuNode, ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST, 11, ATARIX_MAILBOX_STATUS_OK, "GPU bad length (service-level error)"},
	{GpuNode2, ATARIX_MAILBOX_MESSAGE_GPU_TRANSFORM_REQUEST, 12, ATARIX_MAILBOX_STATUS_CAPABILITY_DENIED, "no capability for the second GPU"},
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

// Supervisor register-path commands: Read Board ID accepted, Reset rejected.
struct ExpectedCommand {
	uint8_t command;
	bool accepted;
};
const ExpectedCommand ExpectedCommands[] = {
	{0x03, true},
	{0x01, false},
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
		"                                 no-vector-rewrite|ignore-mask|mailbox-level7|supervisor-accepts-reset|\n"
		"                                 gpu-wrong-math|gpu-off-by-3]\n");
	return 2;
}

}  // namespace

int main(int argc, char **argv) {
	std::string rom_path, trace_path, report_path;
	uint64_t max_cycles = 20'000'000;
	atarix::sim::Injection injection = atarix::sim::Injection::None;
	atarix::sim::InterruptInjection interrupt_injection = atarix::sim::InterruptInjection::None;
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
			else if(which == "no-vector-rewrite") interrupt_injection = atarix::sim::InterruptInjection::NoVectorRewrite;
			else if(which == "ignore-mask") interrupt_injection = atarix::sim::InterruptInjection::IgnoreMask;
			else if(which == "mailbox-level7") interrupt_injection = atarix::sim::InterruptInjection::MailboxLevel7;
			else if(which == "supervisor-accepts-reset") interrupt_injection = atarix::sim::InterruptInjection::SupervisorAcceptsReset;
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

	// Topology: the CPU card (kernel ring) in slot 0, the supervisor and fabric
	// control endpoints, and one test echo service in the test node range.
	Northbridge northbridge(CpuNode, ATARIX_RING_KERNEL, ATARIX_TRUST_IDENTIFIED);
	northbridge.inject(injection);
	atarix::sim::InterruptController interrupts;
	interrupts.inject(interrupt_injection);
	northbridge.set_interrupts(&interrupts);
	const atarix::sim::BoardIdentity identity{};
	atarix::sim::SupervisorMailbox supervisor(interrupts, {
		uint8_t(identity.vendor), uint8_t(identity.vendor >> 8), uint8_t(identity.board), uint8_t(identity.board >> 8)});
	supervisor.inject(interrupt_injection);
	if(injection != atarix::sim::Injection::None || interrupt_injection != atarix::sim::InterruptInjection::None) {
		std::printf("NEGATIVE CONTROL: deliberate Northbridge defect injected\n");
	}
	northbridge.add_endpoint({ATARIX_SIM_NODE_SUPERVISOR, ATARIX_RING_SUPERVISOR, ATARIX_SERVICE_RECOVERY, false, "supervisor"});
	northbridge.add_endpoint({ATARIX_SIM_NODE_FABRIC_BASE, ATARIX_RING_FABRIC, ATARIX_SERVICE_MAILBOX, false, "fabric-control"});
	const Endpoint echo{EchoNode, ATARIX_RING_SERVICE, ATARIX_SERVICE_VALIDATION, true, "test-echo"};
	northbridge.add_endpoint(echo);

	// Display cards offering SERVICE_GPU_COMPUTE (cglm-backed, gpu_service.hpp).
	const Endpoint gpu{GpuNode, ATARIX_RING_SERVICE, ATARIX_SERVICE_GPU_COMPUTE, false, "gpu-compute", true};
	northbridge.add_endpoint(gpu);
	northbridge.add_endpoint({GpuNode2, ATARIX_RING_SERVICE, ATARIX_SERVICE_GPU_COMPUTE, false, "gpu-compute-2", true});

	// The CPU card's capabilities: READ on the echo service and on the first GPU.
	atarix_capability_record_v1_t read_echo{};
	read_echo.capability_id = 1;
	read_echo.operation_id = ATARIX_OPERATION_READ;
	read_echo.target_handle = Northbridge::handle_for(echo);
	read_echo.minimum_ring = ATARIX_RING_WIRE_KERNEL;
	read_echo.maximum_ring = ATARIX_RING_WIRE_APPLICATION;
	read_echo.trust_requirement = ATARIX_TRUST_IDENTIFIED;
	read_echo.issuer_handle = atarix_discovery_make_handle(ATARIX_SIM_NODE_SUPERVISOR, ATARIX_SERVICE_PROVISIONING, 0);
	northbridge.grant(read_echo);
	atarix_capability_record_v1_t read_gpu = read_echo;
	read_gpu.capability_id = 2;
	read_gpu.target_handle = Northbridge::handle_for(gpu);
	northbridge.grant(read_gpu);

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
		if(got.destination != want.destination || got.type != want.type ||
			got.sequence != want.sequence || got.status != want.status || got.source != CpuNode) {
			std::ostringstream s;
			s << "audit[" << i << "] (" << want.why << "): got dst=" << got.destination << " type=" << got.type
				<< " seq=" << got.sequence << " status=" << got.status << " stage=" << got.stage
				<< ", want dst=" << want.destination << " type=" << want.type << " seq=" << want.sequence
				<< " status=" << want.status;
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

	// 6. Supervisor register path: only Read Board ID may succeed.
	const auto &commands = supervisor.commands();
	const size_t expected_commands = sizeof ExpectedCommands / sizeof ExpectedCommands[0];
	if(commands.size() != expected_commands) {
		failures.push_back(std::to_string(commands.size()) + " supervisor commands, expected " + std::to_string(expected_commands));
	}
	for(size_t i = 0; i < commands.size() && i < expected_commands; ++i) {
		if(commands[i].command != ExpectedCommands[i].command || commands[i].accepted != ExpectedCommands[i].accepted) {
			failures.push_back("supervisor command " + std::to_string(i) + " decided unexpectedly");
		}
	}
	if(supervisor.reset_requested()) failures.push_back("CPU reset the system through the supervisor register path");

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
				<< ", \"stage\": \"" << a.stage << "\"}" << (i + 1 < audit.size() ? ",\n" : "\n");
		}
		out << "  ],\n  \"vector_pulls\": [\n";
		for(size_t i = 0; i < pulls.size(); ++i) {
			const auto &v = pulls[i];
			out << "    {\"cycle\": " << v.cycle << ", \"original\": " << v.original << ", \"rewritten\": " << v.rewritten
				<< ", \"level\": " << v.level << ", \"emulation\": " << (v.emulation ? "true" : "false") << "}"
				<< (i + 1 < pulls.size() ? ",\n" : "\n");
		}
		out << "  ],\n  \"supervisor_commands\": [\n";
		for(size_t i = 0; i < commands.size(); ++i) {
			out << "    {\"cycle\": " << commands[i].cycle << ", \"command\": " << unsigned(commands[i].command)
				<< ", \"accepted\": " << (commands[i].accepted ? "true" : "false") << "}"
				<< (i + 1 < commands.size() ? ",\n" : "\n");
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
