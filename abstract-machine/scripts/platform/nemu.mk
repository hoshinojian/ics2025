PREFIX_am_nemu := [=== AM / platform / nemu.mk ===]

$(info $(YELLOW)$(PREFIX_am_nemu)$(NONE))
AM_SRCS := platform/nemu/trm.c \
           platform/nemu/ioe/ioe.c \
           platform/nemu/ioe/timer.c \
           platform/nemu/ioe/input.c \
           platform/nemu/ioe/gpu.c \
           platform/nemu/ioe/audio.c \
           platform/nemu/ioe/disk.c \
           platform/nemu/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
CFLAGS    += -I$(AM_HOME)/am/src/platform/nemu/include
LDSCRIPTS += $(AM_HOME)/scripts/linker.ld
LDFLAGS   += --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
# --defsym????????????????????????????????? _pmem_start ? 0x80000000??????linker.ld????????????????
LDFLAGS   += --gc-sections -e _start# -e _start??????????????????? _start ???????
# NEMUFLAGS += -l $(shell dirname $(IMAGE).elf)/nemu-log.txt
NEMUFLAGS += -b
# NEMUFLAGS += -e $(IMAGE).elf
# ºÍnemuÅäºÏ

MAINARGS_MAX_LEN = 64
MAINARGS_PLACEHOLDER = the_insert-arg_rule_in_Makefile_will_insert_mainargs_here
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=$(MAINARGS_PLACEHOLDER)


$(info $(GREEN)$(PREFIX_am_nemu)$(NONE)MAINARGS_MAX_LEN = $(MAINARGS_MAX_LEN))
$(info $(GREEN)$(PREFIX_am_nemu)$(NONE)MAINARGS_PLACEHOLDER = $(MAINARGS_PLACEHOLDER))
$(info $(GREEN)$(PREFIX_am_nemu)$(NONE)CFLAGS = $(CFLAGS))


# ??mainargs???
insert-arg: image
	@python $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) $(MAINARGS_PLACEHOLDER) "$(mainargs)"
    $(info $(GREEN)$(PREFIX_am_nemu)$(NONE)Command Insert-arg Called)

image: image-dep
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin
    $(info $(GREEN)$(PREFIX_am_nemu)$(NONE)Command image called)


run: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin
    $(info $(GREEN)$(PREFIX_am_nemu)$(NONE)Command run called)


gdb: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) gdb ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin
    $(info $(GREEN)$(PREFIX_am_nemu)$(NONE)Command GDB called)


.PHONY: insert-arg