#ifndef __ELF_LOG_H__
#define __ELF_LOG_H__

#include <common.h>
#include <elf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// 定义函数信息结构体
typedef struct {
  char* func_name;
  void* addr_begin;
  void* addr_end;
} FuncInfo;

// 暴露的全局文件指针声明
extern FILE *log_fp;
extern FILE *func_log_fp;

#ifdef CONFIG_MM_TRACE
extern FILE *mm_log_fp;
#endif

// 暴露的 ELF 节头表声明
extern Elf32_Shdr symtab_shdr;
extern Elf32_Shdr strtab_shdr;

// 核心函数声明
void init_log(const char *log_file);
bool log_enable(void);

void parse_elf(const char *elf_file);
FuncInfo* func_addr(Elf32_Shdr symtab_shdr, Elf32_Shdr strtab_shdr, const char *elf_file, int *out_func_count);
void init_func_log(const char *func_log_file, const char *elf_file);

#ifdef CONFIG_MM_TRACE
void init_mm_log(const char *mm_log_file);
#endif

#endif // __ELF_LOG_H__

extern FuncInfo* funcinfo;
extern int global_func_count;

