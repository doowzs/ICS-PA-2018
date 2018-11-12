#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  GateDesc gd;
  gd.val = vaddr_read(cpu.IDTR.base + (NO << 4), 16);
  printf("offset1 is %08x\n", gd.offset_15_0);
  rtl_j(gd.offset_15_0 | (gd.offset_31_16 << 16));
}

void dev_raise_intr() {
}
