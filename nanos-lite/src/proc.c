#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);

void context_uload(PCB* pcb, const char *filename, char* const argv[], char* const envp[]);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    // if(j%100 == 0)Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* pcb, void(*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb->stack + STACK_SIZE}, entry, arg);
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)0);
  char* const argv[] = {"/bin/busybox", NULL};
  char* const envp[] = {NULL};
  context_uload(&pcb[1], "/bin/busybox", argv, envp);
  switch_boot_pcb();

  yield();

  Log("Initializing processes...");

  // load program here
  naive_uload(NULL, "/bin/nterm");
}

Context *schedule(Context *prev) {
  if(prev == NULL) printf("prev is NULL\n");
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}