#include "common.h"

static _Context* do_event(_Event e, _Context* c) {
  switch (e.event) {
    case 0x81:
      printf("This is do_event in nanos/src/irq.c.\n");
      printf("You are seeing this message because a NO.0x81 yield event occurred.\n");
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
