.section entry, "ax"
//定义一个entry 段, ax标志指定属性是可分配 allocatable 和 executable, 可执行, 让链接脚本能够通过指定entry段的名字, 强行放在内存绝对地址上
.globl _start
//一个全局符号, 让外部课件. LDFLAGS   += --gc-sections -e _start# --entry _start, -e _start标志程序运行的时候, 第一个跳转的是什么地方
//@function指定了_start是函数类型符号
.type _start, @function

_start:
  mv s0, zero//把寄存器s0的值设置成为0. s0也就是fp, 帧指针. 
  la sp, _stack_pointer//la load address, 把_stack_pointer符号的绝对内存地址丢到sp, 栈指针里面去
  call _trm_init

.size _start, . - _start
//我们在前面讨论过的真正程序入口。它包含了放在物理内存 0x80000000 处的汇编指令，负责初始化栈指针（sp），并跳转到 C 语言环境。