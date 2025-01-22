#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* sys_malloc(size_t size) {
  void *p = pf;
  pf += ROUNDUP(size, PGSIZE);
  memset(p, 0, size);
  assert(pf < (void*) heap.end);
  // assert((unsigned int)pf%PGSIZE == 0);
  return p;
}

void* new_page(size_t nr_page) {
  pf += PGSIZE * nr_page;
  assert(pf < (void*) heap.end);
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
  current->max_brk = ROUNDUP(current->max_brk, PGSIZE);
  if(brk > current->max_brk){
    int new_page_num = ROUNDUP(brk - current->max_brk, PGSIZE)/PGSIZE;
    void* new_page_start = new_page(new_page_num);
    for(int i=0; i<new_page_num; ++i){
      map(&current->as, (void*)(current->max_brk + i*PGSIZE), new_page_start + i*PGSIZE, 0);
    }
    current->max_brk += new_page_num * PGSIZE;
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
