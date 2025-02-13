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

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
	for(int idx = 0; idx < 32; ++idx){
		printf("%-20s%-20x%-20u\n", reg_name(idx), gpr(idx), gpr(idx)); 
	}
  printf("%-20s%-20x%-20u\n", "pc", cpu.pc, cpu.pc);
  printf("%-20s%-20x%-20u\n", "mepc", cpu.mepc, cpu.mepc); 
  printf("%-20s%-20x%-20u\n", "mstatus", cpu.mstatus, cpu.mstatus); 
  printf("%-20s%-20x%-20u\n", "mcause", cpu.mcause, cpu.mcause); 

}

word_t isa_reg_str2val(const char *s, bool *success) {
  for(int idx = 0; idx < 32; ++idx){
    if(strcmp(reg_name(idx), s) == 0) {
      if(success!=NULL)*success = true;
      return gpr(idx);
    }
  }
  if(strcmp("pc", s) == 0){
    if(success!=NULL)*success = true;
    return cpu.pc;
  }
  if(strcmp("mepc", s) == 0){
    if(success!=NULL)*success = true;
    return cpu.mepc;
  }
  if(strcmp("mstatus", s) == 0){
    if(success!=NULL)*success = true;
    return cpu.mstatus;
  }
  if(strcmp("mcause", s) == 0){
    if(success!=NULL)*success = true;
    return cpu.mcause;
  }
  if(success!=NULL)*success = false;
  return 0;
}
