/*
 * SPDX-FileCopyrightText: Donovan Worrell
 * SPDX-License-Identifier: Apache-2.0
 *
 * Minimal Cortex-M33 startup for host testing of the display card core on
 * QEMU's mps2-an505 board (same Armv8-M Mainline core and single-precision
 * FPU as the RP2350's Cortex-M33). Not the RP2350 boot path: the real
 * firmware uses the RP2350 boot ROM and SDK startup.
 *
 * Reset enables the FPU (CPACR CP10/CP11 full access) before any
 * floating-point instruction can run, then enters newlib's semihosting crt0.
 */

#include <stdint.h>

extern uint32_t __stack_top;
extern void _start(void);

void Reset_Handler(void);
void Fault_Handler(void);
void _init(void);
void _fini(void);

/* Linked with -nostartfiles: newlib's init/fini array walkers still call these. */
void _init(void) {}
void _fini(void) {}

__attribute__((naked, noreturn)) void Reset_Handler(void) {
    __asm volatile(
        "ldr r0, =0xE000ED88\n"     /* CPACR */
        "ldr r1, [r0]\n"
        "orr r1, r1, #(0xF << 20)\n"
        "str r1, [r0]\n"
        "dsb\n"
        "isb\n"
        "b _start\n");
}

__attribute__((noreturn)) void Fault_Handler(void) {
    /* Semihosting SYS_EXIT with ADP_Stopped_RunTimeErrorUnknown: QEMU exits non-zero. */
    __asm volatile(
        "mov r0, #0x18\n"
        "ldr r1, =0x20023\n"
        "bkpt 0xAB\n");
    for (;;) {
    }
}

__attribute__((section(".vectors"), used))
const void *const vector_table[16] = {
    &__stack_top,
    (const void *)Reset_Handler,
    (const void *)Fault_Handler, /* NMI */
    (const void *)Fault_Handler, /* HardFault */
    (const void *)Fault_Handler, /* MemManage */
    (const void *)Fault_Handler, /* BusFault */
    (const void *)Fault_Handler, /* UsageFault */
    (const void *)Fault_Handler, /* SecureFault */
    0, 0, 0,
    (const void *)Fault_Handler, /* SVCall */
    (const void *)Fault_Handler, /* DebugMonitor */
    0,
    (const void *)Fault_Handler, /* PendSV */
    (const void *)Fault_Handler, /* SysTick */
};
