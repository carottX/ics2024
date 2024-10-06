#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;  
  printf("%d %d\n",w,h);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);  
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  // printf("?");
  int width = inl(VGACTL_ADDR)>>16;
  int height = inl(VGACTL_ADDR)&0xffff;
  // printf("%d %d\n",width, height);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // if (w == 0 || h == 0) return;
  if (ctl->sync) {  
    outl(SYNC_ADDR, 1);
  }
  int hh = io_read(AM_GPU_CONFIG).height;  
  uint32_t* px = ctl->pixels;
  for(int i=0; i<ctl->w; ++i){
    for(int j=0; j<ctl->h; ++j){
      outl((uintptr_t)FB_ADDR+hh*(ctl->x+i)+j, px[i*ctl->h+j]);
    }
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
