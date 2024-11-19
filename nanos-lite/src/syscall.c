#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  // printf("SYSCALL ID=%d\n", a[0]);
  switch (a[0]) {
    case 0: sys_exit(c); break;
    case 1: sys_yield(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_yield(Context *c){
  yield();
  c -> GPRx = 0;
}

void sys_exit(Context* c){
  halt(c->GPR1);
}