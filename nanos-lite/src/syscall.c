#include <common.h>
#include "syscall.h"
#include <sys/time.h>
#include <proc.h>

// #define STRACE

extern PCB *current;

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);
void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB* pcb, const char *filename, char* const argv[], char* const envp[]);
void switch_boot_pcb();

void sys_execve(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=execve\n" );
  #endif
  context_uload(current, (char*)c->GPR2, (char**)c->GPR3, (char**)c->GPR4);
  switch_boot_pcb();
  yield();
  c->GPRx = 0;
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
  printf("exit=%d\n", c->GPR2);
  if(c->GPR2)halt(c->GPR2);
  else{
    c->GPR2 = (uintptr_t)"/bin/nterm";
    sys_execve(c);
  }
}

void sys_sbrk(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=sbrk\n" );
  #endif
  c->GPRx = 0;
}

void sys_read(Context* c){
   #ifdef STRACE
  printf("SYSCALL NAME=read %d\n", c->GPR2 );
  #endif
  c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);
}

void sys_write(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=write %d\n",c->GPR2 );
  #endif
  c->GPRx = fs_write(c->GPR2, (void *)c->GPR3, c->GPR4);
}

void sys_lseek(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=lseek\n" );
  #endif
  c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
}

void sys_open(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=open\n" );
  #endif
  c->GPRx = fs_open((char*)c->GPR2, c->GPR3, c->GPR4);
}

void sys_close(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=close\n" );
  #endif
  c->GPRx = fs_close(c->GPR2);
}

void sys_gettimeofday(Context* c){
  #ifdef STRACE
  printf("SYSCALL NAME=gettimeofday\n" );
  #endif
  struct timeval* tv = (void *)c->GPR2;
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = us/1000000;
  tv->tv_usec = us%1000000;
  c->GPRx = 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  #ifdef STRACE
  printf("SYSCALL ID=%d\n", a[0]);
  #endif
  switch (a[0]) {
    case SYS_exit: sys_exit(c); break;
    case SYS_yield: sys_yield(c); break;
    case SYS_open: sys_open(c); break;
    case SYS_read: sys_read(c); break;
    case SYS_write: sys_write(c); break;
    case SYS_close: sys_close(c); break;
    case SYS_lseek: sys_lseek(c); break;
    case SYS_execve: sys_execve(c); break;
    case SYS_gettimeofday: sys_gettimeofday(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  #ifdef STRACE
  printf("Return Value=%d\n", c->GPRx);
  #endif
}
