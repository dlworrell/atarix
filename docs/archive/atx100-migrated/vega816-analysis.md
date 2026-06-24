# Vega816 Hardware Migration Analysis

## Source

The Vega816 KiCad archive was reviewed as a reference design for ATARIX hardware planning.

The archive contains:

```text
Vega816.kicad_pro
Vega816.kicad_sch
Vega816.kicad_pcb
Vega816.kicad_sym
CPU Shim/
CPU Buffer/
DMA Controller/
Sysctrl Registers/
VectorPullRewrite/
Footprints.pretty/
```

## Purpose of This Analysis

Vega816 should be treated as a Rev 0 reference design for several low-level W65C816 hardware concepts.

It should not be copied wholesale into ATARIX because ATARIX is moving toward a modular workstation architecture with a structured fabric interface rather than a raw shared CPU bus.

## Vega816 Design Summary

The Vega816 design explores symmetric multiprocessing concepts for W65C816 systems, including:

- CPU shims.
- CPU buffering.
- DMA arbitration.
- Per-CPU bank-zero isolation.
- Multi-channel DMA.
- Vector Pull Rewrite.
- IRQ vector expansion.
- BB816-compatible connectors.

The design assumes a hardware environment closer to a shared-bus multiprocessor experiment than a managed workstation fabric.

## Subproject Review

### CPU Shim

Useful ideas:

- CPU-local adaptation layer.
- DMA request hooks.
- A22/A23-based DMA channel selection.
- Optional address rewrite.
- RDY-based CPU pausing.
- Vector Pull Rewrite integration.
- IRQ0B through IRQ7B interrupt concept.

ATARIX migration:

The CPU Shim concept maps well onto an ATARIX CPU-card bridge.

The exact BB816 connector dependency should not be retained as the long-term ATARIX interface.

### CPU Buffer

Useful ideas:

- Explicit CPU bus buffering.
- Direction-controlled data transceiver.
- DMAB-style bus isolation.
- Separation between CPU-side and DMA-side connectors.
- Use of standard logic for debug visibility.

ATARIX migration:

The buffering strategy is valuable for the Rev A CPU card and for debugging CPU-local timing.

The long-term backplane should not expose the entire raw CPU bus.

### DMA Controller

Useful ideas:

- Multiple DMA channels.
- Multiple CPU requestors.
- Per-CPU home DMA channel.
- Bank-zero isolation.
- Arbitration based on valid-address behavior.
- Cross-mapping of DMA channels between CPUs.

ATARIX migration:

The concept of per-CPU home memory and bank-zero isolation is important and should be preserved.

The direct shared-DMA wiring model should be replaced by fabric-mediated DMA and explicit capability grants.

See:

```text
docs/dma-engine-v1.md
docs/capability-model.md
```

### Vector Pull Rewrite

Useful ideas:

- Hardware recognition of W65C816 vector pulls.
- Expansion of IRQ vector handling.
- Multiple prioritized IRQ inputs.
- Native-mode vector rewrite behavior.

ATARIX migration:

This is one of the strongest Vega816 ideas and should be retained as an optional CPU-card feature.

It should be integrated with the ATARIX interrupt architecture rather than treated as a separate one-off circuit.

See:

```text
docs/interrupt-architecture-v1.md
```

### Sysctrl Registers

Useful ideas:

- Register-oriented control architecture.
- System-control register blocks.
- Hardware/software boundary definition.

ATARIX migration:

The concept is compatible with the ATARIX register map, but the register definitions should be normalized into the ATARIX fabric register model.

See:

```text
docs/register-map-v1.md
```

## Keep

The following Vega816 concepts should be kept:

1. CPU-local buffering.
2. CPU shim / bridge layer.
3. Vector Pull Rewrite.
4. Multiple prioritized IRQ inputs.
5. RDY-based wait-state control.
6. Per-CPU bank-zero isolation.
7. Explicit DMA request and grant behavior.
8. Observable discrete-logic bring-up path.

## Modify

The following concepts should be modified for ATARIX:

1. BB816-specific connector model.
2. Direct raw-bus expansion.
3. DMA channel mapping.
4. Multi-CPU arbitration.
5. System-control register layout.
6. Interrupt ownership model.

## Discard or Avoid

The following should not be carried directly into ATARIX:

1. Exposing the full raw W65C816 bus across the system backplane.
2. Treating DMA as an implicit bus right.
3. Treating multi-CPU sharing as primarily a wiring problem.
4. Assuming every participant sees every low-level CPU signal.

## ATARIX Migration Path

Recommended migration:

```text
Vega816 CPU Shim
    -> ATARIX CPU-card local bridge

Vega816 CPU Buffer
    -> ATARIX Rev A CPU-local buffering

Vega816 DMA Controller
    -> ATARIX FPGA-mediated DMA service

Vega816 Vector Pull Rewrite
    -> ATARIX CPU-card interrupt assist

Vega816 Sysctrl Registers
    -> ATARIX fabric register map
```

## Proposed ATARIX CPU Card Model

```text
W65C816
    |
Local SRAM / ROM
    |
CPU-local buffer and shim logic
    |
CPU-card FPGA or CPLD bridge
    |
ATARIX fabric interface
    |
DIN41612 backplane
```

## Architectural Conclusion

Vega816 is highly valuable as a source of low-level W65C816 hardware techniques.

Its strongest contribution to ATARIX is not the entire shared-bus architecture, but the recognition that a W65C816 multiprocessor or modular system needs:

- CPU-local mediation.
- Bus isolation.
- Wait-state control.
- Interrupt assist logic.
- Bank-zero awareness.
- Explicit DMA arbitration.

ATARIX should preserve those lessons while moving the system bus toward a Sun/NuBus-inspired fabric architecture.