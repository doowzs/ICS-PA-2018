#include "common.h"

static _Context* do_event(_Event e, _Context* c) {
  switch (e.event) {
    case _EVENT_YIELD:
      printf("[\033[1;31mITR\033[0m]This is do_event in nanos/src/irq.c.\n");
      printf("You are seeing this message because a NO.0x81 yield event occurred.\n");
      break;
    default: panic("Unhandled event ID = %d, fix in nanos/src/irq.c", e.event);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
