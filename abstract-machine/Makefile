# NAME = add
# SRCS = tests/add.c
# include $(AM_HOME)/Makefile


# Makefile for AbstractMachine Kernels and Libraries

### *Get a more readable version of this Makefile* by `make html` (requires python-markdown)
# Makefile指的是这个文件自身. 这一句语句完成缩进替换. 
# 伪目标, 强行执行

html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html

## 1. Basic Setup and Checks

### Default to create a bare-metal kernel image
# makefile里面, ifeq(1,2). 
# 搭配变量使用, ifeq($(something), x86). 
# 搭配shell使用 ifeq($(shell name), linux)
# 在这里的使用是判断变量是不是为空. 在这里, 如果makecmdgoals没有定义, 就会运行下面语句
# makecmdgoals是gnu make里面的变量. 执行make的时候, make程序读取命令行参数, 赋值给makecmdgoals
# 比如说 make run, 那么makecmdgoals 就是 run, make clean all, ~ clean alloc
# 这一个if判断， 当单纯make 的时候, 相当于是make image
# 但是目标 和 变量赋值不是一个东西. ARCH=riscv32-nemu run的时候, MAKECMDGOALS只是run

RED    := $(shell printf "\033[31m")
GREEN  := $(shell printf "\033[32m")
YELLOW := $(shell printf "\033[33m")
NONE   := $(shell printf "\033[0m")

PREFIX_AM := [=== AM/Makfile ===]

$(info $(YELLOW)$(PREFIX_AM)$(NONE))

# 默认目标。直接敲make 就变成make image
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

### Override checks when `make clean/clean-all/html`
# findstring GNU内置函数, 文本中查找子字符串
# findstring <要找什么>, <查找范围>. 找到了返回这个字符串, 没找到返回空
# 如果没有找到， 也就是说没有进行clean html, 那就执行if里面的
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

### Print build info message
# info类似于echo
# name 接受来自于am-kernels
# make ARCH=riscv32-nemu run的时候, 就是: 
# # Building add-run [riscv32-nemu]
$(info $(GREEN)$(PREFIX_AM)$(NONE)# Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

### Check: environment variable `$AM_HOME` looks sane
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME must be an AbstractMachine repo)
endif

### Check: environment variable `$ARCH` must be in the supported list
# notdir, 把路径里面的目录部分删掉, 只剩下文件名
# basename 去掉文件名的后缀
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
# filter, 在ARCHs列表里面寻找和ARCH一样的字符串
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error Expected $$ARCH in {$(ARCHS)}, Got "$(ARCH)")
endif

### Extract instruction set architecture (`ISA`) and platform from `$ARCH`. Example: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
# $(subst <from>,<to>,<text>). 把ARCH里面的- 改变成为空格. 
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

# 原本的ARCH是riscv32-nemu. 后面ISA变成了riscv32. platfrom变成了nemu

### Check if there is something to build
# flavor 内置函数, 如果变量没有定义过, 那么返回undefined
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif

### Checks end here
endif

## 2. General Compilation Targets

### Create the destination directory (`build/$ARCH`)
# run
# [=== AM/Makfile ===]WORK_DIR = /home/xingjian/ics2025/am-kernels/tests/cpu-tests
# [=== AM/Makfile ===]DST_DIR =  /home/xingjian/ics2025/am-kernels/tests/cpu-tests/build/riscv32-nemu
# [=== AM/Makfile ===]IMAGE_REL = build/dummy-riscv32-nemu
# [=== AM/Makfile ===]IMAGE = /home/xingjian/ics2025/am-kernels/tests/cpu-tests/build/dummy-riscv32-nemu
# [=== AM/Makfile ===]ARCHIVE = /home/xingjian/ics2025/am-kernels/tests/cpu-tests/build/dummy-riscv32-nemu.a

