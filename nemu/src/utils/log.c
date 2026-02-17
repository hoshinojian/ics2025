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

#include <common.h>

extern uint64_t g_nr_guest_inst;

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

//Ä£·Âinit_log
#ifdef CONFIG_MM_TRACE
FILE *mm_log_fp = NULL;

void init_mm_log(const char *mm_log_file){
  if(mm_log_file != NULL){
    FILE *fp = fopen(mm_log_file, "w");
    Assert(fp, "Can not open '%s'", mm_log_file);
    mm_log_fp = fp;
  }
  Log("MM LOG is written to %s", mm_log_file);
  #ifdef CONFIG_MM_TRACE_COND
    Log("MM_TRACE_COND is ON and the begin is %x. ",CONFIG_MM_TRACE_COND_START);
    Log("MM_TRACE_COND is ON and the end   is %x. ",CONFIG_MM_TRACE_COND_END);
  #endif
}

#endif