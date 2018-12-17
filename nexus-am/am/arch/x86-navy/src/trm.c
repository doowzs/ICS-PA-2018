#include <am.h>
#include <x86.h>

#define SERIAL_PORT 0x3f8

extern int main();

void _trm_init() {
  int ret = main();
  _halt(ret);
}

void _putc(char ch) {
  while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  __asm__ volatile(".byte 0xd6" : :"a"(code));

  // should not reach here
  while (1);
}

_Area _heap;
