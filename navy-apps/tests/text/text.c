#include <stdio.h>
#include <assert.h>

#define pass(i) printf("passed %d!\n", i)

int main() {
  printf("test start!\n");
  FILE *fp = fopen("/share/texts/num", "r+");
  printf("file opened\n");
  assert(fp);
  pass(1);

  fseek(fp, 0, SEEK_END);
  printf("fseek done\n");
  long size = ftell(fp);
  printf("fp(size) is %ld\n", size);
  assert(size == 5000);
  pass(2);

  fseek(fp, 500 * 5, SEEK_SET);
  printf("fp is now %ld\n", fp);
  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    printf("read %d-th number %d, fp=%ld\n", i, n, fp);
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
