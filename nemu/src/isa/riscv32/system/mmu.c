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
#include <memory/vaddr.h>
#include <memory/paddr.h>

typedef uint32_t PTE;

#define PPN(addr) (addr & 0x3FFFFF)
#define VPN0(addr) ((addr >> 12) & 0x3ff)
#define VPN1(addr) ((addr >> 22))
#define PTE_V 0x1
#define PTE_A 0x40
#define PTE_D 0x80

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  paddr_t L1PageTable = (PPN(cpu.satp) << 12) + VPN1(vaddr) * sizeof(PTE);
  PTE L2PageTable = paddr_read(L1PageTable, 4);
  Assert(L2PageTable & PTE_V, "vaddr = %x, L1PageTable = %x, L1Entry = %x", vaddr, L1PageTable, L2PageTable);
  L2PageTable = (L2PageTable & ~0xfff) + VPN0(vaddr) * sizeof(PTE);
  PTE L2Entry = paddr_read(L2PageTable, 4);
  Assert(L2Entry & PTE_V, "vaddr = %x, L1PageTable = %x, L2PageTable = %x, L2Entry = %x", vaddr, L1PageTable, L2PageTable, L2Entry);
  paddr_t pa = (L2Entry & ~0xfff) | (vaddr & 0xfff);
  Assert(vaddr == pa , "Now only support identical map,vaddr = %x, pa = %x", vaddr, pa);
  return pa;
}
