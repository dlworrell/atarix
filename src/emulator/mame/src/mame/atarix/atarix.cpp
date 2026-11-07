// license:BSD-3-Clause
// copyright-holders:Donovan Worrell
// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: BSD-3-Clause
/***************************************************************************

    ATARIX CPU card, emulation profile EP-A

    Implements the CPU-visible machine defined in ATX-SPEC-022 (Atarix
    repository, docs/architecture/ATX-SPEC-022-CPU-Card-Platform-Contract.md),
    under Atarix ADR-0016.

    Milestone M0 implements:
      - the W65C816S at 8 MHz;
      - CPU-local RAM at $000000-$00BFFF and $010000-$EFFFFF;
      - the 64 KiB boot ROM at $FF0000, loaded with -cart, with its top
        8 KiB mirrored at $00E000-$00FFFF;
      - the UART console at $00C000, shown on a generic terminal;
      - board identity at $00C300 (revision $EA marks the emulator);
      - fabric control at $00D000: version, status, control and fault
        recording;
      - unassigned accesses: reads return $FF, writes are ignored, and both
        set fault status bit 0 and record the address.

    Interrupts, timer, supervisor mailbox, mailbox engine and discovery
    arrive in milestones M1 to M3.

***************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/g65816/g65816.h"
#include "machine/terminal.h"

#include <deque>


namespace {

// fabric control fault status bits (ATX-SPEC-022)
constexpr u8 FAULT_UNASSIGNED = 0x01;

constexpr u32 BOOT_ROM_SIZE = 0x10000;

class atarix_state : public driver_device
{
public:
	atarix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bootrom(*this, "bootrom")
		, m_terminal(*this, "terminal")
	{ }

	void atarix(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	void record_fault(u8 bit, offs_t address);
	u8 unassigned_r(offs_t address);
	void unassigned_w(offs_t address);

	u8 bootrom_r(offs_t offset);

	u8 uart_r(offs_t offset);
	void uart_w(offs_t offset, u8 data);
	void terminal_key(u8 data);

	u8 board_id_r(offs_t offset);

	u8 fabric_r(offs_t offset);
	void fabric_w(offs_t offset, u8 data);

	required_device<g65816_device> m_maincpu;
	required_device<generic_slot_device> m_bootrom;
	required_device<generic_terminal_device> m_terminal;

	std::deque<u8> m_uart_rx;
	u8 m_uart_control = 0;
	u8 m_uart_baud = 0;

	u8 m_fabric_control = 0;
	u8 m_fault_status = 0;
	u32 m_fault_address = 0;
};


/***************************************************************************
    Faults and unassigned space
***************************************************************************/

void atarix_state::record_fault(u8 bit, offs_t address)
{
	if (machine().side_effects_disabled())
		return;
	m_fault_status |= bit;
	m_fault_address = address & 0xffffff;
	logerror("fabric fault %02x at %06x\n", bit, m_fault_address);
}

u8 atarix_state::unassigned_r(offs_t address)
{
	record_fault(FAULT_UNASSIGNED, address);
	return 0xff;
}

void atarix_state::unassigned_w(offs_t address)
{
	record_fault(FAULT_UNASSIGNED, address);
}


/***************************************************************************
    Boot ROM
***************************************************************************/

u8 atarix_state::bootrom_r(offs_t offset)
{
	return m_bootrom->read_rom(offset & (BOOT_ROM_SIZE - 1));
}


/***************************************************************************
    UART ($00C000)
***************************************************************************/

u8 atarix_state::uart_r(offs_t offset)
{
	switch (offset)
	{
	case 0: // DATA
	{
		if (m_uart_rx.empty())
			return 0x00;
		u8 const data = m_uart_rx.front();
		if (!machine().side_effects_disabled())
			m_uart_rx.pop_front();
		return data;
	}
	case 1: // STATUS: bit 0 RX ready, bit 1 TX ready
		return (m_uart_rx.empty() ? 0x00 : 0x01) | 0x02;
	case 2:
		return m_uart_control;
	case 3:
		return m_uart_baud;
	default:
		return 0x00; // reserved
	}
}

void atarix_state::uart_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		m_terminal->write(data);
		break;
	case 2:
		m_uart_control = data & 0x03;
		break;
	case 3:
		m_uart_baud = data; // a divisor on hardware; the emulated console ignores it
		break;
	default:
		break; // STATUS and reserved registers ignore writes
	}
}

void atarix_state::terminal_key(u8 data)
{
	m_uart_rx.push_back(data);
}


/***************************************************************************
    Board identity ($00C300)
***************************************************************************/

