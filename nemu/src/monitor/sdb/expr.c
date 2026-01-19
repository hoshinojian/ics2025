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
#include <stdio.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum
{
  TK_NOTYPE = 0,

  TK_DEC,

  TK_EQ,
  TK_ADD,
  TK_MIN,
  TK_MUL,
  TK_DIV,

  TK_LEFT,
  TK_RIGHT,

  /* TODO: Add more token types */

};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},  // spaces
    {"\\+", TK_ADD},    // plus
    {"\\-", TK_MIN},    // plus
    {"\\*", TK_MUL},    // plus
    {"\\/", TK_DIV},    // plus
    {"[0-9]+", TK_DEC}, // plus

    {"\\(", TK_LEFT},
    {"\\)", TK_RIGHT},

    {"==", TK_EQ}, // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
// 把所有rules转义
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    // int regcomp(regex_t *preg, const char *pattern, int cflags);
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

#define tokensSize 65535
static Token tokens[tokensSize] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    // 当前解析到的是第i个类型的组
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            //i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type)
        {
        case TK_NOTYPE:
          break;

        case TK_ADD:
        case TK_MIN:
        case TK_MUL:
        case TK_DIV:
        case TK_EQ:
        case TK_LEFT:
        case TK_RIGHT:
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;

        case TK_DEC:
          tokens[nr_token].type = TK_DEC;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;
        default:
          TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int numstk[65535];
int numstktop = 0;

int opstk[65535];
int opstktop = 0;

static void numspush(int n) { numstk[numstktop++] = n; }
static int numspop() { return numstk[--numstktop]; }

static void oppush(int n) { opstk[opstktop++] = n; }
static int oppop() { return opstk[--opstktop]; }
static int optop() { return opstk[opstktop - 1]; }

static void calc()
{
  int b = numspop();
  int a = numspop();
  int op = oppop();
  int ans = 0;
  switch (op)
  {
  case TK_ADD:
    ans = a + b;
    break;
  case TK_MIN:
    ans = a - b;
    break;
  case TK_MUL:
    ans = a * b;
    break;
  case TK_DIV:
    if (b == 0)
    {
      printf("Error: Division by zero\n");
      return;
    }
    ans = a / b;
    break;
  case TK_EQ:
    ans = (a == b);
    break;
  default:
    break;
  }
  numspush(ans);
}

static int op_to_idx(int token_type)
{
  switch (token_type)
  {
  case TK_ADD:
    return 0; // 对应矩阵第 0 行/列
  case TK_MIN:
    return 1; // 对应矩阵第 1 行/列
  case TK_MUL:
    return 2; // 对应矩阵第 2 行/列
  case TK_DIV:
    return 3; // 对应矩阵第 3 行/列
  case TK_LEFT:
    return 4; // 对应 (
  case TK_RIGHT:
    return 5; // 对应 )
  default:
    // 遇到数字或者错误的符号，返回一个非法下标
    return 9;
  }
}
//如果有\0作为开始结束就方便多了,但是在上面的函数里面没有定义,所以只能用更多心思处理
static char pri[10][10] = {
    // 左侧意味着栈顶,右侧意味seq. 1意味着栈顶先运算, 0意味入栈
    // 如果是0意味着非法, 如果是~意味着左括号出站. //右括号不可能在栈顶
    //              +    -    * /    (    )    ...填充0...
    /* + (0) */ {'>', '>', '<', '<', '<', '>', '0', '0', '0', '0'},
    /* - (1) */ {'>', '>', '<', '<', '<', '>', '0', '0', '0', '0'},
    /* * (2) */ {'>', '>', '>', '>', '<', '>', '0', '0', '0', '0'},
    /* / (3) */ {'>', '>', '>', '>', '<', '>', '0', '0', '0', '0'},
    /* ( (4) */ {'<', '<', '<', '<', '<', '~', '0', '0', '0', '0'},
    /* ) (5) */ {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
};
// 输入的是两个enum下来的数值
static char priority(int stacktopop, int seqop)
{
  if (opstktop == 0)
    return '<';
  int p = op_to_idx(stacktopop);
  int q = op_to_idx(seqop);
  return pri[p][q];
}

word_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  numstktop = 0;
  opstktop = 0;
  *success = true;

  for (int i = 0; i < nr_token; i++)
  {
    // 如果这个token的type是数字
    if (tokens[i].type == TK_DEC)
    {
      int num = atoi(tokens[i].str);
      numspush(num);
    }
    // 这个token的type是符号
    else // 在pa1_2的时候,这里只有十进制和运算符
    {
      int top_val;
      if (opstktop == 0)
      {
        top_val = 0;
      }
      else
      {
        top_val = optop();
      }
      char rel = priority(top_val, tokens[i].type);
      if (rel == '<')
      { // seq的优先级比栈顶的更高, 比如+ *
        oppush(tokens[i].type);
      }
      else if (rel == '>')
      { // 一次性计算所有的
        while (priority(optop(), tokens[i].type) == '>')
        {
          calc();
          if(opstktop == 0)break;
        }
        if(tokens[i].type == TK_RIGHT){
          oppop();
          continue;
        }
        else oppush(tokens[i].type);
      }
      else if (rel == '~')
      { // 左括号碰见右括号
        oppop();
      }
      else if (rel == '0')
      { // 非法
        *success = false;
        return 0;
      }
    }
  }
  while (opstktop)
  {
    calc();
  }
  //Log("The answer of the input seq is %d", numstk[0]);
  return numspop();
}
