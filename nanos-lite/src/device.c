#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  _yield(); // schedule
  char *pchar = (char *) buf;
  for (int i = 0; i < len; ++i) {
    _putc(*pchar);
    pchar++;
  }
  return len; // serial has no length
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  _yield(); // schedule
  int key = read_key();
  if (key > 0) {
    if (key & 0x8000) {
      return snprintf(buf, len, "kd %s\n", keyname[key & 0xff]);
    } else {
      return snprintf(buf, len, "ku %s\n", keyname[key & 0xff]);
    }
  } else {
    return snprintf(buf, len, "t %d\n", uptime());
  }
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  strncpy((char *) buf, dispinfo, len);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  _yield(); // schedule
  int w = len / sizeof(uint32_t), h = 1;
  int uint32_offset = offset / sizeof(uint32_t);
  int x = uint32_offset % screen_width(), 
      y = uint32_offset / screen_width();
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
