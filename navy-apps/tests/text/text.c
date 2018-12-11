#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define pass(i) printf("passed %d!\n", i)

int main() {
  printf("test start!\n");

  int xxx[100] = {11111};
  memset(xxx, 0, sizeof(xxx));
  printf("sizeof xxx[100] is %d\n", sizeof(xxx));
  assert(xxx[0] == 0);
  pass(-6);

  int *a = (int *) malloc (sizeof(int) * 5);
  assert(a);
  pass(-5);
  printf("the address of array a is 0x%08x\n", a);
  memset(a, 0, sizeof(int) * 5);
  pass(-4);
  printf("sizeof a[] is %d\n", sizeof(a));
  printf("a[] is [%d %d %d %d %d]\n", a[0], a[1], a[2], a[3], a[4]);
  assert(a[4] == 0);
  pass(-3);

  long *b = (long *) malloc (sizeof(long) * 10);
  assert(b);
  pass(-2);
  memset(b, 0, sizeof(long) * 10);
  pass(-1);
  assert(b[2] == b[8]);
  pass(0);

  FILE *fp = fopen("/share/texts/num", "r+");
  assert(fp);
  pass(1);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);
  pass(2);

  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }
  pass(3);

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }
  pass(4);

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }
  pass(5);

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }
  pass(6);

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
