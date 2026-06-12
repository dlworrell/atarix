# ATARIX CPU Validation Tests

This directory will contain CPU validation tests and harnesses.

Planned layout:

```text
tests/cpu/
├── upstream/
│   └── 65x02/
├── harness/
└── atarix-specific/
```

## Upstream Candidate

`dlworrell/65x02` provides JSON instruction-by-instruction tests for the 65x02 family.

The upstream data should be integrated only after the required test subsets are identified and licensing files are preserved.

## ATARIX-Specific Requirements

Instruction tests are necessary but not sufficient.

ATARIX also requires platform validation for:

- W65C816 native mode
- W65C816 emulation mode
- 24-bit addressing
- CPU-local SRAM
- CPU-local DMA memory
- mailbox access
- Identity EEPROM access
- supervisor watchdog behavior
- validation boot mode
