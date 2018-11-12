#include "cpu/exec.h"
#include "device/port-io.h"

void raise_intr(uint8_t, vaddr_t);
void difftest_skip_ref();
void difftest_skip_dut();

make_EHelper(lidt) {
  rtl_lm(&at, &id_src->val, 2);
  cpu.IDTR.limit = at; 
  cpu.IDTR.base = id_dest->val;
  
  printf("LIDT loaded: base 0x%08x, limit %d\n", cpu.IDTR.base, cpu.IDTR.limit);

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(int) {
  rtl_push(&cpu.eflags32);
  rtl_push(&cpu.CS);
  rtl_push(&cpu.eip);
  raise_intr(id_dest->val, decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
  difftest_skip_dut();
#endif
}

make_EHelper(iret) {
  TODO();

  print_asm("iret");
}

make_EHelper(in) {
  switch (id_dest->width) {
    case 4: rtl_li(&at, pio_read_l(id_src->val)); break;
    case 2: rtl_li(&at, pio_read_w(id_src->val)); break;
    case 1: rtl_li(&at, pio_read_b(id_src->val)); break;
    default: assert(0);
  }
  operand_write(id_dest, &at);

  print_asm_template2(in);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(out) {
  switch (id_src->width) {
    case 4: pio_write_l(id_dest->val, id_src->val); break;
    case 2: pio_write_w(id_dest->val, id_src->val); break;
    case 1: pio_write_b(id_dest->val, id_src->val); break;
    default: assert(0);
  }

  print_asm_template2(out);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}
