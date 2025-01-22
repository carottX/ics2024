#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define VPN0(addr) ((addr >> 12) & 0x3ff)
#define VPN1(addr) ((addr >> 22))

void map(AddrSpace *as, void *va, void *pa, int prot) {
  va = (void*)ROUNDDOWN((uintptr_t)va, PGSIZE);
  pa = (void*)ROUNDDOWN((uintptr_t)pa, PGSIZE);
  PTE* L1PageTable = as->ptr + VPN1((uintptr_t)va) * sizeof(PTE);
  if(*L1PageTable == 0 || (*L1PageTable & PTE_V) == 0) {
    *L1PageTable = (uintptr_t)pgalloc_usr(PGSIZE) | PTE_V;
    // printf("Created 2nd PageTable! Addr = %p, entry= %p\n", L1PageTable, *L1PageTable);
  }
  PTE* L2PageTable = (PTE*)(*L1PageTable & ~0xfff) + VPN0((uintptr_t)va) * sizeof(PTE);
  *L2PageTable = (uintptr_t)pa | PTE_V;
  if((uintptr_t)va/PGSIZE == 0x80001)
  printf("Mapped va = %p, pa = %p\n at L1TableAddr=%p, L1Entry=%p, L2TableAddr=%p, entry=%p\n", va, pa, L1PageTable, *L1PageTable, L2PageTable, *L2PageTable);
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context* c = (Context*)(kstack.end - sizeof(Context));
  c->mepc = (intptr_t)entry;
  return c;
}
