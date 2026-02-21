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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

extern CPU_state cpu;
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  bool flag = true;
  for(int i = 1; i < 31; i++){
    if(ref_r->gpr[i]!=cpu.gpr[i]){
      Log("Difftest error at PC = 0x%08x", pc);
      Log("Register [%d] mismatch: NEMU = 0x%08x, REF(Spike) = 0x%08x", 
          i, cpu.gpr[i], ref_r->gpr[i]);
      flag = false;
    }
  }
  if (cpu.pc != ref_r->pc) {
    Log("Difftest error at PC mismatch!");
    Log("NEMU PC = 0x%08x, REF(Spike) PC = 0x%08x", cpu.pc, ref_r->pc);
    flag = false;
  }
  return flag;
}

void isa_difftest_attach() {
}
