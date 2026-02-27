.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mv s0, zero
  la sp, _stack_pointer
  call _trm_init

.size _start, . - _start
//我们在前面讨论过的真正程序入口。它包含了放在物理内存 0x80000000 处的汇编指令，负责初始化栈指针（sp），并跳转到 C 语言环境。