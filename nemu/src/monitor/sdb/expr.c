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
#include <memory/paddr.h>
#include <memory/host.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum
{
  TK_NOTYPE = 0,

  TK_DEC,
  TK_HEC,

  TK_REG, //$()

  TK_EQ,
  TK_NEQ,
  TK_AND,

  // TK_STAR //解引用

  TK_ADD,
  TK_MIN,
  TK_STAR, // 同时用于乘法和解引用
  TK_MUL,
  TK_DER, // 解引用
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

    {"\\$(0|ra|sp|gp|tp|t[0-6]|a[0-7]|s([0-9]|1[0-1])|pc|fp|zero)", TK_REG},
    {" +", TK_NOTYPE}, // spaces
    {"\\+", TK_ADD},   // plus
    {"\\-", TK_MIN},   // plus
    {"\\*", TK_STAR},  // plus
    {"\\/", TK_DIV},   // plus

    {"0[xX][0-9a-fA-F]+", TK_HEC}, // 16进制应该在十进制前面
    {"[0-9]+", TK_DEC},            // plus

    {"\\(", TK_LEFT},
    {"\\)", TK_RIGHT},

    {"&&", TK_AND},
    {"!=", TK_NEQ},
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

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        // i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type)
        {
        case TK_NOTYPE:
          break;

        case TK_STAR: // 如果前面一个是数字, 那么就应该是乘法. 如果前面一个是运算符, 那么就应该是解引用
          // 假定当前是第一个元素, 那么一定是解引用
          if (nr_token == 0)
          {
            tokens[nr_token].type = TK_DER;
            nr_token++;
            break;
          }
          else
          {
            // 假定前面是十进制数字或者十六进制数字, 或者一个寄存器的取值, 或者右括号, 那么才是乘法
            if (tokens[nr_token - 1].type == TK_HEC || tokens[nr_token - 1].type == TK_DEC || tokens[nr_token - 1].type == TK_REG || tokens[nr_token - 1].type == TK_RIGHT)
            {
              tokens[nr_token].type = TK_MUL;
              nr_token++;
              break;
            }
            else
            {
              tokens[nr_token].type = TK_DER;
              nr_token++;
              break;
            }
          }

        case TK_EQ:
        case TK_NEQ:
        case TK_AND:

        case TK_ADD:
        case TK_MIN:
        case TK_DIV:
        case TK_LEFT:
        case TK_RIGHT:
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;

        case TK_REG:
        case TK_HEC:
        case TK_DEC:
          tokens[nr_token].type = rules[i].token_type;
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

u_int32_t numstk[65535];
int numstktop = 0;

int opstk[65535];
int opstktop = 0;

static void numspush(uint32_t n) { numstk[numstktop++] = n; }
static u_int32_t numspop() { return numstk[--numstktop]; }

static void oppush(int n) { opstk[opstktop++] = n; }
static int oppop() { return opstk[--opstktop]; }
static int optop() { return opstk[opstktop - 1]; }

static word_t pmem_read(paddr_t addr, int len)
{
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void calc_logical() // todo
{
  u_int32_t b = numspop();
  u_int32_t a = numspop();
  int op = oppop();
  uint32_t ans = 0;
  switch (op)
  {
  case TK_EQ:
    ans = (b == a);
    break;
  case TK_NEQ:
    ans = (b != a);
    break;
  case TK_AND:
    ans = (b && a);
    break;
  }
  numspush(ans);
}

static void calc_unary()
{ // 一元运算符, 解决der
  int op = oppop();
  u_int32_t num = 0;
  if (op == TK_DER)
  { // 这个时候完成解引用. 这个时候用来计算的一定是一个地址.
    num = pmem_read(numspop(), 4);
  }
  numspush(num);
}

//输入格式应该是0, pc, ..直接是寄存器的名字, 而不是$寄存器的名字
word_t isa_reg_str2val(const char *s, bool *success);


static void calc_arithmetic() // 只负责解决+ - * /
{
  u_int32_t b = numspop();
  u_int32_t a = numspop();
  int op = oppop();
  u_int32_t ans = 0;
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
    return 0; // 对应矩阵第 0 行/列 (+)
  case TK_MIN:
    return 1; // 对应矩阵第 1 行/列 (-)
  case TK_MUL:
    return 2; // 对应矩阵第 2 行/列 (* 乘法)
  case TK_DIV:
    return 3; // 对应矩阵第 3 行/列 (/)
  case TK_LEFT:
    return 4; // 对应 (
  case TK_RIGHT:
    return 5; // 对应 )
  case TK_REG:
    return 6; // 对应 $ (单目运算)
  case TK_DER:
    return 7; // 对应 * (解引用)
  case TK_EQ:
    return 8; // 对应 ==
  case TK_NEQ:
    return 9; // 对应 !=
  case TK_AND:
    return 10; // 对应 &&
  default:
    // 遇到数字(TK_DEC/TK_HEC)或者非法符号
    // 返回 11，对应矩阵最后那一行全 0 的位置
    return 11;
  }
}

// 如果有\0作为开始结束就方便多了,但是在上面的函数里面没有定义,只能用更多心思处理
static char pri[11][11] = {
    // 左侧意味着栈顶,右侧意味seq. 1意味着栈顶先运算, 0意味入栈
    // 如果是0意味着非法, 如果是~意味着左括号出站. //右括号不可栈顶
    //              +    -    * /    (    )      *.   ==   !=   &&..填充0...
    /* + (0) */ {'>', '>', '<', '<', '<', '>', '<', '>', '>', '>', '0'},
    /* - (1) */ {'>', '>', '<', '<', '<', '>', '<', '>', '>', '>', '0'},
    /* * (2) */ {'>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '0'},
    /* / (3) */ {'>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '0'},
    /* ( (4) */ {'<', '<', '<', '<', '<', '~', '<', '<', '<', '<', '0'},
    /* ) (5) */ {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},

    // 单目运算符高于逻辑运算符, 所以弹幕运算符遇上逻辑运算符的时候应该先出栈运算
    /* $ (6) */ //{'>', '>', '>', '>', '<', '>', '<', '<', '>', '>', '>', '0'}, 栈顶根本不会有这个元素, 当扫描到的时候立马把对应数字入数字stack
    /* * (7) */ {'>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '0'},

    // 判等和不等, 优先级都低于+-/*, 高于&&, 左结合
    /*== (8) */ {'<', '<', '<', '<', '<', '>', '<', '>', '>', '>', '0'},
    /*!= (9) */ {'<', '<', '<', '<', '<', '>', '<', '>', '>', '>', '0'},

    // 优先级最低
    /*&&(10) */ {'<', '<', '<', '<', '<', '>', '<', '<', '<', '>', '0'},
    // 留一行, 缺省为全0
};
// 加入$ 和 解引用两个运算符. 两个都是单目运算符. 所以和bang同优先级
// 假设是合法的式子, 那么一定是3 + $4或者3 * $4. 运算符先于运算数被扫描到, 所以一定要入栈

// 输入的是两个enum下来的数值
static char priority(int stacktopop, int seqop) // 加了多少运算符之后都不用变
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

  // todo: ~~十六进制~~, 解引用, 寄存器, 等于, 不等于, 与
  for (int i = 0; i < nr_token; i++)
  {
    // 如果这个token的type是数字或者寄存器
    if (tokens[i].type == TK_DEC || tokens[i].type == TK_HEC || tokens[i].type == TK_REG)
    {
      if (tokens[i].type == TK_DEC)
      {
        u_int32_t num = atoi(tokens[i].str); // atoi只能处理十进制数字, 处理不了十六进制
        numspush(num);
      }
      else if (tokens[i].type == TK_HEC)
      {
        u_int32_t num;
        sscanf(tokens[i].str, "%x", &num);
        numspush(num);
      }
      else
      { // 这个时候一定是寄存器. 寄存器出现一定就会立马用上, 所以可以在这里就调用寄存器阅读器
        numspush(isa_reg_str2val(tokens[i].str + 1, success));
      }
    }
    // 这个token的type是运算符
    else
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
          if (optop() == TK_ADD || optop() == TK_MUL || optop() == TK_DIV || optop() == TK_MIN)
          {
            calc_arithmetic();
            if (opstktop == 0)
              break;
          }
          else if (optop() == TK_DER)
          {
            calc_unary();
          }
          else
          { // 逻辑运算
            calc_logical();
          }
        }
        if (tokens[i].type == TK_RIGHT)
        {
          oppop();
          continue;
        }
        else
          oppush(tokens[i].type);
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
    if (optop() == TK_ADD || optop() == TK_MUL || optop() == TK_DIV || optop() == TK_MIN)
    {
      calc_arithmetic();
      if (opstktop == 0)
        break;
    }
    else if (optop() == TK_DER)
    {
      calc_unary();
    }
    else
    { // 逻辑运算
      calc_logical();
    }
  }
  // Log("The answer of the input seq is %d", numstk[0]);
  return numspop();
}
