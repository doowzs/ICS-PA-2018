#include <am.h>

extern int main();

void _trm_init() {
  int ret = main();
  _halt(ret);
}

void _putc(char ch) {
}

void _halt(int code) {
  __asm__ volatile(".byte 0xd6" : :"a"(code));

  // should not reach here
  while (1);
}

_Area _heap;
