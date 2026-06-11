# ATARIX ABI v1

## Purpose

This document defines the first draft of the ATARIX application binary interface for firmware, monitor, loader, and early kernel work.

The ABI is intentionally small. It exists to make ROM code, BIOS calls, kernel handoff, and early freestanding C code interoperable before the operating system is complete.

## Scope

This ABI covers:

- W65C816 execution mode
- register assumptions
- stack and direct-page assumptions
- firmware call conventions
- kernel-entry assumptions
- simple error/result conventions

It does not yet define a full userspace ABI.

## Execution Mode

Firmware shall enter the operating-system loader or kernel with the W65C816 in native mode.

Required state at handoff:

- emulation mode cleared
- decimal mode cleared
- interrupts masked unless explicitly documented otherwise
- stack valid
- direct page valid
- data bank register documented
- program bank register naturally set by long jump or long call

## Register Width

Unless a specific entry point says otherwise:

- accumulator: 16-bit
- X/Y index registers: 16-bit
- stack pointer: 16-bit native-mode stack

Code that changes register width must restore the documented ABI width before returning across an ABI boundary.

## Endianness

ATARIX follows W65C816 little-endian memory layout.

Multibyte fields in firmware-visible structures are little-endian unless explicitly marked otherwise.

## Address Model

The ABI distinguishes:

- CPU-local 24-bit addresses visible to the W65C816
- fabric addresses managed by the Fabric Northbridge
- service handles used to avoid hard-coding physical addresses

Early firmware calls should prefer handles and descriptors over raw fabric addresses.

## Stack

The caller owns temporary stack contents.

The callee must preserve stack balance.

Firmware and BIOS calls may use the caller stack for return addresses and small temporaries, but should not assume deep stack availability during early boot.

## Direct Page

Direct page is reserved for the active execution context.

Firmware shall publish the direct-page location in the Boot Information Block at kernel handoff.

Kernel code may relocate direct page after handoff, but must then own all consequences.

## Calling Convention Draft

For early firmware calls:

- A: primary argument or service number
- X: pointer low word or secondary argument
- Y: pointer bank/selector or tertiary argument
- carry clear: success
- carry set: failure
- A on return: result code or error code
- X/Y on return: optional result pointer or value

This convention is deliberately close to monitor/BIOS style firmware calls and easy to inspect with a debugger.

## Register Preservation

Unless otherwise documented:

- callee may clobber A, X, Y, and status flags
- callee must preserve DBR if it changes it
- callee must preserve stack balance
- long-lived service calls must document any additional clobbers

## Error Codes

Initial error-code space:

```text
0x0000  OK
0x0001  ERR_UNSUPPORTED
0x0002  ERR_INVALID_ARGUMENT
0x0003  ERR_NOT_FOUND
0x0004  ERR_BUSY
0x0005  ERR_TIMEOUT
0x0006  ERR_CHECKSUM
0x0007  ERR_PERMISSION
0x0008  ERR_IO
0x0009  ERR_NO_MEMORY
0x000A  ERR_BAD_STATE
```

## Kernel Entry

At kernel entry:

- A should contain the ABI version.
- X/Y should identify the Boot Information Block pointer according to the boot-firmware document.
- carry should be clear.
- stack and direct page must already be valid.

The kernel must validate the Boot Information Block before trusting service pointers.

## Freestanding C Notes

The early C environment should assume:

- no hosted libc
- no operating-system process model
- no implicit heap
- no file descriptors
- no startup runtime beyond explicitly linked reset/startup code

The ABI should be implemented first in assembly stubs, then wrapped by freestanding C headers.

## Versioning

The ABI version is a 16-bit value.

Recommended first value:

```text
0x0001
```

Breaking changes require a new ABI version.
