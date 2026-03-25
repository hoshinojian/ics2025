ENTRY(_start)
/* 指定 ELF 文件的入口点为 _start 符号（即 start.s 中的标号）。如果使用调试器加载这个 ELF 文件，调试器会将 PC 寄存器初始设置到这个符号的地址。 */
PHDRS { text PT_LOAD; data PT_LOAD; }
/* phdr： 程序头表， program headers.pt_load表示需要被加载器实际加载到物理内存 */
SECTIONS {
  /* _pmem_start and _entry_offset are defined in LDFLAGS */
  /* . 是定位器 */
  . = _pmem_start + _entry_offset;
  /* .text定义输出文件的.text section. entry强制要求所有entry必须在最前面, 确保了start.s里面的_start汇编指令成为8000 0000的第一跳指令 */
  .text : {
    *(entry)/*遍历所有的.o文件, 看见start.o里面有一个叫entry, 就放在这个位置*/
    *(.text*)
  } : text
  /* : text标志把这个.text丢到text头里面 */
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data*)
  } : data
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  }
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x8000;
  _stack_pointer = .;/*在start.s里面使用*/
  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
}
