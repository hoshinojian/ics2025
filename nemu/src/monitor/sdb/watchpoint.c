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

#include "sdb.h"

#define NR_WP 32

#define DEST 128
typedef struct watchpoint
{
  int NO;
  struct watchpoint *next;
  struct watchpoint *prev;

  // 要存储旧的值,同时要存储当前正在访问的是什么
  char EXPR[DEST];
  word_t old_value;

  // expr可以是寄存器, 可以是一个地址空间
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *tail = NULL, *free_begin = NULL;
// head组织检视点, free组织空闲的监视点结构

void init_wp_pool()
{
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].prev = (i == 0 ? NULL : &wp_pool[i - 1]);
    memset(wp_pool[i].EXPR, 0, DEST);
    wp_pool[i].old_value = 0;
  }
  head = NULL;
  tail = NULL;
  free_begin = wp_pool;
}

WP *new_wp()
{
  if (!free_begin)
    return NULL;
  WP *ans;
  // the pool is empty
  if (head == NULL && tail == NULL)
  {
    ans = free_begin;
    head = ans;
    tail = ans;
    free_begin = free_begin->next;
    free_begin->prev = NULL;
    head->prev = NULL;
    tail->next = NULL;
  }
  else
  {
    ans = free_begin;
    tail->next = ans;
    ans->prev = tail;
    tail = ans;
    free_begin = free_begin->next;
    if (free_begin)
      free_begin->prev = NULL;
    tail->next = NULL;
  }
  return ans;
};

void free_wp(WP *wp)
{
  if(!wp)return;
  WP *targ = head;
  for (; targ != NULL; targ = targ->next)
  {
    if (targ == wp)
      break;
  }
  if (!targ)
    return;

  if (wp == head)
  {
    head = head->next;
    if(head)head->prev = NULL;
  }
  if (wp == tail)
  {
    tail = tail->prev;
    if(tail)tail->next = NULL;
  }
  WP *PREV = wp->prev;
  WP *NEXT = wp->next;
  if (PREV)
    PREV->next = NEXT;
  if (NEXT)
    NEXT->prev = PREV;
  // 如果用光了所有监视点
  if (!free_begin){
    free_begin = wp;
        memset(wp->EXPR, 0, DEST);
    wp->old_value = 0;
    free_begin -> prev = NULL;
    free_begin -> next = NULL;
  }

  else
  {
    free_begin->prev = wp;
    wp->next = free_begin;
    free_begin = wp;
    memset(wp->EXPR, 0, DEST);
    wp->old_value = 0;
  }

  return;
}

/* TODO: Implement the functionality of watchpoint */
