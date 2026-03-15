#***************************************************************************************
# Copyright (c) 2014-2024 Zihao Yu, Nanjing University
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

YELLOW := $(shell printf "\033[33m")
NONE := $(shell printf "\033[0m")
$(info $(YELLOW)file: NEMU/makefile is used $(NONE))


-include $(NEMU_HOME)/../Makefile
include $(NEMU_HOME)/scripts/build.mk

include $(NEMU_HOME)/tools/difftest.mk

compile_git:
	$(call git_commit, "compile NEMU")

	$(info $(YELLOW)compile_git called$(NONE))
$(BINARY):: compile_git

# Some convenient rules

# override ARGS ?= --batch
override ARGS += --log=$(BUILD_DIR)/nemu-log.txt

ifeq ($(CONFIG_MM_TRACE),y)
override ARGS += --mmlog=$(BUILD_DIR)/mmlog.txt
endif

ifeq ($(CONFIG_DEVICE_TRACE),y)
override ARGS += --device=$(BUILD_DIR)/devicelog.txt
endif

ifeq ($(CONFIG_FUNC_TRACE),y)
override ARGS += --funclog=$(BUILD_DIR)/funclog.txt

ifneq ($(IMG),)
# 匹配模式, 替代模式, 要切的参数
override ARGS += -e $(patsubst %.bin,%.elf,$(IMG))
endif
endif

# override ARGS += --elf=cd 
override ARGS += $(ARGS_DIFF)

# Command to execute NEMU
IMG ?=
ifeq ($(IMG), )
$(info $(YELLOW)IMG of NEMU is not assigned $(NONE))
else
$(info $(YELLOW)IMG of NEMU is $(IMG)$(NONE))
endif


NEMU_EXEC := $(BINARY) $(ARGS) $(IMG)


run-env: $(BINARY) $(DIFF_REF_SO)
	$(info $(YELLOW)Command run-env(native.mk) is used$(NONE))

run: run-env
	$(info $(YELLOW)Command run(native.mk) is used$(NONE))
	$(call git_commit, "run NEMU")
	$(NEMU_EXEC)

gdb: run-env
	$(call git_commit, "gdb NEMU")
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean-tools = $(dir $(shell find ./tools -maxdepth 2 -mindepth 2 -name "Makefile"))
$(clean-tools):
	-@$(MAKE) -s -C $@ clean
clean-tools: $(clean-tools)
clean-all: clean distclean clean-tools

.PHONY: run gdb run-env clean-tools clean-all $(clean-tools)
