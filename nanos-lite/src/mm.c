#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* sys_malloc(size_t size) {
  void *p = pf;
  pf += ROUNDUP(size, PGSIZE);
  memset(p, 0, size);
  // assert((unsigned int)pf%PGSIZE == 0);
  return p;
}

void* new_page(size_t nr_page) {
  pf += PGSIZE * nr_page;
  return pf - PGSIZE * nr_page;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  return sys_malloc(n);
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  uintptr_t brk_pn = brk / PGSIZE;
  // printf("!!!!!!!!!!!!!!!1brk: %x\n", brk);
  assert(current->max_brk % PGSIZE == 0);
  if(brk >= current->max_brk){
    for(uintptr_t i = (current->max_brk-1)/PGSIZE+1; i<=brk_pn; ++i){
      map(&current->as, (void*)(i*PGSIZE), new_page(1), 0);
    }
    current->max_brk = (brk_pn+1) * PGSIZE;
    printf("brk: %x\n", brk);
  }
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
