#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };
enum { R_E_CF=0, R_E_PF=2, R_E_AF=4, R_E_ZF=6, R_E_SF=7, R_E_OF=11 };

/* Re-organized the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct {
	union {
    union {
      uint32_t _32;
      uint16_t _16;
      uint8_t _8[2];
    } gpr[8];

    /* Do NOT change the order of the GPRs' definitions. */

    /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
     * in PA2 able to directly access these registers.
     */

	  struct {
      rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
	  };
	};

  vaddr_t eip;

	union {
		rtlreg_t eflags32;
		struct {
			rtlreg_t CF:1, :1, PF:1, :1, AF:1, :1, ZF:1, SF:1, :1, IF:1, :1, OF:1, :20;
		} eflags;
	};

  rtlreg_t CS;
  rtlreg_t CR[8];

  struct {
    rtlreg_t limit :16;
    vaddr_t base;
  } IDTR;
  
  bool INTR; // timer interrupt

} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])
#define reg_CR(index) (cpu.CR[check_reg_index(index)])
#define EFLAGS_SIZE 4

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];
extern const char* regse[];
extern const char* regse_upper[];
extern const int regse_index[];

static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

#endif
