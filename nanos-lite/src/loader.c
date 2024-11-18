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
  Elf_Ehdr* elf = malloc(sizeof(Elf_Ehdr));
  ramdisk_read(elf, 0, sizeof(Elf_Ehdr));

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
    Elf_Phdr *seg_header = malloc(sizeof(Elf_Phdr));
    ramdisk_read(seg_header, ph_offset + i*entry_size, entry_size);
    if(seg_header->p_type != PT_LOAD) continue;
    printf("LOADED!\n");
    size_t seg_offset = seg_header->p_offset;
    size_t seg_viraddr = seg_header->p_vaddr;
    size_t seg_file_size = seg_header->p_filesz;
    size_t seg_mem_size = seg_header->p_memsz;
    ramdisk_read((void *)seg_viraddr, seg_offset, seg_mem_size);
    memset((void*)seg_viraddr+seg_file_size, 0, seg_mem_size-seg_file_size);
  }
  // printf("!!!\n");
  return elf->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  // printf("0x1234=%x\n",0x1234);
  // printf("entry=%u\n",entry);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

