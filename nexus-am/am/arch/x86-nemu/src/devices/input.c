#include <am.h>
#include <x86.h>
#include <amdev.h>

const int KBD_REG_ADDR = 0x60;

int next_keycode = 0;

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _KbdReg *kbd = (_KbdReg *)buf;
      
      int next_keycode = inl(KBD_REG_ADDR);
      kbd->keydown = (next_keycode & 0x8000) ? 0 : 1;
      kbd->keycode = next_keycode;
      return sizeof(_KbdReg);
    }
  }
  return 0;
}
