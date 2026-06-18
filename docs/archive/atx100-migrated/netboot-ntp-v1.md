# ATARIX Netboot and NTP Boot Services v1

## Status

Draft boot-services specification.

This document records the decision that ATARIX Rev A should support FPGA-assisted networking services early enough to enable netboot and automatic time synchronization at boot.

## Purpose

Because ATARIX includes an FPGA fabric controller from Rev A, networking should be treated as a core bring-up and recovery feature rather than a late operating-system feature.

Netboot allows firmware and kernel images to be loaded without repeatedly programming ROMs or local storage.

NTP allows the RTC and fault logs to be corrected automatically at boot when a network is available.

## Design Goals

1. Support network-assisted bring-up.
2. Support automatic time setting at boot.
3. Keep recovery possible without a network.
4. Keep the boot path observable through UART diagnostics.
5. Avoid making network success mandatory for local boot.
6. Preserve a safe fallback path using RTC, ROM monitor, and recovery mode.

## Hardware Assumptions

Early networking may be provided by:

```text
W5500 Ethernet controller
FPGA-assisted network interface
Service card network controller
```

The W5500 remains the preferred early candidate because it minimizes TCP/IP stack burden on the W65C816.

## Boot-Time Network Services

Required boot-time services:

```text
Link detection
MAC address readout
DHCP or static IP configuration
NTP time synchronization
TFTP or simple netboot image retrieval
Boot-status reporting over UART
```

Optional later services:

```text
BOOTP
PXE-like boot descriptors
HTTP boot
Remote monitor commands
Remote fault-log retrieval
```

## Boot Order Integration

Network services fit into the boot sequence after minimal hardware initialization and before kernel handoff.

Recommended order:

```text
Power stable
Supervisor start
FPGA fabric ready
CPU reset release
ROM monitor start
Basic diagnostics
Discovery scan
Network service discovery
RTC read
NTP sync attempt
Netboot attempt if configured
Fallback to local boot or monitor
```

See:

```text
docs/boot-sequence-v1.md
```

## Time Initialization Policy

Boot-time time source priority:

```text
1. Valid NTP time
2. Valid RTC time
3. Supervisor retained timestamp
4. Build-time firmware timestamp
5. Unknown time
```

NTP should update the RTC only after a sanity check.

Sanity checks should include:

```text
NTP response valid
Time newer than firmware build time
Time not wildly outside expected range
Network source accepted by configuration
```

## RTC Role

The RTC remains required even with NTP.

RTC responsibilities:

```text
Maintain time without network
Timestamp watchdog resets
Timestamp fault logs
Timestamp recovery-mode entry
Provide boot-time fallback time
```

NTP corrects the RTC when available; it does not replace it.

## Netboot Modes

Candidate netboot modes:

```text
Disabled
Manual Only
Try Netboot Then Local
Require Netboot
Recovery Netboot
```

These may be selected by:

```text
DIP switch
Supervisor configuration
ROM monitor command
Stored configuration EEPROM
```

## Netboot Image Flow

A minimal netboot flow:

```text
1. Initialize network device
2. Acquire IP configuration
3. Locate boot server
4. Request boot image
5. Load image into RAM
6. Verify checksum
7. Prepare handoff structure
8. Jump to loader or kernel
```

## Boot Image Requirements

A boot image should include:

```text
Magic number
Image type
Load address
Entry point
Image length
Checksum or hash
Version string
Optional capability requirements
```

## Failure Policy

Network failure must not brick the system.

If NTP fails:

```text
Use RTC if valid
Mark time as unsynchronized
Continue boot unless policy requires synchronized time
```

If netboot fails:

```text
Fallback to local boot if permitted
Enter monitor if no valid boot source exists
Record failure reason
```

## Security Policy

Rev A may begin with checksum-based validation.

Future revisions should support:

```text
Signed boot images
Trusted boot-server configuration
Capability-restricted boot services
Network-service isolation
```

Netboot must not automatically grant DMA, accelerator, or privileged fabric access.

## Diagnostic Output

ROM monitor boot logs should show:

```text
Network device detected
Link status
MAC address
IP address
NTP result
RTC status
Netboot mode
Boot server
Image name
Image verification result
Fallback reason if any
```

## FPGA Role

The Rev A FPGA fabric controller may assist with:

```text
Network device register windowing
Interrupt routing
DMA gating for network buffers
Boot status LEDs
Timeout enforcement
Fault reporting
```

## Design Rule

Network boot and time synchronization are convenience and reliability features.

They must improve bring-up and maintenance without becoming single points of failure.

## Open Questions

- DHCP versus static IP as the Rev A default.
- TFTP versus simpler custom loader protocol.
- Whether W5500 is on CPU card, service card, or backplane service area.
- Boot image header format.
- Whether signed images are required before Rev B.
- How NTP sanity thresholds are configured.
- Whether RTC is on the supervisor card or fabric-controller/backplane card.