#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#include <unistd.h>

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

#include <assert.h>

#define TODO() assert(0);

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  TODO();
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
    // printf("LOADing:%s\n",filename);

  int fd = open(filename);
  if(fd == -1){
    printf("Error when opening the file!\n");
    return NULL;
  }
  int sz = lseek(fd, 0, SEEK_END);
  // printf("size=%d\n",sz);
  if(sz == -1){
    printf("Error when getting the file size!\n");
    return NULL;
  }
  lseek(fd, 0, SEEK_SET);
  void* buf = SDL_malloc(sz);
  int tmp = read(fd, buf, sz);
  // printf("Read ret = %d\n",tmp);
  if(tmp == -1) {
    printf("Error when reading the file!\n");
    return NULL;
  }
  SDL_Surface* ret = STBIMG_LoadFromMemory(buf, sz);
  // printf("Error:%s\n",SDL_GetError());
  close(fd);
  free(buf);
  printf("LOAD FINI\n");
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  TODO();
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  TODO();
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
