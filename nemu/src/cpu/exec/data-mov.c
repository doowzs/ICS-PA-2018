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
  if (decoding.is_operand_size_16) {
    rtl_li(&t0, cpu.gpr[4]._16);
    for (int i = 0; i < 8; ++i) {
      if (i == 4) {
        rtl_push(&t0);
      } else {
        rtl_push((rtlreg_t *) &cpu.gpr[i]._16);
      }
    }
  } else {
    rtl_li(&t0, cpu.gpr[4]._32);
    for (int i = 0; i < 8; ++i) {
      if (i == 4) {
        rtl_push(&t0);
      } else {
        rtl_push(&cpu.gpr[i]._32);
      }
    }
  }

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
		rtl_li(&at, (int16_t) reg_w(0) < 0 ? 0xffff : 0x0000);
    rtl_sr(2, &at, 2);
  }
  else {
		rtl_li(&at, (int32_t) reg_w(0) < 0 ? 0xffffffff : 0x00000000);
    rtl_sr(2, &at, 4);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t2, 0, 1);
    rtl_sext(&at, &t2, 1);
    rtl_sr(0, &at, 2);
  }
  else {
    rtl_lr(&t2, 0, 2);
    rtl_sext(&at, &t2, 2);
    rtl_sr(0, &at, 4);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	rtl_sext(&t0, &id_src->val, id_src->width);
	operand_write(id_dest, &t0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
