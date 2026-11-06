// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Fabric interrupt controller, Vega816-style Vector Pull Rewrite, and the
// register-level Supervisor Mailbox, as seen by the CPU card.
//
// Interrupt controller at $00D100 uses the "preferred v1" layout of
// docs/interrupt-architecture-v1.md (which conflicts with the four-register
// draft in docs/register-map-v1.md; see the test-bench doc):
//   +00 Pending Low   R    +04 Priority Select R  (dispatch level 0-6, $FF none)
//   +01 Pending High  R    +05 Acknowledge     W  (source number)
//   +02 Mask Low      R/W  +06 Route Target    R/W (only $01 primary CPU)
//   +03 Mask High     R/W  +07 Status          R  (bit0 IRQ asserted, bit1 VPR enabled)
// A set mask bit masks the source. Boot default: all masked, nothing pending.
// Sources are level-triggered: acknowledge clears pending only once the
// source has deasserted.
//
// Vector Pull Rewrite (Vega816 README, "Vector Pull Rewrite Shim"): when VPB
// is active and A1-A3 are all high (only the IRQ vector matches), the
// dispatcher's 3-bit level is added to A1-A4 (A0 kept, carry out of A4
// dropped), so native IRQ levels 0-6 fetch $FFEE..$FFFA. ATARIX profile:
// rewrite only in native mode, and the dispatcher never emits level 7 in
// native mode (it would fetch the RESET vector).

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace atarix::sim {

enum InterruptSource: int {
	IrqUart = 0,
	IrqSupervisor = 1,
	IrqTimer = 2,
	IrqMailbox = 3,
	IrqDma = 4,
	IrqNetwork = 5,
	IrqAccelerator = 6,
	IrqExternalSlot = 7,
	IrqFabricFault = 8,
	IrqDebug = 9,
	IrqSourceCount = 10,
};

constexpr uint32_t InterruptBase = 0x00D100;
constexpr uint32_t InterruptLimit = 0x00D107;
constexpr uint32_t SupervisorMailboxBase = 0x00C200;
constexpr uint32_t SupervisorMailboxLimit = 0x00C205;

enum class InterruptInjection {
	None,
	NoVectorRewrite,   // dispatcher offset ignored: every IRQ fetches $FFEE
	IgnoreMask,        // masked sources still assert IRQ
	MailboxLevel7,     // mailbox dispatched at level 7 (pulls the RESET vector)
	SupervisorAcceptsReset,  // supervisor mailbox executes CPU reset requests
};

struct VectorPull {
	uint64_t cycle;
	uint32_t original;
	uint32_t rewritten;
	int level;          // -1 when not rewritten
	bool emulation;
};

struct SupervisorCommand {
	uint64_t cycle;
	uint8_t command;
	bool accepted;
};

class InterruptController {
public:
	InterruptController();

	void inject(InterruptInjection injection) { injection_ = injection; }

	// Device side: set the current level of a source line.
	void set_source(InterruptSource source, bool asserted);

	bool read(uint32_t address, uint8_t &value);
	bool write(uint32_t address, uint8_t value, std::string &fault);

	bool irq_line() const;
	int dispatch_level() const;  // 0-7, or -1 if nothing unmasked is pending

	// Called for every ReadVector bus cycle; returns the address to fetch.
	uint32_t vector_pull(uint32_t address, bool emulation, uint64_t cycle);

	const std::vector<VectorPull> &vector_pulls() const { return pulls_; }

	// Fixed level for each source: the priority order of
	// interrupt-architecture-v1 folded onto Vega816's seven native IRQ vectors.
	static int level_for(int source);

private:
	uint16_t pending_ = 0;
	uint16_t mask_ = 0xFFFF;
	uint16_t level_ = 0;
	uint8_t route_target_ = 0x01;
	int latched_level_ = -1;
	InterruptInjection injection_ = InterruptInjection::None;
	std::vector<VectorPull> pulls_;
};

// Register-level supervisor mailbox from docs/register-map-v1.md:
//   +00 COMMAND W   +01 STATUS R (bit1 done, bit7 rejected; reading clears done)
//   +02..+05 ARG0-ARG3 R
// Only command $03 (Read Board ID) is accepted from the CPU. Reset, recovery,
// fault-log and FPGA-reload commands are rejected: this register path does not
// pass through mailbox ring or capability checks.
class SupervisorMailbox {
public:
	SupervisorMailbox(InterruptController &interrupts, std::array<uint8_t, 4> board_id);

	void inject(InterruptInjection injection) { injection_ = injection; }

	bool read(uint32_t address, uint8_t &value);
	bool write(uint32_t address, uint8_t value, uint64_t cycle);

	const std::vector<SupervisorCommand> &commands() const { return commands_; }
	bool reset_requested() const { return reset_requested_; }

private:
	InterruptController &interrupts_;
	std::array<uint8_t, 4> board_id_;
	std::array<uint8_t, 4> args_{};
	uint8_t status_ = 0;
	bool reset_requested_ = false;
	InterruptInjection injection_ = InterruptInjection::None;
	std::vector<SupervisorCommand> commands_;
};

}  // namespace atarix::sim
