#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>
#include <ndl.h>

#define SCREEN_PORT 0x100
#define SCREEN_H 300
#define SCREEN_W 400

static uint32_t* const fb __attribute__((used)) = (uint32_t *)0x40000;

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      info->width = SCREEN_W;
      info->height = SCREEN_H;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;

      int x = ctl->x, y = ctl->y;
      int w = ctl->w, h = ctl->h;
      uint32_t *ppx = ctl->pixels;
      NDL_DrawRect(ppx, x, y, w, h);
      return sizeof(_FBCtlReg);
    }
  }
  return 0;
}

void vga_init() {
  NDL_OpenDisplay(SCREEN_W, SCREEN_H);
}
