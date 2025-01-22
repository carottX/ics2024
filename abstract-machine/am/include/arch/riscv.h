#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

enum PRIVILEGE {
  KERNEL = 0,
  USER = 1,
};

struct Context {
  // TODO: fix the order of these members to match trap.S
  uintptr_t gpr[NR_REGS];
  uintptr_t mcause, mstatus, mepc;
  void *pdir;
  int np;
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

#endif
