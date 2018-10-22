#include <am.h>
#include <x86.h>
#include <amdev.h>

const int KBD_REG_ADDR = 0x60;

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _KbdReg *kbd = (_KbdReg *)buf;
      kbd->keydown = 0;
      kbd->keycode = inl(KBD_REG_ADDR);
      if (kbd->keycode != (_KEY_NONE | 0x8000)) { 
        kbd->keydown = 1;
      }
      return sizeof(_KbdReg);
    }
  }
  return 0;
}
