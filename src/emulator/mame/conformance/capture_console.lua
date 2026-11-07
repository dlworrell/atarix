-- SPDX-FileCopyrightText: Donovan Worrell
-- SPDX-License-Identifier: Apache-2.0
--
-- MAME autoboot script: records every byte written to the EP-A UART DATA
-- register ($00C000) and writes the transcript when the machine stops.
-- The output path comes from ATARIX_CONSOLE_LOG (default: console.log).

local transcript = {}
local space = manager.machine.devices[":maincpu"].spaces["program"]

uart_tap = space:install_write_tap(0x00c000, 0x00c000, "atarix_uart_tx",
    function(offset, data, mask)
        transcript[#transcript + 1] = string.char(data & 0xff)
    end)

stop_subscription = emu.add_machine_stop_notifier(function()
    local path = os.getenv("ATARIX_CONSOLE_LOG") or "console.log"
    local file = assert(io.open(path, "wb"))
    file:write(table.concat(transcript))
    file:close()
end)
