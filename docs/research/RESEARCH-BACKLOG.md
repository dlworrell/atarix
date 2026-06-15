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

## Guiding Principle

Research informs architecture.

Research does not become architecture until benchmarked, reviewed, documented, and accepted.