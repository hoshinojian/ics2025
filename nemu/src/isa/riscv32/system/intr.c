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
#include "csrnums.h"
#include "cpu/cpu.h"

void set_nemu_state(int state, vaddr_t pc, int halt_ret);


word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  //如果是ecall
  // 1. 保存pc
  // 2. 保存mcause
  // 3. 跳转到mtvec
  mepc = epc;
  mcause = NO;
  if (NO==3)
  {
    //这里要完成nemu_trap
     NEMUTRAP(epc, cpu.gpr[10]);
  }
  
  return mtvec;
  //如果是ebreak
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
