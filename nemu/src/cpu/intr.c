#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  GateDesc gd;
  vaddr_t gd_addr = cpu.IDTR.base + NO * 0x8;
  gd.val[0] = vaddr_read(gd_addr + 0, 4);
  gd.val[1] = vaddr_read(gd_addr + 4, 4);
  printf("OFFSETs are: 0x%04x 0x%04x\n", gd.offset_31_16, gd.offset_15_0);
  if (gd.present) {
    rtl_j((gd.offset_31_16 << 16) | gd.offset_15_0);
  } else {
    panic("Gate is not present!\n");
  }
}

void dev_raise_intr() {
}
