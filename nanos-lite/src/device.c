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

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for(int i=0; i<len; ++i) putch(*((uint8_t*)buf + i));
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  // printf("READ\n");
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
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
