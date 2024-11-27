#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  // printf("POLL BEGIN\n");
  char buf[16];
  if(NDL_PollEvent(buf, 16) == 0) {    printf("POLL FINI\n");
return 0;}
  if(strncmp(buf, "kd", 2) == 0) ev->type = SDL_KEYDOWN;
  else ev->type = SDL_KEYUP;
  for(int i = 0; i < sizeof(keyname)/sizeof(keyname[0]); ++i){
    if(strncmp(keyname[i], buf+3, strlen(keyname[i])) == 0){
      ev -> key.keysym.sym = i;    printf("POLL FINI\n");

      return 1;
    }
  }
    printf("POLL FINI\n");

  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  // printf("???");
  char buf[16];
  while(NDL_PollEvent(buf, 16) == 0);
  // printf("BUF=%s!\n",buf);
  if(strncmp(buf, "kd", 2) == 0) event->type = SDL_KEYDOWN;
  else event->type = SDL_KEYUP;
  for(int i = 0; i < sizeof(keyname)/sizeof(keyname[0]); ++i){
    if(strncmp(keyname[i], buf+3, strlen(keyname[i])) == 0){
      event -> key.keysym.sym = i;
      return 1;
    }
  }
        // printf("???FINI\n");

  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  printf("HELLO?\n");
  size_t sz = sizeof(keyname) / sizeof(keyname[0]);
  uint8_t* ret = malloc(sizeof(uint8_t) * (numkeys == NULL ? sz : *numkeys));
  SDL_Event tmp;
  SDL_PollEvent(&tmp);
  for(int i=0; i<sz; ++i) if(tmp.key.keysym.sym == i && tmp.type == SDL_KEYDOWN) ret[i] = 1; else ret = 0;
  return ret;
}
