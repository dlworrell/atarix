// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0

#include "interrupts.hpp"

namespace atarix::sim {

InterruptController::InterruptController() = default;

int InterruptController::level_for(int source) {
	// interrupt-architecture-v1 "Initial priority order", highest first,
	// folded onto native IRQ vector levels 0 (highest) to 6. Level 7 is never
	// used in native mode. Proposed mapping; see the test-bench doc.
	switch(source) {
		case IrqSupervisor:
		case IrqFabricFault: return 0;
		case IrqDma: return 1;
		case IrqMailbox: return 2;
		case IrqTimer: return 3;
		case IrqUart: return 4;
		case IrqNetwork:
		case IrqAccelerator: return 5;
		case IrqExternalSlot:
		case IrqDebug: return 6;
		default: return 6;
	}
}

void InterruptController::set_source(InterruptSource source, bool asserted) {
	const uint16_t bit = uint16_t(1u << source);
	if(asserted) {
		level_ |= bit;
		pending_ |= bit;
	} else {
		level_ &= uint16_t(~bit);
	}
}

int InterruptController::dispatch_level() const {
	const uint16_t active = injection_ == InterruptInjection::IgnoreMask ? pending_ : uint16_t(pending_ & ~mask_);
	int best = -1;
	for(int s = 0; s < IrqSourceCount; ++s) {
		if(!(active & (1u << s))) continue;
		int level = level_for(s);
		if(injection_ == InterruptInjection::MailboxLevel7 && s == IrqMailbox) level = 7;
		if(best < 0 || level < best) best = level;
	}
	return best;
}

bool InterruptController::irq_line() const {
	return dispatch_level() >= 0;
}

bool InterruptController::read(uint32_t address, uint8_t &value) {
	if(address < InterruptBase || address > InterruptLimit) return false;
	switch(address - InterruptBase) {
		case 0: value = uint8_t(pending_); break;
		case 1: value = uint8_t(pending_ >> 8); break;
		case 2: value = uint8_t(mask_); break;
		case 3: value = uint8_t(mask_ >> 8); break;
		case 4: {
			const int level = dispatch_level();
			value = level < 0 ? 0xFF : uint8_t(level);
		} break;
		case 5: value = 0; break;  // write-only; reads as zero
		case 6: value = route_target_; break;
		case 7: value = uint8_t((irq_line() ? 0x01 : 0) | 0x02); break;
		default: value = 0; break;
	}
	return true;
}

bool InterruptController::write(uint32_t address, uint8_t value, std::string &fault) {
	if(address < InterruptBase || address > InterruptLimit) return false;
	switch(address - InterruptBase) {
		case 2: mask_ = uint16_t((mask_ & 0xFF00) | value); break;
		case 3: mask_ = uint16_t((mask_ & 0x00FF) | (value << 8)); break;
		case 5:
			if(value >= IrqSourceCount) {
				fault = "acknowledge of unknown interrupt source";
			} else if(!(level_ & (1u << value))) {
				pending_ &= uint16_t(~(1u << value));
			}
			// Still asserted: pending stays set (level-triggered).
		break;
		case 6:
			if(value == 0x01) route_target_ = value;
			else fault = "unsupported interrupt route target (only $01 primary CPU)";
		break;
		default:
			fault = "write to read-only interrupt controller register";
		break;
	}
	return true;
}

uint32_t InterruptController::vector_pull(uint32_t address, bool emulation, uint64_t cycle) {
	// Vega816 detection: VPB active and A1-A3 all high. In both modes only
	// the IRQ vector ($FFEE/$FFEF native, $FFFE/$FFFF emulation) matches.
	const bool irq_vector = (address & 0xFFFFF0) == 0x00FFE0 || (address & 0xFFFFF0) == 0x00FFF0;
	const bool detect = irq_vector && (address & 0x0E) == 0x0E;
	if(!detect) {
		pulls_.push_back({cycle, address, address, -1, emulation});
		return address;
	}

	// Latch the level on the low byte so both bytes of the vector agree.
	if((address & 0x01) == 0) latched_level_ = dispatch_level();
	int level = latched_level_ < 0 ? 0 : latched_level_;

	// ATARIX profile: no rewrite in emulation mode; never level 7 natively.
	if(emulation || injection_ == InterruptInjection::NoVectorRewrite) level = 0;
	if(!emulation && level == 7 && injection_ != InterruptInjection::MailboxLevel7) level = 0;

	// 4-bit add into A1-A4 (74HC283), A0 kept, carry out of A4 dropped.
	const uint32_t field = (address >> 1) & 0x0F;
	const uint32_t sum = (field + uint32_t(level)) & 0x0F;
	const uint32_t rewritten = (address & ~uint32_t(0x1E)) | (sum << 1);
	pulls_.push_back({cycle, address, rewritten, level, emulation});
	return rewritten;
}

SupervisorMailbox::SupervisorMailbox(InterruptController &interrupts, std::array<uint8_t, 4> board_id)
	: interrupts_(interrupts), board_id_(board_id) {}

bool SupervisorMailbox::read(uint32_t address, uint8_t &value) {
	if(address < SupervisorMailboxBase || address > SupervisorMailboxLimit) return false;
	const uint32_t offset = address - SupervisorMailboxBase;
	if(offset == 0) {
		value = 0;
	} else if(offset == 1) {
		value = status_;
		// Reading STATUS consumes the completion and deasserts the interrupt.
		status_ &= uint8_t(~0x02);
		interrupts_.set_source(IrqSupervisor, false);
	} else {
		value = args_[offset - 2];
	}
	return true;
}

bool SupervisorMailbox::write(uint32_t address, uint8_t value, uint64_t cycle) {
	if(address < SupervisorMailboxBase || address > SupervisorMailboxLimit) return false;
	if(address != SupervisorMailboxBase) {
		status_ = 0x80;
		commands_.push_back({cycle, 0xFF, false});
		return true;
	}
	bool accepted = false;
	if(value == 0x03) {
		args_ = board_id_;
		accepted = true;
	} else if(value == 0x01 && injection_ == InterruptInjection::SupervisorAcceptsReset) {
		reset_requested_ = true;
		accepted = true;
	}
	commands_.push_back({cycle, value, accepted});
	status_ = accepted ? 0x02 : 0x80;
	if(accepted) interrupts_.set_source(IrqSupervisor, true);
	return true;
}

}  // namespace atarix::sim
