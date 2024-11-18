#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t get_ramdisk_size();
size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  size_t RamSize = get_ramdisk_size();
  void* file = malloc(RamSize);
  ramdisk_read(file, 0, RamSize);
  Elf_Ehdr* elf = file;
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
    Elf_Phdr *seg_header = (Elf_Phdr *)((uint8_t *)elf + ph_offset + i * entry_size);
    size_t seg_offset = seg_header->p_offset;
    size_t seg_viraddr = seg_header->p_vaddr;
    size_t seg_file_size = seg_header->p_filesz;
    size_t seg_mem_size = seg_header->p_memsz;
    ramdisk_read(heap.start + seg_viraddr, seg_offset, seg_mem_size);
    for(int j=seg_file_size; j<seg_mem_size; ++j){
      *(uint8_t*)(heap.start + seg_viraddr + j) = 0;
    }
  }
  printf("!!!\n");
  free(file);
  return 0;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

