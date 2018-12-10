#include <am.h>
#include <x86.h>
#include <stdio.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void irq0();
void vecsys();
void vectrap();
void vecnull();

void print_tf(_Context *tf) {
  printf("_CONTEXT ADDR AT %p\n", tf);
  printf("*PROT:  0x%p\n", tf->prot);
  printf("EDI:    0x%08x\n", tf->edi);
  printf("ESI:    0x%08x\n", tf->esi);
  printf("EBP:    0x%08x\n", tf->ebp);
  printf("ESP:    0x%08x\n", tf->esp);
  printf("EBX:    0x%08x\n", tf->ebx);
  printf("EDX:    0x%08x\n", tf->edx);
  printf("ECX:    0x%08x\n", tf->ecx);
  printf("EAX:    0x%08x\n", tf->eax);
  printf("IRQ:    0x%08x\n", tf->irq);
  printf("ERR:    0x%08x\n", tf->err);
  printf("EIP:    0x%08x\n", tf->eip);
  printf("CS:     0x%08x\n", tf->cs);
  /* NO EFLAGS COMPARISON
   * because qemu has more flags. */
  // printf("EFLAGS: 0x%08x\n", tf->eflags);
  printf("Context structure OK. (w/o EFLAGS.)\n");
}

_Context* irq_handle(_Context *tf) {
  _Context *next = tf;
  // print_tf(tf);
  if (user_handler) {
    _Event ev = {0};
    switch (tf->irq) {
      case 0x80: ev.event = _EVENT_SYSCALL; break;
      case 0x81: ev.event = _EVENT_YIELD; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, tf);
    if (next == NULL) {
      next = tf;
    }
  }

  return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[0x20] = GATE(STS_TG32, KSEL(SEG_KCODE), irq0,    DPL_KERN);
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys,  DPL_KERN);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context *) stack.end--;
  // See trap.S and nemu CPU for how this works!
  c->eflags = cpu.eflags32;
  c->cs  = cpu.CS;
  c->eip = entry;
  c->err = 0;
  c->irq = 0x81; // ASM_TRAP
  c->eax = cpu.eax;
  c->ecx = cpu.ecx;
  c->edx = cpu.edx;
  c->ebx = cpu.ebx;
  c->esp = cpu.esp;
  c->ebp = cpu.ebp;
  c->esi = cpu.esi;
  c->edi = cpu.edi;
  c->prot = 0;
  return c;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
