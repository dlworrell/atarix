# ATARIX Memory Map

## Purpose

This document captures the first-pass ATARIX memory-map plan. It is intentionally conservative for Rev A and leaves room for FPGA-mediated banking, device windows, DMA control, and capability enforcement in later revisions.

## Design Rules

1. Rev A must be simple enough to debug with ordinary tools.
2. ROM, RAM, and UART must be easy to verify independently.
3. Later FPGA windows must not invalidate the Rev A bring-up model.
4. Device registers must be explicitly documented.
5. DMA-visible memory must eventually be mediated by the fabric.

## W65C816 Addressing Context

The W65C816 presents a 24-bit address space organized as 256 banks of 64 KiB each.

For early bring-up, ATARIX should treat bank `$00` as the most important bank because it contains reset vectors, interrupt vectors, direct-page assumptions, stack behavior, and most monitor activity.

## Rev A Draft Map

```text
Bank $00

$000000-$0000FF  Direct page / zero-page-compatible region
$000100-$0001FF  Hardware stack default region
$000200-$007FFF  Low RAM / monitor work area
$008000-$00BFFF  Expansion or device window candidate
$00C000-$00DFFF  I/O window candidate
$00E000-$00FFFF  ROM window, vectors at top of bank
```

This map favors easy decoding and easy monitor development over density.

## Rev A Required Regions

### RAM

Minimum useful RAM:

- Direct page
- Stack page
- Monitor variables
- Upload buffer
- Test area

### ROM

ROM must contain:

- Reset vector
- Native interrupt vectors
- Emulation-mode vectors if used during bring-up
- Monitor firmware
- Memory-test entry points
- Serial loader entry points

### UART

The UART should be placed in the I/O window and decoded simply.

Candidate location:

```text
$00C000-$00C00F  UART register block
```

### GPIO

Candidate location:

```text
$00C100-$00C1FF  GPIO register block
```

## Rev B Extensions

Rev B may add:

- W5500 Ethernet register window
- Supervisor mailbox
- Board identity registers
- Recovery control registers

Candidate locations:

```text
$00C200-$00C2FF  Supervisor mailbox
$00C300-$00C3FF  Board identity / configuration
$00C400-$00C7FF  W5500 window
```

## Rev C FPGA Register Space

Rev C introduces FPGA-mediated system registers.

Candidate locations:

```text
$00D000-$00D0FF  FPGA system control
$00D100-$00D1FF  Interrupt controller
$00D200-$00D2FF  Mailbox registers
$00D300-$00D3FF  DMA control
$00D400-$00D4FF  Timer block
$00D500-$00D5FF  Device discovery block
```

## Future Banked Memory

Banks above `$00` may eventually be used for expanded RAM, memory windows, accelerator buffers, or DMA-visible regions.

Candidate organization:

```text
$010000-$7FFFFF  General RAM / banked system memory
$800000-$BFFFFF  Device and accelerator windows
$C00000-$EFFFFF  Fabric-controlled mappings
$F00000-$FFFFFF  Reserved / ROM shadow / debug / recovery
```

This is provisional and should not be treated as a final hardware contract.

## DMA Visibility

DMA-capable devices must not receive unrestricted access to the full address space.

Future DMA regions should include:

- Owner CPU or supervisor
- Start address
- Length
- Direction
- Expiration or revocation rule
- Fault behavior

## Open Questions

- Final Rev A ROM size
- Final Rev A SRAM size
- Exact UART part
- Whether Rev A uses a 16 KiB or 8 KiB I/O aperture
- Whether ROM is mirrored for simple decoding
- Whether FPGA registers enter at Rev C only or appear in limited form in Rev B