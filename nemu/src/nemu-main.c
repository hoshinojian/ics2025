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

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
int cmd_p(char *args);
typedef struct
{
  unsigned res;
  char expr[65536];
} TestPoint;

void TESTCALC()
{
  FILE *fp = fopen("./tools/gen-expr/build/input", "r");
  assert(fp != NULL);
  // fp里面每一行, 前面一个是结果后面一个是式子
  // 后面的一个喂给cmd_p.结果和前面一个做对比
  int linescount = 0;
  char ch;
  // fgetsc:循环读取每一个字符, 只要不是eof就继续
  while ((ch = fgetc(fp)) != EOF)
  {
    if (ch == '\n')
      linescount++;
  }
  // 第一行开始
  rewind(fp);

  TestPoint *points = (TestPoint *)malloc(sizeof(TestPoint) * linescount);
  int i = 0;
  while (i < linescount && fscanf(fp, "%u %[^\n]", &points[i].res, points[i].expr) == 2)
  {
    i++;
  }
  fclose(fp);

  i = 0;
  int WA = 0;
  while (i < linescount)
  {
    if (points[i].res != cmd_p(points[i].expr))
      WA++;
    i++;
  }
  printf("%d", WA);
  free(points);
}

int main(int argc, char *argv[])
{
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  //TESTCALC();

  return is_exit_status_bad();
}
