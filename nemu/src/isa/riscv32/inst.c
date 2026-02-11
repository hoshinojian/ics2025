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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum
{
  TYPE_R,
  TYPE_I,
  TYPE_S,
  TYPE_B,
  TYPE_U,
  TYPE_J,

  TYPE_N, // none
};

#define src1R()     \
  do                \
  {                 \
    *src1 = R(rs1); \
  } while (0)
#define src2R()     \
  do                \
  {                 \
    *src2 = R(rs2); \
  } while (0)



#define immI()                        \
  do                                  \
  {                                   \
    *imm = SEXT(BITS(i, 31, 20), 12); \
  } while (0)

#define immS()                                               \
  do                                                         \
  {                                                          \
    *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); \
  } while (0)


#define immB()                                       \
  do                                                 \
  {                                                  \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 12) |        \
           (BITS(i, 7, 7) << 11) |                   \
           (BITS(i, 30, 25) << 5) |                  \
           (BITS(i, 11, 8) << 1);                    \
  } while (0)


#define immU()                              \
  do                                        \
  {                                         \
    *imm = SEXT(BITS(i, 31, 12), 20) << 12; \
  } while (0)


/* J-type: imm[20] | imm[10:1] | imm[11] | imm[19:12] | rd | opcode */
/* Content:  s  |  inst[30:21] | i11 | inst[19:12] | rd | 1101111  */
#define immJ()                                                \
  do                                                          \
  {                                                           \
    uint32_t i = s->isa.inst;                                 \
    word_t rs = (BITS(i,31,31) << 20)   |                     \
                (BITS(i, 19, 12) << 12) |                     \
                (BITS(i, 20, 20) << 11) |                     \
                (BITS(i, 30, 21) << 1);                       \
    *imm = (SEXT(rs, 21));                                    \
  } while (0);
  

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type)
{
  uint32_t i = s->isa.inst;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd = BITS(i, 11, 7);   //rd是一个地址 src1 src2都是地址
  switch (type)
  {
  case TYPE_R:
    src1R();
    src2R();
    break;
  case TYPE_I:
    src1R();
    immI();
    break;
  case TYPE_S:
    src1R();
    src2R();
    immS();
    break;

  case TYPE_B:
    src1R();
    src2R();
    immB();
    break;

  case TYPE_U:
    immU();
    break;

  case TYPE_J:
    immJ();
    break;
  case TYPE_N:
    break;

  default:
    panic("unsupported type = %d", type);
  }
}

