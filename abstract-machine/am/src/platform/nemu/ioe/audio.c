#include <am.h>
#include <nemu.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_START_ADDR     (AUDIO_ADDR + 0x18)

static int try_write(uint8_t* stream, uint32_t len){
  uint32_t wlen = len, i;
  uint32_t count = inl(AUDIO_COUNT_ADDR);
  uint32_t size = inl(AUDIO_SBUF_SIZE_ADDR);
  uint32_t first = inl(AUDIO_START_ADDR);
  uint32_t start = AUDIO_SBUF_ADDR;
  if(count+wlen > size) wlen = size-count;
  if(wlen == 0) return 0;
  for(i=0; i<wlen; ++i){
    outb((first-start+i)%size + start, stream[i]);
  }
  outb(AUDIO_COUNT_ADDR, count+wlen);
  return wlen;
}

static void audio_write(uint8_t* stream, int len){
  int b = 0;
  while(b < len){
    b+=try_write(stream+b, len-b);
  }
}

void __am_audio_init() {
  outl(AUDIO_INIT_ADDR,1);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int len = ctl->buf.end - ctl->buf.start;
  audio_write(ctl->buf.end, len);
}
