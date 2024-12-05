#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdlib.h>

static SDL_AudioSpec spec;

static uint32_t last_callback = 0;

static bool paused = false;

void CallBackHelper() {
  if(spec.callback == NULL || paused) return;
  printf("samples=%d\n",spec.samples);
  uint32_t interval = spec.samples*1000;
  if((NDL_GetTicks() - last_callback) * spec.freq >= interval ) {
    printf("CALLBACK\n");
    int sz = (spec.format == AUDIO_S16SYS) ? sizeof(uint16_t) : sizeof(uint32_t);
        printf("CALLBACK sz=%d\n",sz);

    void* stream = malloc(spec.samples * spec.channels * sz);
        printf("CALLBACK\n");

    spec.callback(NULL, stream, spec.samples * spec.channels * sz); 

    NDL_PlayAudio(stream, spec.samples * spec.channels * sz);
    free(stream);
    printf("CALLBACK\n");
    last_callback = NDL_GetTicks();
  }
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
  printf("OPENED\n");
  spec = *desired;
  if(obtained != NULL) {
    *obtained = *desired;
  }
  printf("BeforeHelper\n");
  last_callback = 0;
  // CallBackHelper();
  return 0;
}

void SDL_CloseAudio() {
  memset(&spec, 0, sizeof(spec));
  last_callback = 0;
}

void SDL_PauseAudio(int pause_on) {
  if(pause_on) paused = true;
  else paused = false;
  CallBackHelper();
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
}

void SDL_LockAudio() {
}

void SDL_UnlockAudio() {
}
