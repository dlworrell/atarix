// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0

#include "cpu_card.hpp"

#include <stdexcept>
#include <utility>

namespace atarix::sim {

namespace {

constexpr uint32_t RomBase = 0x00E000;
constexpr uint32_t RomSize = 0x2000;
constexpr uint32_t RamTop = 0x800000;  // RAM is $000000-$007FFF and $010000-$7FFFFF
constexpr uint32_t UartData = 0x00C000;
constexpr uint32_t UartStatus = 0x00C001;
constexpr uint32_t IdentityBase = 0x00C300;
constexpr uint32_t FabricVersion = 0x00D000;
constexpr uint32_t FabricStatus = 0x00D001;

bool is_ram(uint32_t address) {
	return address < 0x008000 || (address >= 0x010000 && address < RamTop);
}

}  // namespace

CpuCard::CpuCard(std::vector<uint8_t> rom, Northbridge &northbridge, InterruptController &interrupts,
	SupervisorMailbox &supervisor, BoardIdentity identity)
	: rom_(std::move(rom)), ram_(RamTop, 0), northbridge_(northbridge), interrupts_(interrupts),
	  supervisor_(supervisor), identity_(identity) {
	if(rom_.size() != RomSize) {
		throw std::runtime_error("CPU card ROM image must be exactly 8192 bytes");
	}
	cpu_ = std::make_unique<Processor>(*this);
}

uint64_t CpuCard::run(uint64_t max_cycles) {
	cpu_->set_power_on(true);
	// Run in slices so STP is noticed promptly.
	while(!stopped_ && cycle_ < max_cycles) {
		cpu_->run_for(Cycles(1000));
	}
	return cycle_;
}

void CpuCard::fault(uint32_t address, bool write, const char *reason) {
	faults_.push_back({cycle_, address, write, reason});
}

uint8_t CpuCard::read(uint32_t address) {
	if(is_ram(address)) return ram_[address];
	if(address >= RomBase && address < RomBase + RomSize) return rom_[address - RomBase];

	uint8_t value = 0;
	if(northbridge_.read(address, value, cycle_)) return value;
	if(interrupts_.read(address, value)) return value;
	if(supervisor_.read(address, value)) return value;

	switch(address) {
		case UartData: return 0;              // no receive path in this test bench
		case UartStatus: return 0x02;         // TX ready, RX empty
		case IdentityBase + 0: return uint8_t(identity_.vendor);
		case IdentityBase + 1: return uint8_t(identity_.vendor >> 8);
		case IdentityBase + 2: return uint8_t(identity_.board);
		case IdentityBase + 3: return uint8_t(identity_.board >> 8);
		case IdentityBase + 4: return identity_.revision;
		case IdentityBase + 5: return identity_.capability_flags;
		case FabricVersion: return 0x01;
		case FabricStatus: return 0x01;       // fabric services ready
		default: break;
	}
	fault(address, false, "read of unmapped address");
	return 0xFF;
}

void CpuCard::write(uint32_t address, uint8_t value) {
	if(is_ram(address)) {
		ram_[address] = value;
		return;
	}
	if(address >= RomBase && address < RomBase + RomSize) {
		fault(address, true, "write to ROM ignored");
		return;
	}
	if(northbridge_.write(address, value, cycle_)) return;
	if(supervisor_.write(address, value, cycle_)) return;
	std::string interrupt_fault;
	if(interrupts_.write(address, value, interrupt_fault)) {
		if(!interrupt_fault.empty()) faults_.push_back({cycle_, address, true, interrupt_fault});
		return;
	}
	if(address == UartData) {
		uart_.push_back(char(value));
		return;
	}
	fault(address, true, "write to unmapped or read-only address");
}

void CpuCard::trace(CPU::MOS6502Esque::BusOperation operation, uint32_t address, uint8_t value) {
	if(!trace_) return;
	using CPU::MOS6502Esque::BusOperation;
	char kind = '-';
	switch(operation) {
		case BusOperation::ReadOpcode: kind = 'O'; break;
		case BusOperation::Read: kind = 'R'; break;
		case BusOperation::ReadProgram: kind = 'P'; break;
		case BusOperation::ReadVector: kind = 'V'; break;
		case BusOperation::Write: kind = 'W'; break;
		case BusOperation::InternalOperationRead: kind = 'i'; break;
		case BusOperation::InternalOperationWrite: kind = 'j'; break;
		default: kind = 'S'; break;
	}
	std::fprintf(trace_, "%llu %c %06X %02X\n",
		static_cast<unsigned long long>(cycle_), kind, unsigned(address & 0xFFFFFF), unsigned(value));
}

Cycles CpuCard::perform_bus_operation(CPU::MOS6502Esque::BusOperation operation, uint32_t address, uint8_t *value) {
	using CPU::MOS6502Esque::BusOperation;
	address &= 0xFFFFFF;
	switch(operation) {
		case BusOperation::ReadVector: {
			// VPB active: the Vector Pull Rewrite shim may alter the fetch address.
			const bool emulation = cpu_->get_extended_bus_output() & CPU::WDC65816::ExtendedBusOutput::Emulation;
			address = interrupts_.vector_pull(address, emulation, cycle_);
			*value = read(address);
		} break;
		case BusOperation::ReadOpcode:
		case BusOperation::Read:
		case BusOperation::ReadProgram:
			*value = read(address);
		break;
		case BusOperation::Write:
			write(address, *value);
		break;
		case BusOperation::InternalOperationRead:
		case BusOperation::InternalOperationWrite:
			*value = 0xFF;
		break;
		case BusOperation::None:
			// STP: the core idles with no bus activity until reset.
			stopped_ = true;
		break;
		default:
		break;
	}
	trace(operation, address, value ? *value : 0);
	cpu_->set_irq_line(interrupts_.irq_line());
	++cycle_;
	return Cycles(1);
}

}  // namespace atarix::sim
