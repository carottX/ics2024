#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <NDL.h>

// uint32_t NDL_GetTicks();

void test_gettimeofday(){
  struct timeval tv;
  // struct timezone tz;
  gettimeofday(&tv, NULL);
  int ms = 500;
  printf("!!!!!!%d\n",ms);
  while (1) {
    while ((tv.tv_sec * 1000 + tv.tv_usec / 1000) < ms) {
      gettimeofday(&tv, NULL);
    }
    ms += 500;
    printf("ms = %d\n", ms);
  }
}

void test_ndltick(){
    uint32_t last_tick = NDL_GetTicks();
  while (1) {
    uint32_t tick = NDL_GetTicks();
    if (tick - last_tick >= 500) {
      printf("%u!\n",tick);
      last_tick = tick;
    }
  }
  // return 0;
}

int main() {
  NDL_Init(0);
  test_ndltick();
  return 0;
}
