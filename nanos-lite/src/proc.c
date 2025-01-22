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
    if(j%100 == 0)Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* pcb, void(*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb->stack + STACK_SIZE}, entry, arg);
}

#define PAL_NAME "/bin/pal"

void init_proc() {
  char* const argv[] = {PAL_NAME, NULL};
  char* const envp[] = {NULL};
  char* const argv2[] = {"/bin/hello", NULL};
  char* const envp2[] = {NULL};
  context_uload(&pcb[1], "/bin/hello", argv2, envp2);
  // context_uload(&pcb[1], PAL_NAME, argv, envp);
  context_uload(&pcb[0], PAL_NAME, argv, envp);
  // context_kload(&pcb[1], hello_fun, "ONE");
  switch_boot_pcb();

  yield();

  Log("Initializing processes...");

  // load program here
  // naive_uload(&pcb[0], "/bin/nterm");
}

Context *schedule(Context *prev) {
  if(prev == NULL) printf("prev is NULL\n");
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}