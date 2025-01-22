#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

void putch(char c);
void yield();

size_t serial_write(const void *buf, size_t offset, size_t len) {
  // yield();
  for(int i=0; i<len; ++i) putch(*((uint8_t*)buf + i));
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  // printf("READ\n");
  // yield();
  size_t bytes_written = 0;
  assert(len >= 3);
  AM_INPUT_KEYBRD_T kbd = io_read(AM_INPUT_KEYBRD);
  if(kbd.keycode == AM_KEY_NONE) return 0;
  // printf("SUCCESS\n");
  if(kbd.keydown) strcpy(buf, "kd ");
  else strcpy(buf,"ku ");
  buf += 3;
  bytes_written += 3;
  if(strlen(keyname[kbd.keycode]) < len - bytes_written) bytes_written += strlen(keyname[kbd.keycode]);
  else bytes_written = len;
  strncpy(buf, keyname[kbd.keycode], len-bytes_written);
  return bytes_written;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int ww = io_read(AM_GPU_CONFIG).width;  
  int hh = io_read(AM_GPU_CONFIG).height;
  return sprintf(buf, "WIDTH:%d\nHEIGHT:%d\n", ww, hh);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  // yield();
  int m = io_read(AM_GPU_CONFIG).width;
  offset /= 4;
  int i = offset / m, j = offset % m;
  io_write(AM_GPU_FBDRAW, j, i, (void*)buf, len/4, 1, true);
  return len;
}

size_t sbctl_read(void* buf, size_t offset, size_t len){
  // printf("bufsize=%d count=%d\n", io_read(AM_AUDIO_CONFIG).bufsize, io_read(AM_AUDIO_STATUS).count);
  size_t freebytes = io_read(AM_AUDIO_CONFIG).bufsize - io_read(AM_AUDIO_STATUS).count;
  *(int32_t*)buf = freebytes;
  return len;
}

size_t sbctl_write(const void* buf, size_t offset, size_t len){
  const int* buf_i = buf;
  io_write(AM_AUDIO_CTRL, buf_i[0], buf_i[1], buf_i[2]);
  return len;
}

size_t sb_write(const void* buf, size_t offset, size_t len){
  // printf("sb_write len=%d\n",len);
  Area sbuf;
  sbuf.start = (void*)buf;
  sbuf.end = (void*)buf + len;
  io_write(AM_AUDIO_PLAY, sbuf);
  // void* bufstart = io_read(AM_AUDIO_PLAY).buf.start;
  // printf("bufstart=%d\n buf=%d\n",(intptr_t)bufstart,(intptr_t)buf);
  // for(int i=0; i<len; ++i) printf("%d ",((uint8_t*)buf)[i]);
  // printf("\n");
  // memcpy(bufstart, buf, len);
  // printf("sb_write end\n");
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
