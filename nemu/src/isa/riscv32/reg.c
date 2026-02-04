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

#include <isa.h>
#include "local-include/reg.h"
#include <memory/paddr.h>
#include <memory/host.h>

const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",   //fp对应s0. 
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

void isa_reg_display()
{
  for (int i = 0; i < 32; i++)
  {
    printf("%-4s 0x%08x    ", regs[i], gpr(i));
    if ((i + 1) % 4 == 0)
      printf("\n");
  }
  printf("pc   0x%08x\n",cpu.pc);
}

word_t isa_reg_str2val(const char *s, bool *success)
{
  //s的格式: $0 $ra ... $t6
  //把s输入到一个seq之中, 如果输入的大小是1, 那么就说明是$0, 否则进入另外一个判断
  if(strcmp(s, "0") == 0){
    *success = true;
    return 0;
  }else{//一定是寄存器
    for(int i = 1; i < 32; i++){
      if(strcmp(s, regs[i]) == 0){
        *success = true;
        return gpr(i);
      }
    }
  if(strcmp(s, "pc") == 0){
    *success = true;
    return cpu.pc;
  }
  if (strcmp(s, "fp") == 0) { // fp 就是 s0 (第8个)
        *success = true;
        return cpu.gpr[8];
    }
  if (strcmp(s, "zero") == 0) { // zero 就是 0号
        *success = true;
        return cpu.gpr[0];
    }
  }
  Log("No pattern name of the reg");
  return -1;
}
