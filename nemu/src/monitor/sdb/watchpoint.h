#ifndef WATCHPOINT_H  
#define WATCHPOINT_H

#define DEST 128

typedef struct watchpoint
{
  int NO;
  struct watchpoint *next;
  struct watchpoint *prev;

  // 要存储旧的值,同时要存储当前正在访问的是什么
  char EXPR[DEST];
  uint32_t old_value;

  // expr可以是寄存器, 可以是一个地址空间
  /* TODO: Add more members if necessary */

} WP;



void init_watchpoint(WP *wp);
WP* new_wp();
void free_wp(WP *wp);
void check_wp();

#endif