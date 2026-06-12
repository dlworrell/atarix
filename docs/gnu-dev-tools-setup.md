# GNU Development Tools Setup

Status: Draft v1

## Purpose

This document defines the baseline GNU-style development environment for ATARIX.

ATARIX should build cleanly on a conventional Unix-like system using standard GNU development tools.

## Baseline Host Tools

Required for normal C development:

```text
GNU Make
GCC or Clang with C11 support
binutils
pkg-config
Git
```

Recommended for debugging and inspection:

```text
GDB
objdump
readelf
nm
addr2line
hexdump or xxd
```

Recommended for generated build systems:

```text
Autoconf
Automake
Libtool
m4
```

Recommended for static checks:

```text
cppcheck
clang-format
clang-tidy
shellcheck
```

Optional but useful:

```text
bison
flex
gperf
doxygen
graphviz
lcov
gcov
```

## Debian / Ubuntu Package Set

```sh
sudo apt update
sudo apt install \
    build-essential \
    gcc \
    g++ \
    make \
    binutils \
    gdb \
    git \
    pkg-config \
    autoconf \
    automake \
    libtool \
    m4 \
    bison \
    flex \
    cppcheck \
    clang-format \
    clang-tidy \
    shellcheck \
    doxygen \
    graphviz \
    lcov
```

## Fedora Package Set

```sh
sudo dnf install \
    gcc \
    gcc-c++ \
    make \
    binutils \
    gdb \
    git \
    pkgconf-pkg-config \
    autoconf \
    automake \
    libtool \
    m4 \
    bison \
    flex \
    cppcheck \
    clang-tools-extra \
    ShellCheck \
    doxygen \
    graphviz \
    lcov
```

## macOS Package Set

Using Homebrew:

```sh
brew install \
    make \
    gcc \
    binutils \
    gdb \
    autoconf \
    automake \
    libtool \
    m4 \
    bison \
    flex \
    cppcheck \
    clang-format \
    shellcheck \
    doxygen \
    graphviz \
    lcov
```

macOS may use Apple Clang for host tools and Homebrew GCC for stricter GNU compatibility testing.

## Expected Repository Workflow

The repository should support this development flow:

```sh
./bootstrap
./configure
make
make check
make distcheck
```

Early development may temporarily use direct compiler commands, but the long-term goal is a GNU-style build system.

## Current Direct Build Examples

Build the Discovery fixture generator:

```sh
cc -std=c11 -Wall -Wextra -Werror -Iinclude \
    tools/mkdiscovery/mkdiscovery.c \
    -o mkdiscovery
```

Generate valid Discovery fixtures:

```sh
mkdir -p tests/reference
./mkdiscovery --profile minimal --output tests/reference/minimal.discovery
./mkdiscovery --profile cpu-card --output tests/reference/cpu_card.discovery
./mkdiscovery --profile supervisor --output tests/reference/supervisor.discovery
./mkdiscovery --profile instrumentation --output tests/reference/instrumentation.discovery
```

Build a Discovery test manually:

```sh
cc -std=c11 -Wall -Wextra -Werror -Iinclude \
    tests/discovery/test_discovery_header.c \
    src/discovery/discovery_database.c \
    src/discovery/discovery_iterator.c \
    src/discovery/discovery_validate.c \
    src/discovery/discovery_parser.c \
    -o test_discovery_header
```

Run it:

```sh
./test_discovery_header
```

## Planned GNU Build Files

Planned files:

```text
bootstrap
configure.ac
Makefile.am
src/Makefile.am
tests/Makefile.am
tools/Makefile.am
m4/
```

## Compiler Policy

Default language standard:

```text
C11
```

Default warning policy:

```text
-Wall -Wextra -Werror
```

Recommended development flags:

```text
-g -O0 -fsanitize=address,undefined
```

Recommended release flags:

```text
-O2
```

## Cross-Compilation Direction

ATARIX host tools should build on the development host.

Firmware and target-specific code should eventually support cross-compilation using explicit toolchain prefixes:

```sh
./configure --host=<target-triplet>
```

Potential future targets:

```text
65C816 toolchain
RP2350 firmware toolchain
ECP5 FPGA toolchain
RISC-V cross toolchain
m68k cross toolchain
PowerPC cross toolchain
```

## Policy

The GNU toolchain setup exists to make ATARIX reproducible, testable, and portable.

New C modules should avoid hidden IDE assumptions.

Every runtime subsystem should eventually be reachable through:

```sh
make check
```

## Related Documents

- discovery-rom-format-v1.md
- resource-type-registry-v1.md
- service-id-registry-v1.md
- operation-id-registry-v1.md
- capability-record-format-v1.md
