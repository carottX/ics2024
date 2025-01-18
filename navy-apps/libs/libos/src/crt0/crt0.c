#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void __libc_init_array (void);
void call_main(uintptr_t *args) {
  int argc = (int*)args[0];
  char** argv = (int*)args[1];
  char** envp = (int*)args[argc+2];
  printf("argc=%d\n",argc);
  for(int i=0; i<argc; ++i){
    printf("argv[%d]=%s\n",i,argv[i]);
  }
  environ = envp;
  __libc_init_array();
  exit(main(argc, argv, envp));
  assert(0);
}
