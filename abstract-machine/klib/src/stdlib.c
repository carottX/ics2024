#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

static uint8_t* addr;
static bool reset=false;

void *malloc(size_t size) {
  printf("SIZE=%d\n",size);
  printf("HEAP SIZE=%d\n",heap.end-heap.start);
  if(!reset){
    reset = true;
    addr = heap.start;
  }
  size = (size_t)ROUNDUP(size, 8);
  addr += size;
  assert((uintptr_t)addr>=(uintptr_t)heap.start && (uintptr_t)addr<(uintptr_t)heap.end);
  return addr-size;
}

void free(void *ptr) {
}

#endif
