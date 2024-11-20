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
  int ms = 500;
  NDL_Init(0);
  while(1){
    while(NDL_GetTicks() < ms);
    printf("ms = %d\n",ms);
    ms += 500;
  }
}

int main() {
  test_ndltick();
  return 0;
}
