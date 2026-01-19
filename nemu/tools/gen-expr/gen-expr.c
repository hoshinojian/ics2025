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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
//static char buf[35000] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

//往buf里面塞一个随机数学表达式

static int choose(int n){
  return rand() % n;
}

static int current = 0;//static每一轮调用完之后要归零

static void gen(char c){
  buf[current] = c;
  current++;
}

static void gen_rand_op(){
  switch(choose(4)){
    case 0:
      gen('+');
      break;
    case 1:
      gen('-');
      break;
    case 2:
      gen('*');
      break;
    case 3:
      gen('/');
      break;
  }
}

static void gen_num(){
  int num = rand() % 1000;
  int ret = sprintf(&buf[current], "%d", num);
  current += ret;
}

static void gen_rand_expr() {
  if(current > 60000){//防止溢出, 需要措施
  //if(current > 30000){
    gen_num();
    return;
    }
  switch(choose(3)){
    case 0: //生成一个数字
      gen_num();
      break;
    case 1://gen('('); gen_rand_expr(); gen(')'); break;
      gen('(');
      gen_rand_expr();
      gen(')');
      break;
    case 2://gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
      gen_rand_expr();
      gen_rand_op();
      gen_rand_expr();
      break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    current = 0;
    gen_rand_expr();
    buf[current] = '\0';//最后一个有可能是右括号,不是数字的情况下不会自动加上\0, 下一轮的时候会留下脏数据

    //把buf里面的内容和code_format的内容塞到code_buf里面去
    //sprintf按照指定的格式,把数据打印到一个缓冲区里面去
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);//把code_buf塞到fp里面去
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    //popopen建立一个管道,拿到执行之后打印出来的内容
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}


