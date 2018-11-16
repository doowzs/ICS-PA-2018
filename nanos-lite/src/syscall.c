#include "common.h"
#include "syscall.h"

extern void *_end;

void syscall_ret(_Context *c, int val) {
  c->GPRx = val;
}

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  char *pbuf;

  a[0] = c->GPR1;
  a[1] = c->ebx;
  a[2] = c->ecx;
  a[3] = c->edx;

  switch (a[0]) {
    case SYS_exit:  // 0
      _halt(0);
      break;

    case SYS_yield: // 1
      _yield();
      syscall_ret(c, 0);
      break;

    case SYS_write: // 4
      switch (a[1]) {
        case 1:
        case 2:
          pbuf = (char *) a[2];
          for (int i = 0; i < a[3]; ++i) {
            _putc(*pbuf);
            ++pbuf;
          }
          syscall_ret(c, a[3]);
          break;
        default:
          panic("fd %d is not supported by SYS_write, fix in nanos/src/syscall.c", a[1]);
      }
      break;

    case SYS_brk: // 9
      _end = (void *) a[1];
      syscall_ret(c, 0);
      break;

    default: panic("Unhandled syscall ID = %d, fix in nanos/src/syscall.c", a[0]);
  }

  return NULL;
}
