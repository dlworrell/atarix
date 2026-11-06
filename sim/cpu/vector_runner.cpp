// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// ATARIX CPU vector runner.
//
// Replays SingleStepTests-format single-instruction vectors against the
// vendored CLK CPU cores and compares, per test:
//   * final registers,
//   * final RAM at every address the vector lists, and
//   * every bus cycle: address, data, and (65816) VDA/VPA/VPB/RWB/E/M/X/ML.
//
// Test-setup and stop logic follow the upstream CLK comparative harness
// (third_party/clk, MIT): state is loaded with emulation flag before width-
// dependent registers, the run stops at the next opcode fetch, and MVN/MVP may
// refetch their own opcode.
//
// Usage:
//   atarix-cpu-vectors --cpu <65816|wdc65c02|rockwell65c02|synertek65c02|6502|nes6502>
//                      [--max-tests N] [--known FILE] [--report FILE] [--verbose] FILE...
// Exit status: 0 all compared tests passed (after known deviations),
//              1 at least one unexpected failure, 2 usage or input error.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Processors/6502Esque/6502Selector.hpp"
#include "json_lite.hpp"

namespace {

using CPU::MOS6502Esque::BusOperation;
using CPU::MOS6502Esque::Register;
using CPU::MOS6502Esque::Type;

struct StopException {};

// 65816 MVN/MVP may copy up to 65536 bytes at 7 cycles each.
constexpr int MaxTestCycles = 65537 * 7;

struct RecordedCycle {
	BusOperation operation = BusOperation::None;
	std::optional<uint32_t> address;
	std::optional<uint8_t> value;
	int extended_bus = 0;
};

template <Type type>
struct VectorBus: public CPU::MOS6502Esque::BusHandlerT<type> {
	using AddressType = typename CPU::MOS6502Esque::BusHandlerT<type>::AddressType;

	std::unordered_map<uint32_t, uint8_t> ram;
	std::vector<RecordedCycle> cycles;
	std::optional<uint32_t> initial_pc;
	bool allow_pc_repetition = false;
	int pc_overshoot = 0;
	int jam_count = 0;
	uint32_t unlisted_reads = 0;

	CPU::MOS6502Esque::Processor<type, VectorBus<type>, false> processor;

	VectorBus() : processor(*this) {
		processor.set_power_on(false);
	}

	Cycles perform_bus_operation(const BusOperation operation, const AddressType address, uint8_t *const value) {
		if(jam_count) {
			if(!--jam_count) throw StopException();
		} else if(processor.is_jammed()) {
			jam_count = 5;
		}

		auto &cycle = cycles.emplace_back();
		cycle.operation = operation;
		if constexpr (CPU::MOS6502Esque::has_extended_bus_output(type)) {
			cycle.extended_bus = processor.get_extended_bus_output();
		}

		switch(operation) {
			case BusOperation::ReadOpcode:
				if(initial_pc && (*initial_pc != address || !allow_pc_repetition)) {
					cycles.pop_back();
					pc_overshoot = -1;
					throw StopException();
				}
				initial_pc = address;
				[[fallthrough]];

			case BusOperation::Read:
			case BusOperation::ReadProgram:
			case BusOperation::ReadVector: {
				cycle.address = address;
				const auto found = ram.find(address);
				uint8_t data = 0;
				if(found != ram.end()) {
					data = found->second;
				} else {
					++unlisted_reads;
					ram[address] = 0;
				}
				*value = data;
				cycle.value = data;
			} break;

			case BusOperation::Write:
				cycle.address = address;
				ram[address] = *value;
				cycle.value = *value;
			break;

			case BusOperation::Ready:
			case BusOperation::None:
				throw StopException();

			case BusOperation::InternalOperationWrite: {
				const auto found = ram.find(address);
				const uint8_t data = found != ram.end() ? found->second : 0;
				*value = data;
				cycle.value = data;
			} [[fallthrough]];

			case BusOperation::InternalOperationRead:
				cycle.address = address;
			break;

			default:
				throw StopException();
		}

		return Cycles(1);
	}

