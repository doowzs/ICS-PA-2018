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
	rtl_update_ZFSFPF(&id_dest->val, t2);
  print_asm_template2(xor);
}

make_EHelper(or) {
	rtl_or(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
	rtl_li(&at, 0);
	rtl_set_CF(&at);
	rtl_set_OF(&at);
	rtl_update_ZFSFPF(&id_dest->val, t2);
  print_asm_template2(or);
}

make_EHelper(sar) {
	rtl_sar(&t2, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t2);
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
	/* none affected eflags */
  print_asm_template1(not);
}
