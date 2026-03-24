# --- 项目基础定义 ---
NAME = add
SRCS = tests/add.c
LIBS += am klib

# --- 1. 基础设置与环境检查 (原 Makefile) ---
ARCH ?= riscv32-nemu
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

RED    := $(shell printf "\033[31m")
GREEN  := $(shell printf "\033[32m")
YELLOW := $(shell printf "\033[33m")
NONE   := $(shell printf "\033[0m")
PREFIX_AM := [=== AM/Merged-Makefile ===]

ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME must be an AbstractMachine repo)
endif

# --- 2. 交叉编译工具链 (原 riscv.mk) ---
CROSS_COMPILE := riscv64-linux-gnu-
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf

# --- 3. 路径配置 ---
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

# --- 4. 编译选项 (Flags 整合) ---
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
INC_PATH += $(AM_HOME)/am/src/platform/nemu/include
INCFLAGS += $(addprefix -I, $(INC_PATH))

# 架构相关 (整合 riscv.mk 和 riscv32-nemu.mk)
ARCH_H := arch/riscv.h
# 强制指定 32 位参数
COMMON_CFLAGS := -fno-pic -mcmodel=medany -mstrict-align -march=rv32im_zicsr -mabi=ilp32 -static

CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"$(ARCH_H)\" -DISA_H=\"riscv/riscv.h\" \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden \
            -fdata-sections -ffunction-sections $(COMMON_CFLAGS)

# NEMU 参数 (原 nemu.mk)
MAINARGS_MAX_LEN = 64
MAINARGS_PLACEHOLDER = the_insert-arg_rule_in_Makefile_will_insert_mainargs_here
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=$(MAINARGS_PLACEHOLDER)

CXXFLAGS += $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -MMD $(INCFLAGS) $(COMMON_CFLAGS) -O0

# 链接选项 (原 nemu.mk)
LDSCRIPTS += $(AM_HOME)/scripts/linker.ld
LDFLAGS   += -z noexecstack $(addprefix -T, $(LDSCRIPTS))
LDFLAGS   += -melf32lriscv --gc-sections -e _start
LDFLAGS   += --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0

# --- 5. 源码采集 (原 riscv32-nemu.mk & nemu.mk) ---
AM_SRCS += riscv/nemu/start.S \
           riscv/nemu/cte.c \
           riscv/nemu/trap.S \
           riscv/nemu/vme.c \
           platform/nemu/trm.c \
           platform/nemu/ioe/ioe.c \
           platform/nemu/ioe/timer.c \
           platform/nemu/ioe/input.c \
           platform/nemu/ioe/gpu.c \
           platform/nemu/ioe/audio.c \
           platform/nemu/ioe/disk.c \
           platform/nemu/mpe.c

OBJS = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LINKAGE = $(OBJS)

# --- 6. 编译规则 ---

$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo "+ CC $<"
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo "+ CXX $<"
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo "+ AS $<"
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

# 递归构建库
ifeq ($(MAKECMDGOALS),archive)
$(ARCHIVE): $(OBJS)
	@echo "+ AR -> $(shell realpath $@ --relative-to .)"
	@$(AR) rcs $@ $^
else
define LIB_TEMPLATE =
$$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a: force
	@$$(MAKE) -s -C $$(AM_HOME)/$(1) archive
LINKAGE += $$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a
endef
$(foreach lib, $(LIBS), $(eval $(call LIB_TEMPLATE,$(lib))))
endif

# 生成 ELF
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	@echo "# Creating image [$(ARCH)]"
	@echo "+ LD -> $(IMAGE_REL).elf"
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group

# 最终 Image 生成 (原 nemu.mk)
image: image-dep
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo "+ OBJCOPY -> $(IMAGE_REL).bin"
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

image-dep: $(IMAGE).elf

# 运行与调试 (原 nemu.mk)
NEMUFLAGS += -b

insert-arg: image
	@python3 $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) $(MAINARGS_PLACEHOLDER) "$(mainargs)"

run: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin

gdb: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) gdb ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin

# 清理与辅助
clean:
	rm -rf $(WORK_DIR)/build/ Makefile.html

force:
.PHONY: image image-dep archive run clean insert-arg force gdb

-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))