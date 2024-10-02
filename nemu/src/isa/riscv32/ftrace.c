#include <elf.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <cpu/ftrace.h>

static Symbol* sym = NULL;
static int indent = 1;
static int sym_l = 0;

void trace_func_call(uint32_t pc, uint32_t target){
    for(int i=0; i<sym_l; ++i){
        if(sym[i].addr == target){
            printf("0x%x:", pc);
            for(int j=0; j<indent*2; ++j) printf(" ");
            ++indent;
            printf("call [%s@0x%x]\n", sym[i].name, sym[i].addr);
            return;
        }
    }
    printf("[WARNING] Target func addr %u not found at pc=%u\n", target, pc);
}

void trace_func_ret(uint32_t pc, uint32_t target){
    for(int i=0; i<sym_l; ++i){
        if(sym[i].addr == target){
            printf("0x%x:", pc);
            for(int j=0; j<indent*2; ++j) printf(" ");
            --indent;
            printf("ret [%s]\n", sym[i].name);
            return;
        }
    }
    printf("[WARNING] Ret addr %x not found at pc=%x\n", target, pc);
}

Symbol* get_sym(const char* filep){
    Symbol *symbol = NULL;
    FILE* fp = fopen(filep,"rb");
    assert(fp);
    Elf32_Ehdr header;
    if(fread(&header, sizeof(Elf32_Ehdr),1, fp) <= 0) {
        printf("Failed to read the elf head!\n");
        return NULL;
    }
    if(header.e_ident[0] != 0x7f || header.e_ident[1] != 'E' || header.e_ident[2] !='L' || header.e_ident[3] != 'F'){
        printf("The file isn't a elf file!");
        return NULL;
    }
    Elf32_Shdr shdr;
    fseek(fp, header.e_shoff, SEEK_SET);
    char* stable = NULL;
    for(int i=0; i<header.e_shnum; ++i){
        if(fread(&shdr, sizeof(Elf32_Shdr), 1, fp) <= 0){
            printf("Failed to read the shdr!");
            return NULL;
        }
        if(shdr.sh_type == SHT_STRTAB){
            fseek(fp, shdr.sh_offset, SEEK_SET);
            stable = malloc(shdr.sh_size);
            if(fread(stable, shdr.sh_size, 1, fp) <= 0){
                printf("Failed to read string table!\n");
                return NULL;
            }
        }
    }
    fseek(fp, header.e_shoff, SEEK_SET);
    int func_num = 0;
    for(int i=0; i<header.e_shnum; ++i){
        if(fread(&shdr, sizeof(Elf32_Shdr), 1, fp) <= 0){
            printf("Failed to read the shdr!");
            return NULL;
        }
        if(shdr.sh_type == SHT_SYMTAB){
            fseek(fp, shdr.sh_offset, SEEK_SET);
            Elf32_Sym sym;
            size_t symcnt = shdr.sh_size / shdr.sh_entsize;
            symbol = malloc(sizeof(Symbol) * symcnt);
            for(size_t j = 0; j<symcnt; ++j){
                if(fread(&sym, sizeof(Elf32_Sym), 1, fp) <= 0){
                    printf("Failed to read symbol!\n");
                    return NULL;
                }
                if(ELF32_ST_TYPE(sym.st_info) == STT_FUNC){
                    const char* name = stable+sym.st_name;
                    strncpy(symbol[func_num].name, name, sizeof(symbol[func_num].name)-1);
                    symbol[func_num].addr = sym.st_value;
                    symbol[func_num].size = sym.st_size;
                    // printf("%s 0x%x %lu\n", symbol[func_num].name, symbol[func_num].addr, symbol[func_num].size);
                    func_num++;
                }
            }
            
        }
    }
    fclose(fp);
    free(stable);
    sym_l = func_num;
    return symbol;
}

void init_ftrace(const char* filep){
    sym = get_sym(filep);
    assert(sym!=NULL);
}