# [=== AM/Makfile ===]# Building am-archive [riscv32-nemu]
# [=== AM/Makfile ===]WORK_DIR = /home/xingjian/ics2025/abstract-machine/am
# [=== AM/Makfile ===]DST_DIR = /home/xingjian/ics2025/abstract-machine/am/build/riscv32-nemu
# [=== AM/Makfile ===]IMAGE_REL = build/am-riscv32-nemu
# [=== AM/Makfile ===]IMAGE = /home/xingjian/ics2025/abstract-machine/am/build/am-riscv32-nemu
# [=== AM/Makfile ===]ARCHIVE = /home/xingjian/ics2025/abstract-machine/am/build/am-riscv32-nemu.a

# [=== AM/Makfile ===]# Building klib-archive [riscv32-nemu]
# [=== AM/Makfile ===]WORK_DIR = /home/xingjian/ics2025/abstract-machine/klib
# [=== AM/Makfile ===]DST_DIR = /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu
# [=== AM/Makfile ===]IMAGE_REL = build/klib-riscv32-nemu
# [=== AM/Makfile ===]IMAGE = /home/xingjian/ics2025/abstract-machine/klib/build/klib-riscv32-nemu
# [=== AM/Makfile ===]ARCHIVE = /home/xingjian/ics2025/abstract-machine/klib/build/klib-riscv32-nemu.a
# 当make的目标不一样的时候, 几个变量也不一样
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))
#mkdir -p 强制创建目录. 如果运行dummy， 最后在/home/xingjian/ics2025/am-kernels/tests/cpu-tests/build/riscv32-nemu创建

### Compilation targets (a binary image or archive)
# 相对路径
# 获取pwd， 和相对路径拼接

IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

# $(info $(GREEN)$(PREFIX_AM)$(NONE)WORK_DIR = $(WORK_DIR))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)DST_DIR = $(DST_DIR))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)IMAGE_REL = $(IMAGE_REL))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)IMAGE = $(IMAGE))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)ARCHIVE = $(ARCHIVE))

### Collect the files to be linked: object files (`.o`) and libraries (`.a`)
# /home/.../build/riscv32-nemu/tests/dummy.o
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS) am klib) # lazy evaluation ("=") causes infinite recursions
LINKAGE   = $(OBJS)  # library archives are added by LIB_TEMPLATE below
# [=== AM/Makfile ===]# Building dummy-run [riscv32-nemu]
# [=== AM/Makfile ===] OBJS = /home/xingjian/ics2025/am-kernels/tests/cpu-tests/build/riscv32-nemu/tests/dummy.o
# [=== AM/Makfile ===] LIBS = am klib 
# [=== AM/Makfile ===] LINKAGE = /home/xingjian/ics2025/am-kernels/tests/cpu-tests/build/riscv32-nemu/tests/dummy.o  

# [=== AM/Makfile ===]# Building klib-archive [riscv32-nemu]
# [=== AM/Makfile ===] OBJS = /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/stdlib.o 
# /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/int64.o 
# /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/cpp.o 
# /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/stdio.o /
# home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/string.o
# [=== AM/Makfile ===] LIBS = am klib 
# [=== AM/Makfile ===] LINKAGE = /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/stdlib.o 
# /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/int64.o
#  /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/cpp.o 
#  /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/stdio.o
#   /home/xingjian/ics2025/abstract-machine/klib/build/riscv32-nemu/src/string.o  

# [=== AM/Makfile ===]# Building am-archive [riscv32-nemu]
# [=== AM/Makfile ===] OBJS = 
# [=== AM/Makfile ===] LIBS = am klib 
# [=== AM/Makfile ===] LINKAGE =   


## 3. General Compilation Flags

### (Cross) compilers, e.g., mips-linux-gnu-g++
# $(info $(GREEN)$(PREFIX_AM)$(NONE)CROSS_COMPILE = $(CROSS_COMPILE) )

AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf
# = 是延迟展开 和 递归赋值, :=才是立即展开. 所以这时候无所谓的

