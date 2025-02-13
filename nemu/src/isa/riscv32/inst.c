/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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
#include <cpu/ftrace.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write
#define MSTATUS_MIE 0x8
#define MSTATUS_MPIE 0x80

void trace_func_call(uint32_t pc, uint32_t target);
void trace_func_ret(uint32_t pc, uint32_t target);


enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_R,
  TYPE_N, TYPE_B,// none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = SEXT(((BITS(i, 31, 25) << 5) | BITS(i, 11, 7)), 12); } while(0)
#define immJ() do { uint32_t ___x = ((((BITS(i,31,31) << 8) | BITS(i,19, 12))<<1) | BITS(i, 20, 20)); \
*imm = SEXT(((___x<<10) | BITS(i,30,21)) << 1, 21);} while(0)
#define immB() do { uint32_t ___x = ((BITS(i, 31, 31) << 1) | BITS(i, 7, 7));\
*imm = SEXT((((((___x<<6) | BITS(i,30,25)) << 4) | BITS(i,11,8))<<1), 13);} while(0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J: src1R();          immJ(); break;
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_B: src1R(); src2R(); immB(); break;
  }
}

vaddr_t* read_csr(int imm){
  if(imm == 0x300) return &cpu.mstatus;
  else if(imm == 0x341) return &cpu.mepc; 
  else if(imm == 0x342) return &cpu.mcause;
  else if(imm == 0x305) return &cpu.mtvec;
  else if(imm == 0x180) return &cpu.satp;
  else if(imm == 0x340) return &cpu.mscratch;
  else assert(0);
}

#define mret() do { \
  cpu.mstatus = (cpu.mstatus & ~MSTATUS_MIE) | ((cpu.mstatus & MSTATUS_MPIE) >> 4); \
  cpu.mstatus = (cpu.mstatus & ~MSTATUS_MPIE) | MSTATUS_MPIE; \
  s->dnpc = cpu.mepc; \
} while(0)

enum{
  EtraceR, EtraceW, EtraceRet, EtraceCall 
}EtraceType;

void PrintEtrace(int type, int imm, int src1){
  static const char *type_name[] = {"csrrw", "csrrs", "ecall", "mret"};
  static const char *csr_name[] = {"mstatus","mepc ","mcause","mtvec", "satp", "mscratch"};
  int csr_id = 0;
  if(imm == 0x300) csr_id = 0;
  else if(imm == 0x341) csr_id = 1;
  else if(imm == 0x342) csr_id = 2;
  else if(imm == 0x305) csr_id = 3;
  else if(imm == 0x180) csr_id = 4;
  else if(imm == 0x340) csr_id = 5;
  else csr_id = 6;
  printf("Name\t\t\t | CSR\t\t\t | rs1\t\t\n%s\t\t\t | %s\t\t | %d\n",type_name[type],csr_name[csr_id],(type == EtraceRet || type == EtraceCall ? -1 : src1));
}

#ifdef CONFIG_ETRACE
#define etrw(x) PrintEtrace(x, imm, src1)
#define etcr(x) PrintEtrace(x, imm, -1)
#else
#define etrw(x) 
#define etcr(x) 
#endif

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
  
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(rd) = s->pc + 4; s->dnpc = s->pc+imm; IFDEF(CONFIG_FTRACE,if(rd!=0)trace_func_call(s->pc, s->dnpc)));
  
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, s->dnpc = src1 + ((imm>>1)<<1); R(rd) = s->pc+4; IFDEF(CONFIG_FTRACE,if(s->isa.inst.val == 0x00008067)trace_func_ret(s->pc, s->dnpc); else if(rd == 1)trace_func_call(s->pc, s->dnpc); else if(rd == 0 && imm == 0)trace_func_call(s->pc, s->dnpc);));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, R(rd) = ((int32_t) src1) >> (imm & 31));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = (uint32_t)src1 >> (imm & 31));
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = src1 << (imm & 31));

  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);

  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = (src1 + imm));
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = (((int32_t)src1)<((int32_t)imm)?1:0)); 

  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = (((uint32_t)src1)<((uint32_t)imm)?1:0)); 
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT(Mr(src1 + imm, 2),16));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = SEXT(Mr(src1 + imm, 1),8));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2));


  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1 << (src2 & 31));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = (src1 < src2));  
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = (uint32_t)src1 >> (src2 & 31));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = (int32_t)src1 >> (src2 & 31));


  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt   ,  R, R(rd) = ((int32_t)src1 < (int32_t)src2));
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2);
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = (int32_t)src1 * (int32_t)src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(rd) = (((int64_t)(int32_t)src1 * (int64_t)(int32_t)src2) >> 32));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, R(rd) = (((uint64_t)src1 * (uint64_t)src2) >> 32));
  
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = (src2?(int64_t)(int32_t)src1 / (int32_t)src2:-1));
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = (src2?(uint32_t)src1 / (uint32_t)src2:4294967295));
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = (src2?(int64_t)(int32_t)src1 % (int32_t)src2:src1));
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = (src2?(uint32_t)src1 % (uint32_t)src2:src1));


  INSTPAT("??????? ????? ????? 000 ????? 11000 11", BEQ    , B, s->dnpc=((src1 == src2) ? (s->pc + imm) : s->dnpc));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", BGE    , B, s->dnpc=(((int32_t)src1 >= (int32_t)src2) ? (s->pc + imm) : s->dnpc));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", BGEU   , B, s->dnpc=(((uint32_t)src1 >= (uint32_t)src2) ? (s->pc + imm) : s->dnpc));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", BNE    , B, s->dnpc=((src1 != src2) ? (s->pc + imm) : s->dnpc));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", BLT    , B, s->dnpc=(((int32_t)src1 < (int32_t)src2) ? (s->pc + imm) : s->dnpc));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, s->dnpc=(((uint32_t)src1 < (uint32_t)src2) ? (s->pc + imm) : s->dnpc));

  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I, if(rd) R(rd) = *read_csr(imm); *read_csr(imm) = src1; etrw(EtraceW););
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , I, if(rd) R(rd) = *read_csr(imm); (*read_csr(imm)) |= src1; etrw(EtraceR););
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , N, s->dnpc=isa_raise_intr(8,s->pc); etcr(EtraceCall);); // R(10) is $a0
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , I, mret(); etcr(EtraceRet););

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));

  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  #ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
  void add_inst(const char* s);
  add_inst(s->logbuf);
  #endif
  return decode_exec(s);
}
