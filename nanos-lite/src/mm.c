#include <memory.h>

static void *pf = NULL;

void* sys_malloc(size_t size) {
  void *p = pf;
  pf += ROUNDUP(size, PGSIZE);
  memset(p, 0, size);
  assert((unsigned int)pf%PGSIZE == 0);
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
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