### Compilation flags
# 所有include加入到inc_path里面去
# 然后在所有的path前面加上一个-I, 成为-I/home/...
# -I给编译器指路, #include和-I配合
# include默认只找在标准路径下面的,但是这下面绝对没有am.h头文件. -I给编译器的搜索目录表里面强制加上新的. 
# gcc -I/home/xingjian/ics2025/abstract-machine/am/include -c dummy.c, 这时候先找标准目录, 没找到， 检查-I参数, 在后续的目录下面找到am.h

# [=== AM/Makfile ===]# Building dummy-run [riscv32-nemu]
# # INC_PATH /home/xingjian/ics2025/am-kernels/tests/cpu-tests/include
#  /home/xingjian/ics2025/abstract-machine/am/include/
#   /home/xingjian/ics2025/abstract-machine/klib/include/ 

# [=== AM/Makfile ===]# Building am-archive [riscv32-nemu]
# # INC_PATH /home/xingjian/ics2025/abstract-machine/am/src  赋值改成立即展开 := 之后, src消失
# /home/xingjian/ics2025/abstract-machine/am/include 
# /home/xingjian/ics2025/abstract-machine/am/include/ 
# /home/xingjian/ics2025/abstract-machine/klib/include/ 

# klib
# # INC_PATH /home/xingjian/ics2025/abstract-machine/klib/include 
# /home/xingjian/ics2025/abstract-machine/am/include/ 
# /home/xingjian/ics2025/abstract-machine/klib/include/ 
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
$(info # INC_PATH $(INC_PATH) )
INCFLAGS += $(addprefix -I, $(INC_PATH))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)INC_PATH = $(INC_PATH) )


ARCH_H := arch/$(ARCH).h

CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"$(ARCH_H)\"
# -fno-builtin -fno-stack-protector → 裸机专用（禁止 libc 替换和栈保护）

# 禁止生成用来站展开 stack unwinding 和 CFI Call frame information指令
# 默认情况, GCC生成`.eh_frame` 段，主要用于 C++ 异常处理（`try/catch`）或 C 语言中的堆栈回溯
# （如 `backtrace` 函数）以及调试器调试。使用此选项可以 == 精简生成的二进制文件体积。 == 
# 具体地, 核心功能是描述程序在运行到任何一条汇编指令时，如何准确地恢复上一层函数的寄存器状态和栈帧。
# 当程序出现异常的时候, 需要跨越多个stack找到匹配的catch块. 这个过程是站展开
# 这个时候, 程序就要准确知道每个调用层级的返回地址、以及如何恢复当时的寄存器
# eh_frame提供了静态数据表
# 不要保护
CFLAGS 	+=  -fno-asynchronous-unwind-tables 

# 阻止编译器将某些标准 C 库函数（如 `printf`、`strlen`、`memcpy` 等）识别为“内置函数”并进行自动优化或替换
# 如果没有这个选项, 编译器做分析的时候, 发现printf打印一个常量字符串, 那么就会换成puts
# 加上之后, 不做替换, 在写内核的时候是必须的, 因为内核要用自己实现的printf, 不能让编译器使用内核中可能没有的puts
CFLAGS	+=  -fno-builtin
# 我就是标准, 不要替换

# 禁用stack的溢出保护. 不加这个选项, 如果发生stack溢出覆盖, 程序触发__stack_chk_fial并且退出,
# 但是这玩意不一定存在, 所以加上, 开启保护可能导致链接失败. 
CFLAGS	+=  -fno-stack-protector
# 不要替换

# wno-main取消对main的签名的检查, 允许void main
CFLAGS  +=  -Wno-main 

# u == undefine, 取消这个定义. 这个宏把一些linux发型版本的标准头文件隐式开启, 把一些可能有
# 安全风险的 比如strcpy 替换成为带着边界检查的版本. 但是有边界检查功能的函数不一定实现.
CFLAGS	+=  -U_FORTIFY_SOURCE 
# 我就是标准