u8 atarix_state::board_id_r(offs_t offset)
{
	switch (offset)
	{
	case 0: return 0x00; // vendor ID, low (unassigned)
	case 1: return 0x00; // vendor ID, high
	case 2: return 0x16; // board ID $0816, low
	case 3: return 0x08; // board ID, high
	case 4: return 0xea; // revision: $EA marks the emulator (diagnostic only)
	case 5: return 0x00; // capability flags
	default: return 0x00;
	}
}


/***************************************************************************
    Fabric control ($00D000)
***************************************************************************/

u8 atarix_state::fabric_r(offs_t offset)
{
	switch (offset)
	{
	case 0: return 0x01;                    // fabric version
	case 1: return 0x01;                    // fabric status: ready
	case 2: return m_fabric_control;
	case 3: return m_fault_status;
	case 4: return m_fault_address & 0xff;
	case 5: return (m_fault_address >> 8) & 0xff;
	case 6: return (m_fault_address >> 16) & 0xff;
	case 7: return 0x00;                    // staged image status (quickload arrives in M4)
	default: return 0x00;
	}
}

void atarix_state::fabric_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 2:
		m_fabric_control = data & 0x0b;     // bit 2 (DMA) cannot be set in EP-A
		break;
	case 3:
		m_fault_status &= ~data;            // write 1 to clear
		break;
	default:
		break;
	}
}


/***************************************************************************
    Address map (ATX-SPEC-022)
***************************************************************************/

void atarix_state::mem_map(address_map &map)
{
	// Unassigned regions first; blocks mapped later in this function take
	// precedence over them.
	map(0x00c000, 0x00dfff).lrw8(
			NAME([this] (offs_t offset) { return unassigned_r(0x00c000 + offset); }),
			NAME([this] (offs_t offset, u8 data) { unassigned_w(0x00c000 + offset); }));
	map(0xf00000, 0xfeffff).lrw8(
			NAME([this] (offs_t offset) { return unassigned_r(0xf00000 + offset); }),
			NAME([this] (offs_t offset, u8 data) { unassigned_w(0xf00000 + offset); }));

	// CPU-local RAM
	map(0x000000, 0x00bfff).ram();
	map(0x010000, 0xefffff).ram();

	// I/O aperture
	map(0x00c000, 0x00c0ff).rw(FUNC(atarix_state::uart_r), FUNC(atarix_state::uart_w));
	map(0x00c300, 0x00c3ff).r(FUNC(atarix_state::board_id_r))
			.lw8(NAME([this] (offs_t offset, u8 data) { }));
	map(0x00d000, 0x00d0ff).rw(FUNC(atarix_state::fabric_r), FUNC(atarix_state::fabric_w));

	// Boot ROM, with its top 8 KiB mirrored into bank 0. ROM writes are faults.
	map(0x00e000, 0x00ffff).lrw8(
			NAME([this] (offs_t offset) { return bootrom_r(0xe000 + offset); }),
			NAME([this] (offs_t offset, u8 data) { unassigned_w(0x00e000 + offset); }));
	map(0xff0000, 0xffffff).lrw8(
			NAME([this] (offs_t offset) { return bootrom_r(offset); }),
			NAME([this] (offs_t offset, u8 data) { unassigned_w(0xff0000 + offset); }));
}


/***************************************************************************
    Machine
***************************************************************************/

void atarix_state::machine_start()
{
	if (!m_bootrom->exists())
		throw emu_fatalerror("atarix: a 64 KiB boot ROM image is required (-cart atarix-monitor.bin)");
	if (m_bootrom->get_rom_size() != BOOT_ROM_SIZE)
		throw emu_fatalerror("atarix: boot ROM image is %u bytes; EP-A requires %u", m_bootrom->get_rom_size(), BOOT_ROM_SIZE);

	save_item(NAME(m_uart_control));
	save_item(NAME(m_uart_baud));
	save_item(NAME(m_fabric_control));
	save_item(NAME(m_fault_status));
	save_item(NAME(m_fault_address));
}

void atarix_state::machine_reset()
{
	m_uart_rx.clear();
	m_uart_control = 0;
	m_fabric_control = 0;
	m_fault_status = 0;
	m_fault_address = 0;
}

static INPUT_PORTS_START( atarix )
INPUT_PORTS_END

void atarix_state::atarix(machine_config &config)
{
	G65816(config, m_maincpu, 8'000'000); // EP-A nominal clock
	m_maincpu->set_addrmap(AS_PROGRAM, &atarix_state::mem_map);

	GENERIC_CARTSLOT(config, m_bootrom, generic_plain_slot, "atarix_rom", "bin,rom");

	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(atarix_state::terminal_key));
}

ROM_START( atarix )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY            FULLNAME                         FLAGS
COMP( 2026, atarix, 0,      0,      atarix,  atarix, atarix_state, empty_init, "Donovan Worrell", "ATARIX CPU card (EP-A, M0)",    MACHINE_NO_SOUND_HW )
