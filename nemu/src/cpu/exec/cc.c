#include "cpu/rtl.h"

/* Condition Code */

void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1;
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  switch (subcode & 0xe) {
    case CC_O:
			rtl_li(&at, cpu.eflags.OF);
			break;
    case CC_B:
			rtl_li(&at, cpu.eflags.CF);
			break;
    case CC_E:
			rtl_li(&at, cpu.eflags.ZF);
			break;
    case CC_BE:
			rtl_li(&at, cpu.eflags.CF || cpu.eflags.ZF);
			break;
    case CC_S:
			rtl_li(&at, cpu.eflags.SF);
			break;
    case CC_L:
			rtl_li(&at, cpu.eflags.SF != cpu.eflags.OF);
			break;
    case CC_LE:
			rtl_li(&at, (cpu.eflags.SF != cpu.eflags.OF) || cpu.eflags.ZF);
			break;

    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }
	*dest = at;

  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
}
