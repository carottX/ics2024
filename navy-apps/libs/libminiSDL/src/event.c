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
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[16];
  while(NDL_PollEvent(buf, 16) == 0);
  if(strncmp(buf, "kd", 2) == 0) event->type = SDL_KEYDOWN;
  else event->type = SDL_KEYUP;
  for(int i = 0; i < sizeof(keyname)/sizeof(keyname[0]); ++i){
    if(strncmp(keyname[i], buf+3, strlne(keyname[i])) == 0){
      event -> key.keysym.sym = i;
      return 1;
    }
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
