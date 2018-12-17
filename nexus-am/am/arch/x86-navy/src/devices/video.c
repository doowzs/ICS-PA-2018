#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

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

      int cnt = 0;
      int x = ctl->x, y = ctl->y;
      int w = ctl->w, h = ctl->h;
      int W = SCREEN_W, H = SCREEN_H;
      size_t cp_bytes = sizeof(uint32_t) * ((W - x < w) ? W - x : w);
      uint32_t *pfb = fb + y * W + x; 
      uint32_t *ppx = ctl->pixels;
      for ( ; cnt < h && y + cnt < H; ++cnt, pfb += W, ppx += w) {
        memcpy(pfb, ppx, cp_bytes);
      }
      
      if (ctl->sync) {
        // do nothing, hardware syncs.
      }
      return sizeof(_FBCtlReg);
    }
  }
  return 0;
}

void vga_init() {
}
