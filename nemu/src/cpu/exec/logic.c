#include "cpu/exec.h"
#include "cpu/cc.h"

make_EHelper(test) {
	rtl_and(&t2, &id_dest->val, &id_src->val);
	rtl_li(&at, 0);
	rtl_set_CF(&at);
	rtl_set_OF(&at);
	rtl_update_ZFSFPF(&t2, id_dest->width);
  print_asm_template2(test);
}

make_EHelper(and) {
	rtl_and(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
	rtl_li(&at, 0);
	rtl_set_CF(&at);
	rtl_set_OF(&at);
	rtl_update_ZFSFPF(&t2, id_dest->width);
  print_asm_template2(and);
}

make_EHelper(xor) {
	rtl_xor(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
	rtl_li(&at, 0);
	rtl_set_CF(&at);
	rtl_set_OF(&at);
	rtl_update_ZFSFPF(&t2, id_dest->width);
  print_asm_template2(xor);
}

make_EHelper(or) {
	rtl_or(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
	rtl_li(&at, 0);
	rtl_set_CF(&at);
	rtl_set_OF(&at);
	rtl_update_ZFSFPF(&t2, id_dest->width);
  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_sext(&t2, &id_dest->val, id_dest->width);
	rtl_sar(&at, &t2, &id_src->val);
	operand_write(id_dest, &at);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(sar);
}

make_EHelper(shl) {
	rtl_shl(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shl);
}

make_EHelper(shr) {
	rtl_shr(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint32_t cc = decoding.opcode & 0xf;

  rtl_setcc(&t2, cc);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
	rtl_not(&t2, &id_dest->val);
	operand_write(id_dest, &t2);
	/* no affected eflags */
  print_asm_template1(not);
}

make_EHelper(rol) {
  rtl_li(&t0, id_dest->val);
  rtl_li(&t1, id_src->val);
  while (t1) {
    rtl_msb(&t2, &t0, id_dest->width);
    rtl_shli(&t0, &t0, 1);
    rtl_or(&t0, &t0, &t2);
    rtl_subi(&t1, &t1, 1);
  }
  if (id_src->val == 1) {
    /* update OF */
    rtl_msb(&t2, &id_dest->val, id_dest->width);
    rtl_get_CF(&t3);
    rtl_li(&at, t2 != t3 ? 1 : 0);
    rtl_set_OF(&at);
  }
  print_asm_template2(rol);
}

make_EHelper(ror) {
  printf("ROR\n");
  TODO();
  print_asm_template2(ror);
}
