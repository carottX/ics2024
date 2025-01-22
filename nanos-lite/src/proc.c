#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
int now_index;

void naive_uload(PCB *pcb, const char *filename);

void context_uload(PCB* pcb, const char *filename, char* const argv[], char* const envp[]);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void switch_pcb(int id){
  if(now_index == id)return;
  switch_boot_pcb();
  now_index = id;
  pcb[0].cp->pdir = NULL;
  yield();
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* pcb, void(*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb->stack + STACK_SIZE}, entry, arg);
}

#define PAL_NAME "/bin/pal"
#define TERM_NAME "/bin/nterm"
#define BIRD_NAME "/bin/bird"
#define SLIDER_NAME "/bin/slider"

void init_proc() {
  char* const argv[] = {TERM_NAME, NULL};
  char* const envp[] = {NULL};
  char* const argv2[] = {PAL_NAME,"--skip",NULL};
  char* const envp2[] = {NULL};
  // char* const argv3[] = {BIRD_NAME, NULL};
  // char* const envp3[] = {NULL};
  char* const argv4[] = {SLIDER_NAME, NULL};
  char* const envp4[] = {NULL};
  // context_uload(&pcb[1], )
  context_kload(&pcb[0], hello_fun, "ONE");
  context_uload(&pcb[1], TERM_NAME, argv, envp);
  context_uload(&pcb[2], PAL_NAME, argv2, envp2);
  context_uload(&pcb[3], SLIDER_NAME, argv4, envp4);
  switch_boot_pcb();

  // yield();

  Log("Initializing processes...");

  // load program here
  // naive_uload(&pcb[0], "/bin/nterm");
}

Context *schedule(Context *prev) {
  if(prev == NULL) printf("prev is NULL\n");
  // printf("SWITCHING TO %d!\n", current == &pcb[0] ? 1 : 0);
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[now_index] : &pcb[0]);
  return current->cp;
}