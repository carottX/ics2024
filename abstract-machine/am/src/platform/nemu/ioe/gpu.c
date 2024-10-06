#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  int w = io_read(AM_GPU_CONFIG).width/32;  
  int h = io_read(AM_GPU_CONFIG).height/32;  
  printf("%d %d\n",w,h);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);  
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  // printf("?");
  int width = inl(VGACTL_ADDR)>>16;
  int height = inl(VGACTL_ADDR)&0xffff;
  printf("%d %d\n",width, height);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    int ww = io_read(AM_GPU_CONFIG).width/32;  
    outl(SYNC_ADDR, 1);
    uint32_t* px = ctl->pixels;
    // printf("%d %d %d %d\n", ctl->x, ctl->y, ctl->w, ctl->h);
    for(int i=0; i<ctl->w; ++i){
      for(int j=0; j<ctl->h; ++j){
        outl((uintptr_t)FB_ADDR+ww*(ctl->y+j)+i, px[j*ctl->w+i]);
      }
    }
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
