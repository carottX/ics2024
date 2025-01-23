#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  return 1;
}

uint32_t SDL_GetTicks() {
  // printf("HELLO?")
  return NDL_GetTicks();
}

void SDL_Delay(uint32_t ms) {
  if(ms < 10) return;
  uint32_t t = ms + SDL_GetTicks();
  while(SDL_GetTicks() < t);
}
