#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  char *pchar = (char *) buf;
  for (int i = 0; i < len; ++i) {
    _putc(*pchar);
    pchar++;
  }
  return 0; // serial has no length
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  strncpy((char *) buf, dispinfo, 128);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int w = len >> 2, h = 1; // len / 4;
  int x = offset / screen_width(), y = offset % screen_width();
  draw_rect((uint32_t *) buf, x, y, w, h);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  /* initialize dispinfo */
  memset(dispinfo, 0, 128 * sizeof(char));
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", screen_width(), screen_height());
}
