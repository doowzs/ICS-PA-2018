#include "common.h"

_Context* do_syscall(_Context* c);

static _Context* do_event(_Event e, _Context* c) {
  switch (e.event) {
    case _EVENT_YIELD:
      Log("A yield event (0x81) is triggered.");
      break;
    case _EVENT_SYSCALL:
      do_syscall(c);
      break;
    default: panic("Unhandled event ID = %d, fix in nanos/src/irq.c", e.event);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
