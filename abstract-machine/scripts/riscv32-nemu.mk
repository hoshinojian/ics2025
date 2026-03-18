RED    := $(shell printf "\033[31m")
GREEN  := $(shell printf "\033[32m")
YELLOW := $(shell printf "\033[33m")
NONE   := $(shell printf "\033[0m")

PREFIX_riscv32_nemu := [=== AM/riscv32nemu.mk ===]

$(info$(YELLOW)$(PREFIX_riscv32_nemu)$(NONE))
$(info$(YELLOW)$(PREFIX_riscv32_nemu)$(NONE))
$(info$(YELLOW)$(PREFIX_riscv32_nemu)$(NONE))
$(info$(YELLOW)$(PREFIX_riscv32_nemu)$(NONE))
$(info$(YELLOW)$(PREFIX_riscv32_nemu)$(NONE))

include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/nemu.mk
#define ISA_H "riscv/riscv.h"。
CFLAGS  += -DISA_H=\"riscv/riscv.h\"
# 告诉gcc底层硬件的能力: -march=~, 指令集架构. 
COMMON_CFLAGS += -march=rv32im_zicsr -mabi=ilp32   # overwrite
# 最后的elf必须是32位, 小端序, riscv格式
LDFLAGS       += -melf32lriscv                     # overwrite

# /home/xingjian/ics2025/abstract-machine/am/src/riscv/nemu/start.S
AM_SRCS += riscv/nemu/start.S \
           riscv/nemu/cte.c \
           riscv/nemu/trap.S \
           riscv/nemu/vme.c
