/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "log.h"
#include <common.h>
#include <elf.h>

extern uint64_t g_nr_guest_inst;
FuncInfo* funcinfo = NULL;
int global_func_count = 0;

Elf32_Shdr symtab_shdr;
Elf32_Shdr strtab_shdr;
FILE *func_log_fp = NULL;

#ifndef CONFIG_TARGET_AM
FILE *log_fp = NULL;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
#endif

void init_func_log(const char *func_log_file, const char *elf_file){
  if(func_log_file != NULL){
    parse_elf(elf_file);
    funcinfo = func_addr(symtab_shdr,strtab_shdr, elf_file, &global_func_count);
    FILE *fp = fopen(func_log_file, "w");
    Assert(fp, "Can not open '%s'", func_log_file);
    func_log_fp = fp;
    Log("Func LOG is written to %s", func_log_file);
  }
}




void parse_elf(const char *elf_file){
  FILE* fp = fopen(elf_file, "rb");
  if(!fp)Assert(fp, "Cannot open the ELF file");
  Elf32_Ehdr ehdr;
  //ehdr????????
  //?????????????? / ???, ?????, ??rb. 
  //size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
  if(fread(&ehdr, 1, sizeof(Elf32_Ehdr), fp) == 0)Assert(0,"assert of func");
  //??????????????
  //Elf32_Shdr symtab_shdr;
  for(int i = 0; i < ehdr.e_shnum; i++){
    fseek(fp, ehdr.e_shoff + i * ehdr.e_shentsize, SEEK_SET);
    if(fread(&symtab_shdr, 1, sizeof(Elf32_Shdr), fp) == 0)Assert(0,"assert of func");
    if(symtab_shdr.sh_type == SHT_SYMTAB)break;
  }

  //Elf32_Shdr strtab_shdr;
  //????sh+link??????????
  fseek(fp, ehdr.e_shoff + symtab_shdr.sh_link * ehdr.e_shentsize,    SEEK_SET);
  if(fread(&strtab_shdr, 1, sizeof(Elf32_Shdr), fp) == 0)Assert(0, "assert of func");

  fclose(fp);
  //????, ???????????????
};



FuncInfo* func_addr(Elf32_Shdr symtab_shdr, Elf32_Shdr strtab_shdr, const char *elf_file, int *out_func_count){
  FILE* fp = fopen(elf_file, "rb");
  //??????? = ????? / ??????
  int sym_count = symtab_shdr.sh_size / sizeof(Elf32_Sym);
  Elf32_Sym *syms = (Elf32_Sym *)malloc(symtab_shdr.sh_size);
  char *strtab = (char *)malloc(strtab_shdr.sh_size);

  //?????????syms???
  fseek(fp, symtab_shdr.sh_offset, SEEK_SET);
  if(fread(syms, 1, symtab_shdr.sh_size, fp) == 0)Assert(0, "assert of func");

  fseek(fp, strtab_shdr.sh_offset, SEEK_SET);
  if(fread(strtab, 1, strtab_shdr.sh_size, fp) == 0)Assert(0, "assert of func");

  //???????????func????
  FuncInfo *func_array = (FuncInfo *)malloc(sym_count * sizeof(FuncInfo));
  int actual_count = 0;

  for(int i = 0; i < sym_count; i++){
    Elf32_Sym* sym = &syms[i];
    
    if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC){
      // 4. ??????
      func_array[actual_count].func_name = strdup(strtab + sym->st_name);
      
      // ?????????
      func_array[actual_count].addr_begin = (void*)(uintptr_t)sym->st_value;
      func_array[actual_count].addr_end = (void*)(uintptr_t)(sym->st_value + sym->st_size);
      
      actual_count++;
    }
  }
  free(syms);
  free(strtab);
  fclose(fp);

  // ???????????
  if (actual_count > 0) {
      func_array = (FuncInfo *)realloc(func_array, actual_count * sizeof(FuncInfo));
  } else {
      free(func_array);
      func_array = NULL;
  }
  *out_func_count = actual_count;
  return func_array;
}



//Ä£·Âinit_log
#ifdef CONFIG_MM_TRACE
FILE *mm_log_fp = NULL;

void init_mm_log(const char *mm_log_file){
  if(mm_log_file != NULL){
    FILE *fp = fopen(mm_log_file, "w");
    Assert(fp, "Can not open '%s'", mm_log_file);
    mm_log_fp = fp;
    Log("MM LOG is written to %s", mm_log_file);
  #ifdef CONFIG_MM_TRACE_COND
    Log("MM_TRACE_COND is ON and the begin is %x. ",CONFIG_MM_TRACE_COND_START);
    Log("MM_TRACE_COND is ON and the end   is %x. ",CONFIG_MM_TRACE_COND_END);
  #endif
  }
}

#endif