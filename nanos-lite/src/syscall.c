#include "common.h"
#include "syscall.h"

void syscall_ret(_Context *c, int val) {
  c->GPRx = val;
}

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit:  // 0
      _halt(0);
      break;
    case SYS_yield: // 1
      _yield();
      syscall_ret(c, 0);
      break;
    default: panic("Unhandled syscall ID = %d, fix in nanos/src/syscall.c", a[0]);
  }

  return NULL;
}