# 生成的ELF文件里面, 所有全局符号设置成为hidden, 减小动态符号表大小
CFLAGS  +=  -fvisibility=hidden
# 不要冗余

# freestanding告诉编译器不假设标准库存在, -fno-rtti依旧减小体积, 并且exce~ 关闭对抛出异常的处理
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -MMD $(INCFLAGS)
# asflag控制汇编文件的处理. -mmd和cflags的作用一样

#######################################################################################
# 总之, "我就是标准!" "每一个字节都很宝贵, 不要冗余" "不要乱七八糟的保护"
#######################################################################################


# 指示操作系统或运行环境：“该程序运行时的栈内存（Stack）区域没有执行权限”。这可以防止某些将机器指令恶意写入栈内存并执行的安全漏洞。
# 控制链接器的合并规则. -T的作用是放弃默认的内存排布, 按照linker.ld来
# +=也是延迟展开
LDFLAGS  += -z noexecstack $(addprefix -T, $(LDSCRIPTS))

## 4. Arch-Specific Configurations

### Paste in arch-specific configurations (e.g., from `scripts/x86_64-qemu.mk`)
-include $(AM_HOME)/scripts/$(ARCH).mk


## 5. Compilation Rules

### Rule (compile): a single `.c` -> `.o` (gcc)

# $(info $(GREEN)$(PREFIX_AM)$(NONE)WORK_DIR = $(WORK_DIR))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)DST_DIR = $(DST_DIR) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)IMAGE_REL = $(IMAGE_REL) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)IMAGE = $(IMAGE) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)ARCHIVE = $(ARCHIVE) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)OBJS = $(OBJS) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)LIBS = $(LIBS) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)LINKAGE = $(LINKAGE) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)CFLAGS = $(CFLAGS) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)CXXFLAGS = $(CXXFLAGS) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)ASFLAGS = $(ASFLAGS) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)COMMON_CFLAGS = $(COMMON_CFLAGS) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE)AM_SRCS = $(AM_SRCS))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)LDFLAGS = $(LDFLAGS))
# $(info $(GREEN)$(PREFIX_AM)$(NONE)INCFLAGS = $(INCFLAGS) )
# $(info $(GREEN)$(PREFIX_AM)$(NONE) ARCH_H        : $(ARCH_H))


$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

ifeq ($(MAKECMDGOALS),archive)
### Rule (archive): objects (`*.o`) -> `ARCHIVE.a` (ar)
# ar是打包的指令, 把多个 .o 文件 “打包压缩” 成的一个文件，也叫静态库，本质是 .o 文件的集合
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs $@ $^
else
# $(1): library name
define LIB_TEMPLATE =
$$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a: force
	@$$(MAKE) -s -C $$(AM_HOME)/$(1) archive
LINKAGE += $$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a
endef

### Rule (recursive make): build a dependent library (am, klib, ...)
$(foreach lib, $(LIBS), $(eval $(call LIB_TEMPLATE,$(lib))))
endif

### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `IMAGE.elf`, the final ELF binary to be packed into image (ld)
# LINKAGE = $(OBJS) + am-riscv32-nemu.a + klib-riscv32-nemu.a
# $(LDSCRIPTS) = linker.ld
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	@echo \# Creating image [$(ARCH)]
	@echo + LD "->" $(IMAGE_REL).elf
ifneq ($(filter $(ARCH),native),)
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif
	$(info $(GREEN)$(PREFIX_AM)$(NONE)Command image.elf Called)
# group句, 反复迭代


### Rule (`#include` dependencies): paste in `.d` files generated by gcc on `-MMD`
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous

### Build order control
image: image-dep
archive: $(ARCHIVE)
image-dep: $(IMAGE).elf
.PHONY: image image-dep archive run

### Force to rebuild a rule
force:
.PHONY: force

### Clean a single project (remove `build/`)
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

### Clean all sub-projects within depth 2 (and ignore errors)
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)