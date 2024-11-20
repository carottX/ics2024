#include <common.h>
#include "syscall.h"

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

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
  // printf("exit=%d\n", c->GPR2);
  halt(c->GPR2);
}

// void sys_write(Context* c){
//   #ifdef STRACE
//   printf("SYSCALL NAME=write\n" );
//   #endif
//   int fd = c->GPR2;
//   // printf("fd = %d len = %d\n", fd, c->GPR4);
//   if(fd == 1 || fd == 2) {
//     uint8_t* buf = (uint8_t*)c->GPR3;
//     int len = c->GPR4;
//     for(int i=0; i<len; ++i) putch(*(buf+i));
//     c->GPRx=len;
//   }
//   else {
//     c->GPRx = -1;
//   }
// }

void sys_sbrk(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=sbrk\n" );
  #endif
  c->GPRx = 0;
}

void sys_read(Context* c){
  c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);
}

void sys_write(Context* c){
  c->GPRx = fs_write(c->GPR2, (void *)c->GPR3, c->GPR4);
}

void sys_lseek(Context* c){
  c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
}

void sys_open(Context* c){
  c->GPRx = fs_open((void*)c->GPR2, c->GPR3, c->GPR4);
}

void sys_close(Context* c){
  c->GPRx = fs_close(c->GPR2);
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  #ifdef STRACE
  printf("SYSCALL ID=%d\n", a[0]);
  #endif
  switch (a[0]) {
    case 0: sys_exit(c); break;
    case 1: sys_yield(c); break;
    case 2: sys_open(c); break;
    case 3: sys_read(c); break;
    case 4: sys_write(c); break;
    case 7: sys_close(c); break;
    case 8: sys_lseek(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  #ifdef STRACE
  printf("Return Value=%d\n", c->GPRx);
  #endif
}
