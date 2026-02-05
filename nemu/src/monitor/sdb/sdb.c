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
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <memory/host.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>
#include "watchpoint.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  int steps = 1;
  if(args != NULL) sscanf(args,"%d",&steps);
  if(steps <= 0)steps = 1;
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args){
  char _command;
  sscanf(args,"%c",&_command);
  if(_command == 'r'){
    isa_reg_display();
  }else if(_command == 'w'){

  }
  return 0;
}

//第一个版本的cmd_x,允许第二个参数是一个位置而不是一个待计算的seq


static int cmd_x(char *args){
  if(args == NULL)return 0;
  int steps = 0;
  int addr = 0;
  int n = sscanf(args,"%d %x", &steps, &addr);
  if(n == 2){
    for(int i = 0; i < steps;i++){
      printf("0x%x : %x\n",addr + (i * 4),vaddr_read(addr + (i *4), 4));
    }
  }else if(n == 1){
    printf("Parameter invalid\n");
  }
  return 0;
}

int cmd_p(char *args){
  if(args == NULL)return 0;
  bool success = true;
  word_t ans = expr(args,&success);
  printf("The answer in DEC is %d\n",ans);
  printf("The answer in HEX is 0x%x\n", ans);
  return 0;
}

int cmd_w(char *args){
  if(args == NULL)return 0;
  //把当前的断点塞到断点池里面去
  WP* wp = new_wp();
  if(!wp)return -1;
  sscanf(args, "%c", wp->EXPR);
  bool success = true;
  wp->old_value = expr(args,&success);
  return 0;
}
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Program by steps as the form of \"si [x]\", the default x is 1", cmd_si},
  { "info", "Print the state of the program. Eg: info r will print the REGS, and info w will print the info of watchpoints.", cmd_info },
  { "x", "Use the command in form of \"x N EXPR\", Eg:x 4 1+1. Calculate the expr, use the ans as the beginner position of mm, then print the following 4 * N Bytes. ", cmd_x},
  { "p", "Caculator", cmd_p},
  { "w", "Set a watchpoint", cmd_w},
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)



static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
