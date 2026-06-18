# ATARIX Toolchain Strategy

## Goals

Use open and maintainable tools wherever practical.

The toolchain should support firmware, operating-system development, FPGA development, simulation, and automated testing.

## Assembly Tools

Candidate tools:

- 64TASS
- WDC tools

## FPGA Tools

Primary path:

- Yosys
- nextpnr
- Project Trellis

Target platform:

- Lattice ECP5
- ULX3S

## Editors and IDEs

Candidate tools:

- Vim
- VS Code
- Foenix IDE

## Build System

Candidate tools:

- Make
- CMake research path
- CI automation

## Simulation

Candidate tools:

- Verilator
- Icarus Verilog
- Custom W65C816 emulation

## Firmware Development

Stages:

- ROM monitor
- Diagnostics
- Loader
- Kernel

## Long-Term Objectives

- Fully reproducible builds
- Automated regression testing
- FPGA simulation workflows
- Cross-platform development support