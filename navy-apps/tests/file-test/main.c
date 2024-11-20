#include <stdio.h>
#include <assert.h>

int main() {
  printf("Entering file-test....\n");
  FILE *fp = fopen("/share/files/num", "r+");
  printf("Passed 0th!\n");
  assert(fp);
  printf("Passed 1st!\n");

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);
  printf("Passed 2nd!\n");

  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
