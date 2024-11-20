#include <common.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  #ifdef STRACE
  printf("SYSCALL ID=%d\n", a[0]);
  #endif
  switch (a[0]) {
    case 0: sys_exit(c); break;
    case 1: sys_yield(c); break;
    case 4: sys_write(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  #ifdef STRACE
  printf("Return Value=%d\n", c->GPRx);
  #endif
}

void sys_yield(Context *c){
  #ifdef STRACE
  printf("SYSCALL NAME=yield\n" );
  #endif
  yield();
  c -> GPRx = 0;
}

void sys_exit(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=exit\n" );
  #endif
  halt(c->GPR2);
}

void sys_write(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=write\n" );
  #endif
  int fd = c->GPR2;
  if(fd == 1 || fd == 2) {
    uint8_t* buf = (uint8_t*)c->GPR3;
    int len = c->GPR4;
    for(int i=0; i<len; ++i) putch(*(buf+i));
    c->GPRx=len;
  }
  else {
    c->GPRx = -1;
  }
}