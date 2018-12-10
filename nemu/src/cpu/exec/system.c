#include "cpu/exec.h"
#include "device/port-io.h"

void raise_intr(uint8_t, vaddr_t);
void difftest_skip_ref();
void difftest_skip_dut();

make_EHelper(lidt) {
  rtl_lm(&at, &id_src->val, 2);
  cpu.IDTR.limit = at; 
  cpu.IDTR.base = id_dest->val;

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  operand_write(id_dest, &id_src->val);
  //printf("moving R No.%d to CR No.%d with value 0x%08x\n", id_src->reg, id_dest->reg, id_src->val);

  printf("CR0 is now 0x%08x\n", cpu.CR[0]);
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(mov_cr2r) {
  operand_write(id_dest, &id_src->val);
  //printf("moving CR No.%d to R No.%d with value 0x%08x\n", id_src->reg, id_dest->reg, id_src->val);

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(int) {
  rtl_push(&cpu.eflags32);
  rtl_push(&cpu.CS);
  raise_intr(id_dest->val, decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
  difftest_skip_dut();
#endif
}

make_EHelper(iret) {
  if (decoding.is_operand_size_16) {
    printf("16-bit iret is not implemented!\n");
    TODO();
  } else {
    rtl_pop(&t0); // NEXT EIP
    rtl_pop(&cpu.CS);
    rtl_pop(&cpu.eflags32);
    rtl_j(t0);
  }

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
