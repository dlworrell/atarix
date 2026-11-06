// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Emulated ATARIX Rev A CPU card: W65C816 (vendored CLK core) on the bank-$00
// map of docs/memory-map.md and docs/register-map-v1.md.
//
//   $000000-$007FFF  RAM (direct page, stack, monitor work area)
//   $008000-$00BFFF  expansion window: unmapped (fault)
//   $00C000-$00C003  UART: +00 DATA, +01 STATUS (bit1 TX ready)
//   $00C300-$00C305  Board identity (read-only)
//   $00D000-$00D003  Fabric control (+00 version, +01 status)
//   $00D200-$00D2FF  Mailbox engine (Northbridge model)
//   $00E000-$00FFFF  ROM, vectors at top of bank (writes fault, contents unchanged)
//   $010000-$7FFFFF  RAM (CPU-local SRAM target, banks $01-$7F)
//   everything else  unmapped: reads return $FF and every access is a recorded fault
//
// Every bus cycle can be written to a trace for RTL co-simulation.

#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "Processors/65816/65816.hpp"
#include "northbridge.hpp"

namespace atarix::sim {

struct BoardIdentity {
	uint16_t vendor = 0xA7A1;   // simulator vendor id: not a registered identifier
	uint16_t board = 0x0816;
	uint8_t revision = 0x0A;
	uint8_t capability_flags = 0x01;
};

class CpuCard: public CPU::MOS6502Esque::BusHandler<uint32_t> {
public:
	CpuCard(std::vector<uint8_t> rom, Northbridge &northbridge, BoardIdentity identity);

	// Power on and run until STP, or until max_cycles. Returns cycles run.
	uint64_t run(uint64_t max_cycles);

	bool stopped() const { return stopped_; }
	const std::string &uart() const { return uart_; }
	const std::vector<BusFault> &faults() const { return faults_; }
	void set_trace(FILE *trace) { trace_ = trace; }

	// CLK bus-handler interface.
	Cycles perform_bus_operation(CPU::MOS6502Esque::BusOperation operation, uint32_t address, uint8_t *value);

private:
	using Processor = CPU::WDC65816::Processor<CpuCard, false>;

	std::vector<uint8_t> rom_;
	std::vector<uint8_t> ram_;
	Northbridge &northbridge_;
	BoardIdentity identity_;
	std::unique_ptr<Processor> cpu_;
	std::string uart_;
	std::vector<BusFault> faults_;
	FILE *trace_ = nullptr;
	uint64_t cycle_ = 0;
	bool stopped_ = false;

	uint8_t read(uint32_t address);
	void write(uint32_t address, uint8_t value);
	void fault(uint32_t address, bool write, const char *reason);
	void trace(CPU::MOS6502Esque::BusOperation operation, uint32_t address, uint8_t value);
};

}  // namespace atarix::sim
