#include "common.h"

_Context* schedule(_Context *, bool); 
_Context* do_syscall(_Context* c);

static _Context* do_event(_Event e, _Context* c) {
  switch (e.event) {
    case _EVENT_IRQ_TIMER:
#ifdef SYS_DEBUG
      Log("A timer event is triggered.");
#endif
      return schedule(c, false);
    case _EVENT_YIELD:
#ifdef SYS_DEBUG
      //Log("A yield event (0x81) is triggered.");
#endif
      return schedule(c, false);
      break;
    case _EVENT_SYSCALL:
      return do_syscall(c);
    default: panic("Unhandled event ID = %d, fix in nanos/src/irq.c", e.event);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
