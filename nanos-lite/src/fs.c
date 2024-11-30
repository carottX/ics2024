#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t sb_write(const void *buf, size_t offset, size_t len);
size_t sbctl_write(const void *buf, size_t offset, size_t len);
size_t sbctl_read(void *buf, size_t offset, size_t len);


int min(int x, int y){if(x<y) return x; return y;}

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t p_offset;
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
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [3]         = {"/dev/events",0,0, events_read, invalid_write},
  [4]         = {"/proc/dispinfo",0,0, dispinfo_read, invalid_write},
  [5]         = {"/dev/fb",0,0,invalid_read,fb_write},
  [6]         = {"/dev/sb",0,0,invalid_read,sb_write},
  [7]         = {"/dev/sbctl",0,0,sbctl_read,sbctl_write},
#include "files.h"
};

#define ARRLEN(a) (sizeof(a) / sizeof(a[0]))

int fs_open(const char *pathname, int flags, int mode){
  // printf("pathname=%s\n",pathname);
  int n = ARRLEN(file_table);
  for(int i=0; i<n; ++i){
    if(strcmp(file_table[i].name, pathname) == 0) {
      printf("%s size=%d\n",pathname, file_table[i].size);
      file_table[i].p_offset = 0;
      return i;
    }
  }
  panic("Cannot find file of name", printf("%s", pathname));
  return 114514;
}

uint32_t fs_read(int fd, void *buf, size_t len){
  // printf("FSREAD fd=%d\n",fd);
  ReadFn ReadFunc = ramdisk_read;
  if(file_table[fd].read != NULL) ReadFunc = file_table[fd].read;
  else len = min(len, file_table[fd].size - file_table[fd].p_offset);
  size_t ret = ReadFunc(buf, file_table[fd].disk_offset + file_table[fd].p_offset, len);
  file_table[fd].p_offset += ret;
  return ret;
}

uint32_t fs_write(int fd, const void *buf, size_t len){
  // if(fd == FD_STDOUT) panic("?");
  WriteFn WriteFunc = ramdisk_write;
  if(file_table[fd].write != NULL){ WriteFunc = file_table[fd].write;}
  else len = min(len, file_table[fd].size - file_table[fd].p_offset);
  size_t ret = WriteFunc(buf, file_table[fd].disk_offset + file_table[fd].p_offset, len);
  file_table[fd].p_offset += ret;
  return ret;
}

uint32_t fs_lseek(int fd,int offset, int whence){
  printf("lseek fd=%d offset=%d whence=%d\n",fd,offset,whence);
  printf("name=%s poffset = %d\n", file_table[fd].name,file_table[fd].p_offset);
  if(whence == SEEK_CUR) file_table[fd].p_offset += offset;
  if(whence == SEEK_END) file_table[fd].p_offset = file_table[fd].size + offset;
  if(whence == SEEK_SET) file_table[fd].p_offset = offset;
  return file_table[fd].p_offset;
}

uint32_t fs_close(int fd){
  file_table[fd].p_offset = 0;
  return 0;
}

size_t GetFileSize(int fd){
  return file_table[fd].size;
}

void init_fs() {
  AM_GPU_CONFIG_T ev = io_read(AM_GPU_CONFIG);
  int width = ev.width;
  int height = ev.height;
  file_table[5].size = width * height * sizeof(uint32_t);
}
