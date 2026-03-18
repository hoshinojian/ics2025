PREFIX_riscv := [=== AM / isa / riscv.mk ===]

$(info $(YELLOW)$(PREFIX_riscv)$(NONE))

CROSS_COMPILE := riscv64-linux-gnu-

# -mstrict-align (严格内存对齐)：这极其关键。底层硬件（NEMU）处理未对齐的内存读写非常麻烦甚至会直接崩溃。
# 这个参数强制要求 GCC：遇到未对齐的内存访问时，必须自动把它拆分成多次单字节的读写指令，绝对不允许生成跨边界的直接访存指令。

# -fno-pic (禁用位置无关代码)：普通的 Linux 程序加载到内存的位置是随机的（为了安全）。
# 但在裸机里，代码必须放在绝对固定的物理地址（如 0x80000000）。这个参数强制编译器生成使用绝对地址或固定相对地址的机器指令。

# -mcmodel=medany (中等代码模型)：这是一个专门针对裸机环境的内存寻址参数。
# 它允许程序被链接到任意的物理地址范围内（通常是极高的地址如 0x80000000），并强制编译器使用 auipc 等基于 PC 相对寻址的指令来进行跳转和数据访问。
COMMON_CFLAGS := -fno-pic -march=rv64g -mcmodel=medany -mstrict-align
CFLAGS        += $(COMMON_CFLAGS) -static# 彻底静态链接
ASFLAGS       += $(COMMON_CFLAGS) -O0# 关闭所有优化
LDFLAGS       += -melf64lriscv	

# overwrite ARCH_H defined in $(AM_HOME)/Makefile
ARCH_H := arch/riscv.h

$(info $(GREEN)$(PREFIX_riscv)$(NONE) CROSS_COMPILE : $(CROSS_COMPILE))
$(info $(GREEN)$(PREFIX_riscv)$(NONE) CFLAGS        : $(CFLAGS))
$(info $(GREEN)$(PREFIX_riscv)$(NONE) ASFLAGS       : $(ASFLAGS))
$(info $(GREEN)$(PREFIX_riscv)$(NONE) LDFLAGS       : $(LDFLAGS))
$(info $(GREEN)$(PREFIX_riscv)$(NONE) ARCH_H        : $(ARCH_H))

