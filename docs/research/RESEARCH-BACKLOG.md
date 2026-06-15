# ATARIX Research Backlog

## Purpose

This document tracks research topics under investigation.

Architecture specifications represent decisions.
Research items represent investigations.

## Active Research Areas

### ATX-RESEARCH-002 Data-Oriented Systems Architecture

Focus:
- Ownership-centric design
- Arena allocation
- Cache-friendly layouts
- Zero-copy transport
- Authority movement vs data movement
- Persistent memory integration
- Recovery-aware design

### ATX-RESEARCH-003 Advanced Lookup and Index Structures

Focus:
- Krapivin open addressing
- Bubble-up d-ary cuckoo hashing
- Modern cuckoo filters
- Roaring bitmaps
- SPIDER rank/select structures
- Elias-Fano indexes
- Succinct data structures

Target Subsystems:
- Directory
- Services
- Capabilities
- Resources
- Audit

### ATX-RESEARCH-004 Memory Locality and Allocation Systems

Focus:
- Arena allocators
- Region allocators
- Generational allocation
- NUMA placement
- Fabric-aware memory placement
- Persistent memory
- Recovery-aware allocation

### ATX-RESEARCH-005 Zero-Copy and Data Movement Architectures

Focus:
- Zero-copy networking
- Scatter/gather I/O
- DMA capability models
- Buffer ownership systems
- Content-addressed transfer
- Fabric transport optimization

### ATX-RESEARCH-006 Prime Lookup and Sieve Acceleration

Focus:
- Existing OSE.c segmented 6-wheel CPU sieve from dlworrell/code-noodling
- Existing OSE_CUDA.cc multi-GPU segmented odd-only CUDA sieve from dlworrell/code-noodling
- JSON prime table export
- Cache-aware segmented sieves
- Bit-packed sieve representations
- Wheel factorization
- Incremental sieves
- Prime-counting algorithms
- Distributed sieve partitioning
- FPGA sieve acceleration
- Persistent prime databases
- Prime lookup services

Target Subsystems:
- Hash and lookup acceleration
- Memory locality research
- Data movement research
- Service-oriented computational fabrics
- Future math service experiments

## Guiding Principle

Research informs architecture.

Research does not become architecture until benchmarked, reviewed, documented, and accepted.
