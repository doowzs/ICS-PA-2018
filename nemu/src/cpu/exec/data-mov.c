#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
	// sign extension for imm8. See PA 2.3.html
	if (id_dest->type == OP_TYPE_IMM && id_dest->width == 1) {
		rtl_sext(&id_dest->val, &id_dest->val, 1);
	}
	rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
	rtl_pop(&t2);
	operand_write(id_dest, &t2);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
	rtl_li(&cpu.esp, cpu.ebp);
	rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
		if ((int16_t) reg_w(0) < 0) {
			reg_w(2) = 0xffff;
		} else {
			reg_w(2) = 0x0000;
		}
  }
  else {
		if ((int32_t) reg_l(0) < 0) {
			reg_l(2) = 0xffffffff;
		} else {
			reg_l(2) = 0x00000000;
		}
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    TODO();
  }
  else {
    TODO();
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	assert(id_dest->width >= id_src->width);
	printf("1 = 0x%08x\n", id_src->val);
	rtl_li(&t0, id_src->val);
	rtl_li(&t1, id_dest->width - id_src->width);
	rtl_sar(&t2, &t0, &t1);
	printf("2 = 0x%08x\n", t2);
	rtl_sext(&at, &t2, id_dest->width);
	printf("3 = 0x%08x\n", at);
  operand_write(id_dest, &at);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	if (id_dest->width == 2) {
		rtl_li(&at, id_src->val & 0xffff);
	}
  operand_write(id_dest, &at);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