	void reset() {
		ram.clear();
		cycles.clear();
		initial_pc.reset();
		allow_pc_repetition = false;
		pc_overshoot = 0;
		jam_count = 0;
		unlisted_reads = 0;
		processor.restart_operation_fetch();
	}
};

constexpr bool is_65816(Type type) {
	return type == Type::TWDC65816;
}

// Bus-line text as used by the 65816 vector set: "dpvr" + "emxl", '-' when inactive.
std::string lines_text(const RecordedCycle &cycle) {
	bool vda = false, vpa = false, vpb = false, read = false, wait = false;
	switch(cycle.operation) {
		case BusOperation::Read: read = vda = true; break;
		case BusOperation::ReadOpcode: read = vda = vpa = true; break;
		case BusOperation::ReadProgram: read = vpa = true; break;
		case BusOperation::ReadVector: read = vpb = vda = true; break;
		case BusOperation::InternalOperationRead: read = true; break;
		case BusOperation::Write: vda = true; break;
		case BusOperation::InternalOperationWrite: break;
		default: wait = true; break;
	}
	if(wait) return "--------";
	using Ext = CPU::WDC65816::ExtendedBusOutput;
	std::string s(8, '-');
	if(vda) s[0] = 'd';
	if(vpa) s[1] = 'p';
	if(vpb) s[2] = 'v';
	s[3] = read ? 'r' : 'w';
	if(cycle.extended_bus & Ext::Emulation) s[4] = 'e';
	if(cycle.extended_bus & Ext::MemorySize) s[5] = 'm';
	if(cycle.extended_bus & Ext::IndexSize) s[6] = 'x';
	if(cycle.extended_bus & Ext::MemoryLock) s[7] = 'l';
	return s;
}

struct FileResult {
	std::string file;
	size_t tests = 0;
	size_t passed = 0;
	size_t register_failures = 0;
	size_t ram_failures = 0;
	size_t cycle_failures = 0;
	size_t unlisted_read_tests = 0;
	size_t known_deviation = 0;
	bool empty_file = false;
	std::vector<std::string> first_failures;
};

struct Options {
	std::string cpu;
	size_t max_tests = 0;
	bool verbose = false;
	std::string known_file;
	std::string report_file;
	std::vector<std::string> files;
};

std::string basename_of(const std::string &path) {
	const auto slash = path.find_last_of('/');
	return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::set<std::string> load_known(const std::string &path) {
	std::set<std::string> known;
	if(path.empty()) return known;
	std::ifstream in(path);
	if(!in) throw std::runtime_error("cannot open known-deviation file " + path);
	std::string line;
	while(std::getline(in, line)) {
		const auto hash = line.find('#');
		if(hash != std::string::npos) line.resize(hash);
		while(!line.empty() && (line.back() == ' ' || line.back() == '\t' || line.back() == '\r')) line.pop_back();
		size_t start = 0;
		while(start < line.size() && (line[start] == ' ' || line[start] == '\t')) ++start;
		if(start < line.size()) known.insert(line.substr(start));
	}
	return known;
}

template <Type type>
FileResult run_file(const std::string &path, const Options &options, const std::set<std::string> &known) {
	FileResult result;
	result.file = basename_of(path);

	std::ifstream in(path, std::ios::binary);
	if(!in) throw std::runtime_error("cannot open " + path);
	const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	if(text.empty()) {
		// Upstream ships some opcodes (e.g. WDC 65C02 WAI/STP) as empty files.
		// Report them as uncovered rather than as passes or input errors.
		result.empty_file = true;
		return result;
	}
	const auto tests = atarix::json::parse(text);
	if(tests.kind != atarix::json::Value::Kind::Array) throw std::runtime_error(path + ": top level is not an array");

	VectorBus<type> bus;
	for(const auto &test: tests.array) {
		if(options.max_tests && result.tests >= options.max_tests) break;
		++result.tests;

		const std::string &name = test.at("name").text();
		const auto &initial = test.at("initial");
		const auto &final_state = test.at("final");

		bus.reset();
		auto &cpu = bus.processor;
		if constexpr (is_65816(type)) {
			cpu.set_value_of(Register::DataBank, uint16_t(initial.at("dbr").integer()));
			cpu.set_value_of(Register::ProgramBank, uint16_t(initial.at("pbr").integer()));
			cpu.set_value_of(Register::Direct, uint16_t(initial.at("d").integer()));
			cpu.set_value_of(Register::EmulationFlag, uint16_t(initial.at("e").integer()));
		}
		cpu.set_value_of(Register::A, uint16_t(initial.at("a").integer()));
		cpu.set_value_of(Register::Flags, uint16_t(initial.at("p").integer()));
		cpu.set_value_of(Register::X, uint16_t(initial.at("x").integer()));
		cpu.set_value_of(Register::Y, uint16_t(initial.at("y").integer()));
		cpu.set_value_of(Register::ProgramCounter, uint16_t(initial.at("pc").integer()));
		cpu.set_value_of(Register::StackPointer, uint16_t(initial.at("s").integer()));

		for(const auto &cell: initial.at("ram").array) {
			bus.ram[uint32_t(cell[0].integer())] = uint8_t(cell[1].integer());
		}

		if constexpr (is_65816(type)) {
			const uint32_t pc = uint32_t(initial.at("pc").integer()) | (uint32_t(initial.at("pbr").integer()) << 16);
			const auto opcode = bus.ram.find(pc);
			bus.allow_pc_repetition = opcode != bus.ram.end() && (opcode->second == 0x54 || opcode->second == 0x44);
		}

		try {
			cpu.run_for(Cycles(MaxTestCycles));
		} catch(const StopException &) {}

		bool registers_ok = true;
		std::ostringstream why;
		const auto check = [&](const char *reg, int64_t actual, int64_t expected) {
			if(actual != expected) {
				registers_ok = false;
				why << ' ' << reg << '=' << actual << "!=" << expected;
			}
		};
		if constexpr (is_65816(type)) {
			check("dbr", cpu.value_of(Register::DataBank), final_state.at("dbr").integer());
			check("pbr", cpu.value_of(Register::ProgramBank), final_state.at("pbr").integer());
			check("d", cpu.value_of(Register::Direct), final_state.at("d").integer());
			check("e", cpu.value_of(Register::EmulationFlag), final_state.at("e").integer());
		}
		check("a", cpu.value_of(Register::A), final_state.at("a").integer());
		if constexpr (is_65816(type)) {
			check("p", cpu.value_of(Register::Flags), final_state.at("p").integer());
		} else {
			// On 6502-family parts B (bit 4) is not a register bit: it exists only
			// in copies of P pushed to the stack, which the RAM and bus-cycle
			// comparisons still check exactly. CLK reports it as 1; the vector
			// set records 0. Compare the remaining seven bits.
			check("p", cpu.value_of(Register::Flags) & 0xEF, final_state.at("p").integer() & 0xEF);
		}
		check("x", cpu.value_of(Register::X), final_state.at("x").integer());
		check("y", cpu.value_of(Register::Y), final_state.at("y").integer());
		check("s", cpu.value_of(Register::StackPointer), final_state.at("s").integer());
		check("pc", (cpu.value_of(Register::ProgramCounter) + bus.pc_overshoot) & 0xFFFF, final_state.at("pc").integer());

		bool ram_ok = true;
		for(const auto &cell: final_state.at("ram").array) {
			const auto found = bus.ram.find(uint32_t(cell[0].integer()));
			if(found == bus.ram.end() || found->second != uint8_t(cell[1].integer())) {
				ram_ok = false;
				why << " ram[" << cell[0].integer() << ']';
				break;
			}
		}

		const auto &expected_cycles = test.at("cycles").array;
		bool cycles_ok = expected_cycles.size() == bus.cycles.size();
		if(!cycles_ok) {
			why << " cycles=" << bus.cycles.size() << "!=" << expected_cycles.size();
		}
		for(size_t i = 0; cycles_ok && i < expected_cycles.size(); ++i) {
			const auto &want = expected_cycles[i];
			const auto &got = bus.cycles[i];
			const auto compare = [&](const atarix::json::Value &field, const auto &actual) {
				if(field.is_null()) return !actual.has_value();
				return actual.has_value() && int64_t(*actual) == field.integer();
			};
			bool same = compare(want[0], got.address) && compare(want[1], got.value);
			if constexpr (is_65816(type)) {
				same = same && lines_text(got) == want[2].text();
			} else {
				const bool want_read = want[2].text() == "read";
				same = same && (CPU::MOS6502Esque::is_read(got.operation) == want_read);
			}
			if(!same) {
				cycles_ok = false;
				why << " cycle#" << i;
			}
		}

		if(bus.unlisted_reads) ++result.unlisted_read_tests;

		if(registers_ok && ram_ok && cycles_ok) {
			++result.passed;
			continue;
		}

		const std::string opcode_key = result.file;
		if(known.count(opcode_key) || known.count(name)) {
			++result.known_deviation;
			continue;
		}
		if(!registers_ok) ++result.register_failures;
		if(!ram_ok) ++result.ram_failures;
		if(!cycles_ok) ++result.cycle_failures;
		if(result.first_failures.size() < 3) {
			result.first_failures.push_back(name + ":" + why.str());
		}
	}
	return result;
}

template <Type type>
int run_all(const Options &options) {
	const auto known = load_known(options.known_file);
	std::vector<FileResult> results;
	size_t total = 0, passed = 0, failed = 0, deviations = 0, empty = 0;
	for(const auto &file: options.files) {
		auto r = run_file<type>(file, options, known);
		if(r.empty_file) {
			++empty;
			std::printf("%-12s no vectors (empty upstream file): opcode not covered\n", r.file.c_str());
			results.push_back(std::move(r));
			continue;
		}
		const size_t failures = r.tests - r.passed - r.known_deviation;
		total += r.tests;
		passed += r.passed;
		failed += failures;
		deviations += r.known_deviation;
		if(failures || options.verbose) {
			std::printf("%-12s tests=%zu passed=%zu failed=%zu known=%zu (regs=%zu ram=%zu cycles=%zu)\n",
				r.file.c_str(), r.tests, r.passed, failures, r.known_deviation,
				r.register_failures, r.ram_failures, r.cycle_failures);
			for(const auto &f: r.first_failures) std::printf("    %s\n", f.c_str());
		}
		results.push_back(std::move(r));
	}

	std::printf("cpu=%s files=%zu tests=%zu passed=%zu failed=%zu known_deviations=%zu uncovered_files=%zu\n",
		options.cpu.c_str(), results.size(), total, passed, failed, deviations, empty);

	if(!options.report_file.empty()) {
		std::ofstream out(options.report_file);
		if(!out) {
			std::fprintf(stderr, "cannot write report %s\n", options.report_file.c_str());
			return 2;
		}
		out << "{\n  \"cpu\": \"" << options.cpu << "\",\n"
			<< "  \"files\": " << results.size() << ",\n"
			<< "  \"tests\": " << total << ",\n"
			<< "  \"passed\": " << passed << ",\n"
			<< "  \"failed\": " << failed << ",\n"
			<< "  \"known_deviations\": " << deviations << ",\n"
			<< "  \"uncovered_files\": " << empty << ",\n"
			<< "  \"per_file\": [\n";
		for(size_t i = 0; i < results.size(); ++i) {
			const auto &r = results[i];
			out << "    {\"file\": \"" << r.file << "\", \"empty\": " << (r.empty_file ? "true" : "false")
				<< ", \"tests\": " << r.tests
				<< ", \"passed\": " << r.passed
				<< ", \"known_deviations\": " << r.known_deviation
				<< ", \"register_failures\": " << r.register_failures
				<< ", \"ram_failures\": " << r.ram_failures
				<< ", \"cycle_failures\": " << r.cycle_failures
				<< ", \"tests_with_unlisted_reads\": " << r.unlisted_read_tests << '}'
				<< (i + 1 < results.size() ? ",\n" : "\n");
		}
		out << "  ]\n}\n";
	}
	return failed ? 1 : 0;
}

int usage() {
	std::fprintf(stderr,
		"usage: atarix-cpu-vectors --cpu <65816|wdc65c02|rockwell65c02|synertek65c02|6502|nes6502>\n"
		"                          [--max-tests N] [--known FILE] [--report FILE] [--verbose] FILE...\n");
	return 2;
}

}  // namespace

int main(int argc, char **argv) {
	Options options;
	for(int i = 1; i < argc; ++i) {
		const std::string_view arg(argv[i]);
		const auto next = [&]() -> std::string {
			if(i + 1 >= argc) throw std::runtime_error(std::string(arg) + " needs a value");
			return argv[++i];
		};
		try {
			if(arg == "--cpu") options.cpu = next();
			else if(arg == "--max-tests") options.max_tests = std::stoul(next());
			else if(arg == "--known") options.known_file = next();
			else if(arg == "--report") options.report_file = next();
			else if(arg == "--verbose") options.verbose = true;
			else if(!arg.empty() && arg[0] == '-') return usage();
			else options.files.emplace_back(arg);
		} catch(const std::exception &e) {
			std::fprintf(stderr, "%s\n", e.what());
			return usage();
		}
	}
	if(options.cpu.empty() || options.files.empty()) return usage();
	std::sort(options.files.begin(), options.files.end());

	try {
		if(options.cpu == "65816") return run_all<Type::TWDC65816>(options);
		if(options.cpu == "wdc65c02") return run_all<Type::TWDC65C02>(options);
		if(options.cpu == "rockwell65c02") return run_all<Type::TRockwell65C02>(options);
		if(options.cpu == "synertek65c02") return run_all<Type::TSynertek65C02>(options);
		if(options.cpu == "6502") return run_all<Type::T6502>(options);
		if(options.cpu == "nes6502") return run_all<Type::TNES6502>(options);
	} catch(const std::exception &e) {
		std::fprintf(stderr, "error: %s\n", e.what());
		return 2;
	}
	return usage();
}
