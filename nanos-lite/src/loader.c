#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

void* sys_malloc(size_t size);

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

size_t GetFileSize(int fd);
size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  void* file = sys_malloc(GetFileSize(fd));
  Elf_Ehdr* elf = sys_malloc(sizeof(Elf_Ehdr));
  fs_read(fd, file, GetFileSize(fd));
  memcpy(elf, file, sizeof(Elf_Ehdr));

  if(elf->e_ident[EI_MAG0] != ELFMAG0 ||
     elf->e_ident[EI_MAG1] != ELFMAG1 ||
     elf->e_ident[EI_MAG2] != ELFMAG2 ||
     elf->e_ident[EI_MAG3] != ELFMAG3){
    panic("Not a valid elf file!");
    return (uintptr_t)NULL;
  }
  size_t ph_offset = elf->e_phoff;
  size_t entry_size = elf->e_phentsize;
  size_t ph_num = elf->e_phnum;
  for(int i=0; i<ph_num; ++i){
    Elf_Phdr *seg_header = sys_malloc(sizeof(Elf_Phdr));
    memcpy(seg_header, file+ph_offset+entry_size*i, sizeof(Elf_Phdr));
    if(seg_header->p_type != PT_LOAD) continue;
    // printf("LOADED!\n");
    size_t seg_offset = seg_header->p_offset;
    size_t seg_viraddr = seg_header->p_vaddr;
    size_t seg_file_size = seg_header->p_filesz;
    size_t seg_mem_size = seg_header->p_memsz;
    memcpy((void*)seg_viraddr, file+seg_offset,seg_mem_size);
    memset((void*)seg_viraddr+seg_file_size, 0, seg_mem_size-seg_file_size);
  }
  // printf("!!!\n");
  fs_close(fd);
  return elf->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  // printf("0x1234=%x\n",0x1234);
  // printf("entry=%u\n",entry);
  Log("Jump to entry = %p", entry);
  // __libc_init_array();
  ((void(*)())entry) ();
  assert(0);
}

void context_uload(PCB* pcb, const char *filename, char* const argv[], char* const envp[]) {
  // protect(&pcb->as);
  int argc = 0;
  while(argv[argc] != NULL) argc++;
  int envc = 0;
  while(envp[envc] != NULL) envc++;
  printf("uload:argc=%d envc=%d\n",argc,envc);
  for(int i=0; i<argc; ++i) printf("argv[%d]=%s\n",i,argv[i]);
  uintptr_t argv_pos[argc], envp_pos[envc];
  char* stk = (char*)new_page(8) + PGSIZE * 8;
  // printf("stk=%p\n",stk);
  for(int i = 0; i<argc; i++){
    int len = strlen(argv[i]) + 1;
    stk -= len;
    argv_pos[i] = (uintptr_t)stk;
    strncpy(stk, argv[i], len);
  }
  stk = (char*)ROUNDDOWN((uintptr_t)stk, sizeof(uintptr_t));
  for(int i = 0; i<envc; i++){
    int len = strlen(envp[i]) + 1;
    stk -= len;
    envp_pos[i] = (uintptr_t)stk;
    strncpy(stk, envp[i], len);
  }
  // printf("stk=%p\n",stk);
  stk = (char*)ROUNDDOWN((uintptr_t)stk, sizeof(uintptr_t));
  stk -= sizeof(uintptr_t)*(argc + envc + 3);
  ((uintptr_t*)stk)[0] = argc;
  for(int i=0; i<argc; ++i){
    ((uintptr_t*)stk)[i+1] = (uintptr_t)argv_pos[i];
  }
  ((uintptr_t*)stk)[argc + 1] = 0;
  for(int i=0; i<envc; ++i){
    ((uintptr_t*)stk)[argc + 2 + i] = (uintptr_t)envp_pos[i];
  }
  ((uintptr_t*)stk)[argc + envc + 2] = 0;
  printf("argc=%d\n",((uintptr_t*)stk)[0]);
  printf("argv[0]=%s\n",(char*)((uintptr_t*)stk)[1]);
  printf("argv[1]=%s\n",(char*)((uintptr_t*)stk)[2]);
  uintptr_t entry = loader(pcb, filename);
  pcb->cp = ucontext(&pcb->as, (Area) { pcb->stack, pcb->stack + STACK_SIZE }, (void *)entry);  
  pcb->cp->GPRx = (uintptr_t)stk;
  // printf("%d\n",((uintptr_t*)stk)[0]);
}