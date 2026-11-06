# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# ATARIX software test bench: CPU vector runner, emulated CPU card, test ROM.
# Run from the repository root:  make -C sim            (build everything)
#                                 make -C sim test-card  (boot + mailbox test)
# Requires: a C11 and C++20 compiler; ca65/ld65 (cc65) for the test ROM.

ROOT      := $(abspath ..)
BUILD     ?= $(ROOT)/build/sim
CLK       := $(ROOT)/third_party/clk
CC        ?= cc
CXX       ?= c++
CFLAGS    ?= -O2 -g
CXXFLAGS  ?= -O2 -g
WARN_C    := -std=c11 -Wall -Wextra -Werror
# Public headers use C11 _Static_assert; map it the same way
# scripts/engineering/check_headers.sh does for its C++ self-compile.
WARN_CXX  := -std=c++20 -Wall -Wextra -Werror -D_Static_assert=static_assert
CPPFLAGS  += -I$(ROOT)/include
CLKFLAGS  := -std=c++20 -w -I$(CLK)
CA65      ?= ca65
LD65      ?= ld65

LIB_SRCS  := $(ROOT)/src/mailbox/mailbox.c \
             $(ROOT)/src/ring/ring.c \
             $(ROOT)/src/capability/capability_evaluator.c \
             $(ROOT)/src/firmware/display-card/gpu_core.c
CLK_SRCS  := $(CLK)/Processors/65816/Implementation/65816Base.cpp \
             $(CLK)/Processors/65816/Implementation/65816Storage.cpp \
             $(CLK)/Processors/6502/Implementation/6502Storage.cpp

LIB_OBJS  := $(patsubst $(ROOT)/%.c,$(BUILD)/obj/%.o,$(LIB_SRCS))
CLK_OBJS  := $(patsubst $(ROOT)/%.cpp,$(BUILD)/obj/%.o,$(CLK_SRCS))

VECTORS   := $(BUILD)/atarix-cpu-vectors
CARD      := $(BUILD)/atarix-cpu-card
ROM       := $(BUILD)/cpu-card-test.rom
LIAR      := $(BUILD)/cpu-card-liar.rom

.PHONY: all test-card negative-control clean
all: $(VECTORS) $(CARD) $(ROM) $(LIAR)

$(BUILD)/obj/third_party/%.o: $(ROOT)/third_party/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CLKFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD)/obj/src/%.o: $(ROOT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(WARN_C) $(CPPFLAGS) -isystem $(ROOT)/third_party/cglm $(CFLAGS) -c $< -o $@

$(BUILD)/obj/sim/%.o: $(ROOT)/sim/%.cpp $(wildcard $(ROOT)/sim/cpu/*.hpp $(ROOT)/sim/cpu-card/*.hpp)
	@mkdir -p $(dir $@)
	$(CXX) $(WARN_CXX) $(CPPFLAGS) -I$(ROOT)/src/firmware/display-card -isystem $(CLK) $(CXXFLAGS) -c $< -o $@

$(VECTORS): $(BUILD)/obj/sim/cpu/vector_runner.o $(CLK_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(CARD): $(BUILD)/obj/sim/cpu-card/cpu_card.o $(BUILD)/obj/sim/cpu-card/northbridge.o $(BUILD)/obj/sim/cpu-card/interrupts.o $(BUILD)/obj/sim/cpu-card/gpu_service.o \
         $(BUILD)/obj/sim/cpu-card/main.o $(LIB_OBJS) $(CLK_OBJS)
	$(CXX) $(CXXFLAGS) $^ -lm -o $@

GPU_VECTORS := $(BUILD)/obj/firmware/gpu_vectors.inc

$(GPU_VECTORS): $(ROOT)/sim/cpu-card/firmware/gen_gpu_vectors.py
	@mkdir -p $(dir $@)
	python3 -I $< $@

$(ROM): $(ROOT)/sim/cpu-card/firmware/test_rom.s $(ROOT)/sim/cpu-card/firmware/rom.cfg $(GPU_VECTORS)
	@mkdir -p $(BUILD)/obj/firmware
	$(CA65) --cpu 65816 -I $(BUILD)/obj/firmware -o $(BUILD)/obj/firmware/test_rom.o -l $(BUILD)/obj/firmware/test_rom.lst $<
	$(LD65) -C $(ROOT)/sim/cpu-card/firmware/rom.cfg -o $@ $(BUILD)/obj/firmware/test_rom.o

$(LIAR): $(ROOT)/sim/cpu-card/firmware/liar_rom.s $(ROOT)/sim/cpu-card/firmware/rom.cfg
	@mkdir -p $(BUILD)/obj/firmware
	$(CA65) --cpu 65816 -o $(BUILD)/obj/firmware/liar_rom.o $<
	$(LD65) -C $(ROOT)/sim/cpu-card/firmware/rom.cfg -o $@ $(BUILD)/obj/firmware/liar_rom.o

test-card: $(CARD) $(ROM)
	$(CARD) --rom $(ROM) --trace $(BUILD)/cpu-card-bus.trace --report $(BUILD)/cpu-card-report.json

negative-control: $(CARD) $(ROM) $(LIAR)
	$(ROOT)/sim/cpu-card/negative_control.sh $(CARD) $(ROM) $(LIAR)

clean:
	rm -rf $(BUILD)
