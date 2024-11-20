#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  // size_t p_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

#define ARRLEN(a) (sizeof(a) / sizeof(a[0]))

int fs_open(const char *pathname, int flags, int mode){
  printf("pathname=%s\n",pathname);
  int n = ARRLEN(file_table);
  for(int i=0; i<n; ++i){
    if(strcmp(file_table[i].name, pathname) == 0) return i;
  }
  panic("Cannot find file of name", printf("%s", pathname));
}

size_t fs_read(int fd, void *buf, size_t len){
  assert(file_table[fd].size >= len);
  return ramdisk_read(buf, file_table[fd].disk_offset, len);
}

size_t fs_close(int fd){
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
