#include <proc.h>
#include <elf.h>
#include <fs.h>

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

#define max(a,b) ((a) > (b) ? (a) : (b))

static uintptr_t loader(PCB *pcb, const char *filename) {
  assert(pcb != NULL);
  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr elf;
  fs_read(fd, &elf, sizeof(Elf_Ehdr));
  if(elf.e_ident[EI_MAG0] != ELFMAG0 ||
     elf.e_ident[EI_MAG1] != ELFMAG1 ||
     elf.e_ident[EI_MAG2] != ELFMAG2 ||
     elf.e_ident[EI_MAG3] != ELFMAG3){
    panic("Not a valid elf file!");
    return (uintptr_t)NULL;
  }
  size_t ph_offset = elf.e_phoff;
  size_t entry_size = elf.e_phentsize;
  size_t ph_num = elf.e_phnum;
  for(int i=0; i<ph_num; ++i){
    Elf_Phdr seg_header;
    fs_lseek(fd, ph_offset+entry_size*i, 0);
    fs_read(fd, &seg_header, entry_size);
    if(seg_header.p_type != PT_LOAD) continue;
    // size_t seg_offset = seg_header.p_offset;
    // uintptr_t seg_viraddr = seg_header.p_vaddr;
    // size_t seg_file_size = seg_header.p_filesz;
    // size_t seg_mem_size = seg_header.p_memsz;
    // void* paddr = allocate(&pcb->as, seg_viraddr, seg_mem_size);
    // fs_lseek(fd, seg_offset, 0);
    // fs_read(fd, paddr + (seg_viraddr & 0xfff), seg_file_size);
    // memset(paddr + (seg_viraddr & 0xfff) + seg_file_size, 0, seg_mem_size-seg_file_size);
    // printf("Loaded segments from [%p, %p]\n",seg_viraddr, seg_viraddr + seg_mem_size);
    // printf("To physical address %p\n",paddr);
    // pcb->max_brk = ROUNDUP(seg_viraddr + seg_mem_size, PGSIZE);
    // printf("max_brk=%p\n",pcb->max_brk);
    uintptr_t vpage_start = seg_header.p_vaddr & (~0xfff); 
    uintptr_t vpage_end = (seg_header.p_vaddr + seg_header.p_memsz - 1) & (~0xfff); 
    int page_num = ((vpage_end - vpage_start)/PGSIZE) + 1;
    uintptr_t page_ptr = (uintptr_t)new_page(page_num);
    for (int j = 0; j < page_num; ++ j) {
      map(&pcb->as, 
          (void*)(vpage_start + (j*PGSIZE)), 
          (void*)(page_ptr    + (j*PGSIZE)), 
          0);
    }
    void* page_off = (void *)(seg_header.p_vaddr & 0xfff); 
    fs_lseek(fd, seg_header.p_offset, SEEK_SET);
    fs_read(fd, page_ptr + page_off, seg_header.p_filesz); 
    memset(page_ptr + page_off + seg_header.p_filesz, 0, seg_header.p_memsz - seg_header.p_filesz);
    pcb->max_brk = vpage_end + PGSIZE; 
    
  }
  fs_close(fd);
  return elf.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  // __libc_init_array();
  ((void(*)())entry) ();
  assert(0);
}

void context_uload(PCB* pcb, const char *filename, char* const argv[], char* const envp[]) {
  protect(&pcb->as);
  int argc = 0;
  while(argv[argc] != NULL) argc++;
  int envc = 0;
  while(envp[envc] != NULL) envc++;
  // printf("uload:argc=%d envc=%d\n",argc,envc);
  // for(int i=0; i<argc; ++i) printf("argv[%d]=%s\n",i,argv[i]);
  uintptr_t argv_pos[argc], envp_pos[envc];
  char* stk = (char*)new_page(8) + PGSIZE * 8 - 4;
  char* tmp = stk + 4;
  for(int i=1; i<=8; ++i){
    map(&pcb->as, pcb->as.area.end - PGSIZE * i, tmp - PGSIZE * i, 0);
  }
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
  // printf("argc=%d\n",((uintptr_t*)stk)[0]);
  // printf("argv[0]=%s\n",(char*)((uintptr_t*)stk)[1]);
  // printf("argv[1]=%s\n",(char*)((uintptr_t*)stk)[2]);
  uintptr_t entry = loader(pcb, filename);
  stk -= sizeof(uintptr_t);
  ((uintptr_t*)stk)[0] = 0;
  pcb->cp = ucontext(&pcb->as, (Area) { pcb->stack, pcb->stack + STACK_SIZE }, (void *)entry);  
  pcb->cp->GPRx = (uintptr_t)stk - (uintptr_t)tmp + (uintptr_t)pcb->as.area.end + 4;
  pcb->cp->gpr[2] = (uintptr_t)stk - (uintptr_t)tmp + (uintptr_t)pcb->as.area.end;
}