/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  reg_start,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static void audio_read(uint8_t *stream, int len){
  memset(stream,0,len);
  uint32_t count = audio_base[reg_count];
  int rlen = len;
  if(count < rlen) rlen = count;
  uint32_t size = audio_base[reg_sbuf_size];
  uint32_t writep = audio_base[reg_start];
  uint32_t cnt_t = size-writep;
  if(cnt_t >= rlen) {
    // SDL_MixAudio(stream, sbuf+writep, rlen, SDL_MIX_MAXVOLUME);
    memcpy(stream, sbuf+writep, rlen);
  }
  else {
    // printf("len=%d count=%d size=%d writep=%d cnt_t=%d\n",len, count,size,writep,cnt_t);
    // SDL_MixAudio(stream, sbuf+writep, cnt_t, SDL_MIX_MAXVOLUME);
    memcpy(stream, sbuf+writep, cnt_t);
    // SDL_MixAudio(stream+cnt_t, sbuf, rlen-cnt_t, SDL_MIX_MAXVOLUME);
    memcpy(stream+cnt_t, sbuf, rlen-cnt_t);
  }
  audio_base[reg_start] += rlen;
  if(audio_base[reg_start]>size)audio_base[reg_start] -= size;
  audio_base[reg_count] -= rlen;
  // if(len > rlen){
  //   memset(stream+rlen, 0, len-rlen);
  // }
}

static void audio_play(void *userdata, uint8_t *stream, int len){
  audio_read(stream, len);
  // printf("nread=%d len=%d\n",nread,len);
  // printf("start=%d count=%d\n",audio_base[reg_start],audio_base[reg_count]);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  assert(offset%4==0);
  // printf("%d\n", audio_base[reg_count]);
  // printf("0x%x %d %d\n",offset, len, is_write);
  switch(offset/4){
    case reg_freq:
    case reg_channels:
    case reg_samples:
    case reg_sbuf_size:
    break;
    case reg_start:
    // printf("Changed start!\n");
    break;
    case reg_count:
    break;
    case reg_init:
    if(audio_base[reg_init] == 1){
    SDL_AudioSpec s = {};
    s.freq = audio_base[reg_freq];
    s.format = AUDIO_S16SYS;
    s.channels = audio_base[reg_channels];
    s.samples = audio_base[reg_samples];
    s.callback = audio_play;
    s.userdata = NULL;
    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (ret == 0) {
      SDL_OpenAudio(&s, NULL);
      SDL_PauseAudio(0);
    }
    // audio_base[reg_count] = 0;
    audio_base[reg_init] = 0;}
    break;
    default:
    printf("[Error]Unknown audio register:%d\n", offset/4);
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
  audio_base[reg_count] = 0;
  audio_base[reg_start] = 0;
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
