/*
 * SPDX-FileCopyrightText: Donovan Worrell
 * SPDX-License-Identifier: Apache-2.0
 *
 * Linker script for host testing on QEMU mps2-an505 (Cortex-M33, secure).
 * Code at the secure SSRAM1 alias 0x10000000 (the reset VTOR); data and
 * stack in secure SSRAM2 at 0x38000000. QEMU loads every segment directly.
 */

MEMORY
{
    CODE (rx)  : ORIGIN = 0x10000000, LENGTH = 4M
    RAM  (rwx) : ORIGIN = 0x38000000, LENGTH = 2M
}

ENTRY(Reset_Handler)

SECTIONS
{
    .vectors : { KEEP(*(.vectors)) } > CODE

    .text :
    {
        *(.text*)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP(*(.preinit_array))
        __preinit_array_end = .;
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
        __fini_array_start = .;
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        __fini_array_end = .;
        *(.eh_frame*)
    } > CODE

    .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > CODE

    .data : { *(.data*) } > RAM

    .bss (NOLOAD) :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > RAM

    end = .;
    _end = .;
    __end__ = .;
    __stack_top = ORIGIN(RAM) + LENGTH(RAM);
    __stack = __stack_top;
}
