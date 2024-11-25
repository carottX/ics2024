#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#include <unistd.h>

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  int fd = open(filename);
  if(fd == -1){
    printf("Error when opening the file!\n");
    return NULL;
  }
  int sz = lseek(fd, 0, SEEK_END);
  if(sz == -1){
    printf("Error when getting the file size!\n");
    return NULL;
  }
  void* buf = SDL_malloc(sz);
  if(read(fd, buf, sz) == -1) {
    printf("Error when reading the file!\n");
    return NULL;
  }
  SDL_Surface* ret = STBIMG_LoadFromMemory(buf, sz);
  close(fd);
  free(buf);
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