static int decode_exec(Decode *s)
{
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst)   //INSTPAT INST s 代表s的指令
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)         \
  {                                                                  \
    int rd = 0;                                                      \
    word_t src1 = 0, src2 = 0, imm = 0;                              \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
    __VA_ARGS__;                                                     \
  }
  // s: 解码结构体指针
  // name 指令名字
  // type 什么类型的， uisj， 决定了怎么取出来立即数和寄存器索引
  // ... 可变参数, 接受具体的C代码. 
  //----------------------------------
  //decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); 自动解码操作数
  //concat TYPE_type, 完成自动的拼接, 之后自动完成执行
  // __VA_ARGS__; 完成最后 "..."的指令

  INSTPAT_START();
  //INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
  INSTPAT("0000000 ????? ????? 000 ????? 0110011", add, R, {R(rd) = src1 + src2;});
  INSTPAT("0100000 ????? ????? 000 ????? 0110011", sub, R, R(rd) = src1 - src2;);
  INSTPAT("0000000 ????? ????? 001 ????? 0110011", sll, R, {R(rd) = src1 << (src2 & 0x1f);});//逻辑左移最低五个 of src2
  INSTPAT("0000000 ????? ????? 010 ????? 0110011", slt, R, R(rd) = ((int32_t)src1 < (int32_t)src2););
  INSTPAT("0000000 ????? ????? 011 ????? 0110011", sltu, R, {R(rd) = ((uint32_t)src1 < (uint32_t)src2);});
  INSTPAT("0000000 ????? ????? 100 ????? 0110011", xor, R, {R(rd) = src1 ^ src2;});
  INSTPAT("0000000 ????? ????? 101 ????? 0110011", srl, R, {R(rd) = src1 >> ((uint32_t)src2 & 0x1f);});//逻辑右移
  INSTPAT("0100000 ????? ????? 101 ????? 0110011", sra, R, {R(rd) = (int32_t)src1 >> ((int32_t)src2 & 0x1f);});//算数右移
  INSTPAT("0000000 ????? ????? 110 ????? 0110011", or, R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 0110011", and, R, R(rd) = src1 & src2);

  INSTPAT("??????? ????? ????? 000 ????? 0010011", addi,  I, R(rd) = src1 + imm);
  INSTPAT("??????? ????? ????? 010 ????? 0010011", slti,  I, R(rd) = ((int32_t)src1 < (int32_t)imm));
  INSTPAT("??????? ????? ????? 011 ????? 0010011", sltiu, I, R(rd) = (uint32_t)src1 < (uint32_t)imm);
  INSTPAT("??????? ????? ????? 100 ????? 0010011", xori,  I, R(rd) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 110 ????? 0010011", ori,   I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 111 ????? 0010011", andi,  I, R(rd) = src1 & imm);
  INSTPAT("0000000 ????? ????? 001 ????? 0010011", slli,  I, R(rd) = src1 << (imm & 0x1f));
  INSTPAT("0000000 ????? ????? 101 ????? 0010011", srli,  I, R(rd) = src1 >> (imm & 0x1f));
  INSTPAT("0100000 ????? ????? 101 ????? 0010011", srai,  I, R(rd) = (int32_t)src1 >> (imm & 0x1f));
  // sltiu: 无符号比较立即数
  INSTPAT("??????? ????? ????? 011 ????? 0010011", sltiu, I, R(rd) = src1 < imm);

  INSTPAT("??????? ????? ????? 000 ????? 0000011", lb,  I, R(rd) = SEXT(Mr(src1 + imm, 1), 8));
  INSTPAT("??????? ????? ????? 001 ????? 0000011", lh,  I, R(rd) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("????????????  ????? 010 ????? 0000011", lw, I, {R(rd) = Mr((src1 + imm), 4);});
  INSTPAT("??????? ????? ????? 100 ????? 0000011", lbu, I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 0000011", lhu, I, R(rd) = Mr(src1 + imm, 2));
  //src1就是R[rs1]
  INSTPAT("??????? ????? ????? 000 ????? 0100011", sb, S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 0100011", sh, S, {Mw(src1 + imm,2,src2);});
  INSTPAT("??????? ????? ????? 010 ????? 0100011", sw, S, {Mw(src1 + imm,4,src2);});

  INSTPAT("??????? ????? ????? ??? ????? 0010111", auipc, U, R(rd) = s->pc + imm);
  INSTPAT("????????????????????    ????? 0110111", lui, U, {R(rd) = imm;});

  INSTPAT("????????????  ????? ??? ????? 1101111", jal, J, {R(rd) = s->snpc; s->dnpc = s->pc + imm;});
  INSTPAT("????????????  ????? 000 ????? 1100111", jarl, I, {int target_ = (src1 + imm) & ~1; R(rd) = s->snpc; s->dnpc = target_;});

  INSTPAT("??????? ????? ????? 000 ????? 1100011", beq, B, {s->dnpc = src1 == src2 ? s->pc + imm : s-> snpc;});
  INSTPAT("??????? ????? ????? 001 ????? 1100011", bne, B, {s->dnpc = src1 != src2 ? s->pc + imm : s-> snpc;});
  INSTPAT("??????? ????? ????? 100 ????? 1100011", blt, B, {s->dnpc = (int32_t)src1 <  (int32_t)src2 ? s->pc + imm : s-> snpc;});
  INSTPAT("??????? ????? ????? 101 ????? 1100011", bge, B, {s->dnpc = (int32_t)src1 >=  (int32_t)src2 ? s->pc + imm : s-> snpc;});
  INSTPAT("??????? ????? ????? 110 ????? 1100011", bltu, B, {s->dnpc = (uint32_t)src1 < (uint32_t)src2 ? s->pc + imm : s-> snpc;});
  INSTPAT("??????? ????? ????? 111 ????? 1100011", bgeu, B, {s->dnpc = (uint32_t)src1 >= (uint32_t)src2 ? s->pc + imm : s-> snpc;});

/*        RISCV M EXTENSION        */
  INSTPAT("0000001 ????? ????? 000 ????? 0110011", mul, R, {R(rd) = src1 * src2;});
  INSTPAT("0000001 ????? ????? 001 ????? 0110011", mulh, R, R(rd) = ((int64_t)(int32_t)src1*(int64_t)(int32_t)src2) >> 32);
  INSTPAT("0000001 ????? ????? 010 ????? 0110011", mulhsu, R, R(rd) = ((int64_t)(int32_t)src1 * (uint64_t)(uint32_t)src2) >> 32;);
  INSTPAT("0000001 ????? ????? 011 ????? 0110011", mulhu, R, R(rd) = ((uint64_t)(uint32_t)src1 * (uint64_t)(uint32_t)src2) >> 32;);//因为在右移之前， 结果就已经被丢弃了， 所以R(rd) = (src1 * src2) >> 32;不行
  INSTPAT("0000001 ????? ????? 100 ????? 0110011", div, R, {
    sword_t a = (sword_t)src1;
    sword_t b = (sword_t)src2;
    if (b == 0) {
        R(rd) = 0xffffffff; // 除以0，返回 -1
    } else if (a == 0x80000000 && b == -1) {
        R(rd) = 0x80000000; // 溢出情况，返回 INT_MIN
    } else {
        R(rd) = a / b;      // 正常计算
    }
});
  INSTPAT("0000001 ????? ????? 101 ????? 0110011", divu, R, {
    if (src2 == 0) {
        R(rd) = 0xffffffff; // 除以0，返回最大值
    } else {
        R(rd) = src1 / src2; // 正常计算
    }
});
  INSTPAT("0000001 ????? ????? 110 ????? 0110011", rem, R, {});
  INSTPAT("0000001 ????? ????? 111 ????? 0110011", remu, R, {});

  INSTPAT("0000000 00001 00000 000 00000 1110011", ebreak, N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ???????", inv, N, INV(s->pc));
  
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s)
{
  s->isa.inst = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
