#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int start_x = 0, start_y = 0;
// static uint64_t ndl_sec = 0, ndl_usec = 0;

#define strip() for(;i < 1024 && (buf[i] ==' ' || buf[i] == '\n' ); ++i);
#define readnum(x) for(; i<1024 && buf[i] >= '0' && buf[i] <= '9'; ++i) x = x*10 + buf[i] - '0';

uint32_t NDL_GetTicks() {
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  evtdev = open("/dev/events",0,0);
  return read(evtdev, buf, len);
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  char* buf = malloc(1024);
  int fd = open("/proc/dispinfo",0,0);
  read(fd, buf, 1024);
  int i = 0;
  strip();
  if(strncmp(buf+i, "WIDTH", 5) != 0) return;
  i+=5;
  strip();
  if(buf[i] != ':') return;
  ++i;strip();
  int ww = 0, hh = 0;
  readnum(ww);
  strip();
  if(strncmp(buf+i, "HEIGHT", 6) != 0) return;
  i+=6;
  strip();
  if(buf[i] != ':') return;
  ++i;strip();
  readnum(hh);
  screen_h = hh;
  screen_w = ww;
  // printf("ww=%d hh=%d\n w=%d h=%d\n",ww,hh,*w,*h);
  if(*w == 0 && *h == 0) *w = ww, *h = hh;
  start_x = (screen_w - *w) / 2;
  start_y = (screen_h - *h) / 2;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb",0,0);
  // printf("x=%d y=%d w=%d h=%d\n",x,y,w,h);
  // printf("startx = %d starty = %d\n",start_x, start_y);
  for(int i=0; i<h; ++i){
    lseek(fd, (screen_w * (y+i+start_y) + x + start_x)*4, SEEK_SET);
    write(fd, pixels + i * w, 4*w);
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  printf("HELLO?\n");
  int fd = open("/dev/sbctl",0,0);
  int* to_write = malloc(sizeof(int) * 3);
  to_write[0] = freq; to_write[1] = channels; to_write[2] = samples;
  printf("BEFORE WRITE\n");
  write(fd, to_write, sizeof(int)*3);
  printf("HELLO?\n");
  close(fd);
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  while(NDL_QueryAudio() < len){
    printf("NDL_QueryAudio() = %d\n", NDL_QueryAudio());
  }
  int fd = open("/dev/sb",0,0);
  int ret = write(fd, buf, len);
  close(fd);
  return ret;
}

int NDL_QueryAudio() {
  int fd = open("/dev/sbctl",0,0);
  int buf[0];
  read(fd, buf, 4);
  // printf("%d\n",buf[0]);
  close(fd);
  return buf[0];
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  // struct timeval tv;
  // gettimeofday(&tv, NULL);
  // ndl_sec = tv.tv_sec;
  // ndl_usec = tv.tv_usec;
  return 0;
}

void NDL_Quit() {
}
