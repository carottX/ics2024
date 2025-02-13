#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

#define IRQ_TIMER 0x80000007

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) {
  uintptr_t mscratch;
  uintptr_t kas = 0;
  asm volatile("csrr %0, mscratch" : "=r"(mscratch));
  c->np = (mscratch == 0 ? KERNEL : USER);
  asm volatile("csrw mscratch, %0" :: "r"(kas));
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 11:case 8: ev.event = EVENT_SYSCALL; c->mepc += 4;break;
      case IRQ_TIMER: ev.event = EVENT_IRQ_TIMER; break;
      default: ev.event = EVENT_ERROR; break;
    } 
    if(ev.event == EVENT_SYSCALL && c -> GPR1 == -1) ev.event = EVENT_YIELD;
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  uintptr_t* t0 = kstack.end-4;
  *t0 = 0;
  Context* c = (Context*)(kstack.end - sizeof(Context));
  c->mepc = (intptr_t)entry;
  c->gpr[10] = (intptr_t)arg;
  c->pdir = NULL;
  c->mstatus = 0x80; // MPIE
  c->np = 3;
  c->gpr[2] = (uintptr_t)kstack.end - 4;
  return c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
