#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>

#define TODO() assert(0);

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static uint8_t keys[1024];

void CallBackHelper();

int SDL_PushEvent(SDL_Event *ev) {
  TODO();
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  // printf("POLL BEGIN\n");
  char buf[16];
  if(NDL_PollEvent(buf, 16) == 0) return 0;
  if(strncmp(buf, "kd", 2) == 0) ev->type = SDL_KEYDOWN;
  else ev->type = SDL_KEYUP;
  for(int i = 0; i < sizeof(keyname)/sizeof(keyname[0]); ++i){
    if(strncmp(keyname[i], buf+3, strlen(buf)) == 0){
      ev -> key.keysym.sym = i;
      printf("!!!!!!%s\n",keyname[i]);
      keys[i] = ((ev->type == SDL_KEYDOWN) ? 1 : 0);
      return 1;
    }
  }
  CallBackHelper();
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  // printf("???");
  char buf[16];
  while(NDL_PollEvent(buf, 16) == 0);
  buf[15] = '\0';
  // printf("BUF=%s!\n",buf);
  if(strncmp(buf, "kd", 2) == 0) event->type = SDL_KEYDOWN;
  else event->type = SDL_KEYUP;
  for(int i = 0; i < sizeof(keyname)/sizeof(keyname[0]); ++i){
    if(strncmp(keyname[i], buf+3, strlen(buf)) == 0){
      event -> key.keysym.sym = i;
            printf("!!!!!!%s\n",keyname[i]);

      keys[i] = ((event->type == SDL_KEYDOWN) ? 1 : 0);
      return 1;
    }
  }
    CallBackHelper();

  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  TODO();
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if(numkeys != NULL) *numkeys = sizeof(keyname) / sizeof(keyname[0]);
  return keys;
}
