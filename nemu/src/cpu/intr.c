#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  uint16_t offset_15_0 = 0, offset_31_16 = 0; 
  offset_15_0 = vaddr_read(cpu.IDTR.base + (NO << 6), 2);
  printf("base is %04x\n", cpu.IDTR.base);
  printf("offset1 is %04x\n", offset_15_0);
  panic("AT INT 0x%x", NO);
  rtl_j(offset_15_0 | (offset_31_16 << 16));
}

void dev_raise_intr() {
}
