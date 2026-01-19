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
  int ret = sprintf(&buf[current], "%dU", num);//转换成Unsigned, 但是这样生成所有数字后面都有个U,需要解决
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

//一个新的函数,当扫到u的时候就跳过
static void print_expr(char* c){
  for(int i = 0;i < strlen(c); i++){
    if(c[i] != 'U')putchar(c[i]);
  }
  putchar('\n');
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

    //编译阶段除以0会警告,改成error, 从而避免把这些东西写到expr里面去
    //-Werror=div-by-zero
    int ret = system("gcc -Werror=div-by-zero /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    //popopen建立一个管道,拿到执行之后打印出来的内容
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    //int result;
    uint32_t result;
    //如果因为除以0崩溃掉, 那么fscanf会返回-1
    //The fscanf() function returns the number of fields that it successfully converted and assigned. The return value does not include fields that the fscanf() function read but did not assign.
    //The return value is EOF if an input failure occurs before any conversion, or the number of input items assigned if successful.
    ret = fscanf(fp, "%u", &result);//fp写入的时候写成unsigned
    pclose(fp);
    if(ret != 1)continue;

    //printf("%u %s\n", result, buf);
    printf("%u ",result);
    print_expr(buf);
  }
  return 0;
}

//解决除以0错误
//在生成时候解决不太可能, 因为可能出现1 / (5 - 5). 生成时候不能知道后面这玩意是0
//整数除以0因为SIGFPE终止, 可以试着捕捉这个信号
//通过什么方式试着在输出的时候把有warning的停掉