#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define pass(i) printf("passed %d!\n", i)

int main() {
  printf("test start!\n");

  int *a = (int *) malloc (sizeof(int) * 5);
  assert(a);
  pass(-1);
  double *b = (double *) malloc (sizeof(double) * 10);
  assert(b);
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
