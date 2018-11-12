#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  // FIXME FIXME FIXME what is the start address???
  GateDesc gd;
  gd.val = vaddr_read(cpu.IDTR.base + (NO << 4), 16);
  if (gd.present) {
    vaddr_t ptar = gd.offset_15_0 | (gd.offset_31_16 << 16);
    cpu.eip = ptar;
  } else {
    panic("gate is not valid! check at /cpu/intr.c");
  }
}

void dev_raise_intr() {
}
