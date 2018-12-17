#include <am.h>

extern int main();

void _trm_init() {
  int ret = main();
  _halt(ret);
}

void _putc(char ch) {
  printf("HELLO\n");
  while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  __asm__ volatile(".byte 0xd6" : :"a"(code));

  // should not reach here
  while (1);
}

_Area _heap;
