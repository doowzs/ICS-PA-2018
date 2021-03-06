/*
 * this is the first process in the OS:
 *   display a splash screen
 *   display a boot menu and receive input
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ndl.h>
#include <font.h>

const char *font_fname = "/share/fonts/Courier-7.bdf";

int W = 0, H = 0;
FILE *evtdev;
static Font *font;

struct MenuItem {
  const char *name, *bin, *arg1;
} items[] = {
  {"Test - dummy",              "/bin/dummy",   NULL},
  {"Test - hello",              "/bin/hello",   NULL},
  {"Test - events",             "/bin/events",  NULL},
  {"Test - text",               "/bin/text",    NULL},
  {"Test - bmptest",            "/bin/bmptest", NULL},
  //{"App - LUA",                 "/bin/lua",     NULL},
  //{"App - NTerminator",         "/bin/nterm",   NULL},
  //{"App - NWM",                 "/bin/nwm",     NULL},
  //{"App - slider",              "/bin/slider",  NULL},
  {"PAL - Xian Jian Gayme",     "/bin/pal",     NULL},
  //{"NES - Battle City",         "/bin/litenes", "/share/games/nes/battlecity.nes"},
  //{"NES - Circus",              "/bin/litenes", "/share/games/nes/circus.nes"},
  {"NES - Yie Ar Kung Fu",      "/bin/litenes", "/share/games/nes/kungfu.nes"},
  //{"NES - Lode Runner",         "/bin/litenes", "/share/games/nes/loderunner.nes"},
  {"NES - Super Mario Bros.",   "/bin/litenes", "/share/games/nes/mario.nes"},
  {"AM - coremark",    "/bin/coremark-am", NULL},
  {"AM - dhrystone",   "/bin/dhrystone-am", NULL},
  {"AM - hello",       "/bin/hello-am", NULL},
  {"AM - litenes",     "/bin/litenes-am", NULL},
  //{"AM - microbench",  "/bin/microbench-am", NULL},
  {"AM - slider",      "/bin/slider-am", NULL},
  {"AM - typing",      "/bin/typing-am", NULL},
  {"AM - videotest",   "/bin/videotest-am", NULL},
};

#define nitems (sizeof(items) / sizeof(items[0]))
#define MAX_PAGE ((nitems - 1) / 10)
#define MAX_IDX_LAST_PAGE ((nitems - 1) % 10)

static int page = 0;
static int i_max = 0;

static void set_i_max() {
  i_max = (page == MAX_PAGE ? MAX_IDX_LAST_PAGE : 9);
  printf("page = %d, MAX_PAGE = %d, MAX_IDX_LAST_PAGE = %d\n", page, MAX_PAGE, MAX_IDX_LAST_PAGE);
}
static void next() {
  if (page < MAX_PAGE) {
    page ++;
    set_i_max();
  }
}

static void prev() {
  if (page > 0) {
    page --;
    set_i_max();
  }
}

static NDL_Bitmap logo;

static void open_display();
static void clear_display();
static void display_menu(int n);

int main(int argc, char *argv[], char *envp[]) {
  if (!freopen("/dev/tty", "r", stdin) ||
      !freopen("/dev/tty", "w", stdout) ||
      !freopen("/dev/tty", "w", stderr)) {
    // the OS does not meet the spec
    exit(1);
  }

  open_display();

  evtdev = fopen("/dev/events", "r");
  font = new Font(font_fname);
  NDL_LoadBitmap(&logo, "/share/pictures/projectn.bmp");
  set_i_max();

  while (1) {
    display_menu(i_max);

    NDL_Event e;
    do {
      NDL_WaitEvent(&e);
    } while (e.type != NDL_EVENT_KEYDOWN);

    int i = -1;
    switch (e.data) {
      case NDL_SCANCODE_0: i = 0; break;
      case NDL_SCANCODE_1: i = 1; break;
      case NDL_SCANCODE_2: i = 2; break;
      case NDL_SCANCODE_3: i = 3; break;
      case NDL_SCANCODE_4: i = 4; break;
      case NDL_SCANCODE_5: i = 5; break;
      case NDL_SCANCODE_6: i = 6; break;
      case NDL_SCANCODE_7: i = 7; break;
      case NDL_SCANCODE_8: i = 8; break;
      case NDL_SCANCODE_9: i = 9; break;
      case NDL_SCANCODE_LEFT: prev(); break;
      case NDL_SCANCODE_RIGHT: next(); break;
    }

    if (i != -1 && i <= i_max) {
      i += page * 10;
      auto *item = &items[i];
      clear_display();

      //avoid const complaints
      char arg1[128], arg2[128], argx[8];
      strcpy(arg1, item->bin);
      strcpy(arg2, item->arg1);
      strcpy(argx, "\0");
      char* const exec_argv[] = {arg1, arg2, argx};
      execve(item->bin, &exec_argv[0], envp);
      
      fprintf(stderr, "\033[31m[ERROR]\033[0m Exec %s failed. (execve in init.cpp returned!)\n\n", item->bin);
      // return 66; // REMEMBER THIS SPECIAL CODE!
    } else {
      fprintf(stderr, "Choose a number between %d and %d\n\n", 0, i_max);
    }
  }
  return -1;
}

static void open_display() {
  FILE *dispinfo = fopen("/proc/dispinfo", "r");

  if (!dispinfo) {
    fprintf(stderr, "[IGN] Cannot open /proc/dispinfo.\n");
    return;
  }

  char buf[128], key[128], value[128], *delim;
  while (fgets(buf, 128, dispinfo)) {
    *(delim = strchr(buf, ':')) = '\0';
    sscanf(buf, "%s", key);
    sscanf(delim + 1, "%s", value);
    if (strcmp(key, "WIDTH") == 0) sscanf(value, "%d", &W);
    if (strcmp(key, "HEIGHT") == 0) sscanf(value, "%d", &H);
  }

  fclose(dispinfo);

  if (W == 0 || H == 0) {
    fprintf(stderr, "[IGN] Invalid display information.\n");
    return;
  }

  NDL_OpenDisplay(W, H);
}

static void clear_display(void) {
  uint32_t *bg_color = (uint32_t*)malloc(sizeof(uint32_t) * W);
  assert(bg_color);
  memset(bg_color, 0xff, sizeof(uint32_t) * W);
  for (int y = 0; y < H; y ++) {
    NDL_DrawRect(bg_color, 0, y, W, 1);
  }
  free(bg_color);
  NDL_Render();
}

static void draw_ch(Font *font, int x, int y, char ch, uint32_t fg, uint32_t bg) {
  uint32_t *bm = font->font[ch];
  if (!bm) return;
  for (int j = 0; j < font->h; j ++) {
    uint32_t pixels[font->w];
    for (int i = 0; i < font->w; i ++) {
      pixels[i] = ((bm && ((bm[j] >> i) & 1))) ? fg : bg;
    }
    NDL_DrawRect(pixels, x, y + j, font->w, 1);
  }
}

static void draw_str(Font *font, int x, int y, char *str, uint32_t fp, uint32_t bg) {
  while (*str) {
    draw_ch(font, x, y, *str, fp, bg);
    x += font->w;
    str ++;
  }
}

static void draw_text_row(char *s, int r) {
  r += 3;
  puts(s);
  draw_str(font, 0, r * font->h, s, 0x123456, 0xffffff);
}

static void display_menu(int n) {
  clear_display();
  NDL_DrawRect(logo.pixels, W - logo.w, 0, logo.w, logo.h);
  
  char buf[80];
  int i = 0;

  sprintf(buf, "  Welcome to Aunt's custom X-NEMU!\n");
  draw_text_row(buf, i);
  i ++;

  sprintf(buf, "  Available applications:\n");
  draw_text_row(buf, i);
  i ++;

  for (int j = 0; j <= n; j ++) {
    auto *item = &items[page * 10 + j];
    sprintf(buf, "  [%d] %s", j, item->name);
    draw_text_row(buf, i + j);
  }

  i = 13;

  sprintf(buf, "  page = %2d / %2d, #total apps = %d", page + 1, nitems / 10 + 1,  nitems);
  draw_text_row(buf, i);
  i ++;

  sprintf(buf, "  help:");
  draw_text_row(buf, i);
  i ++;

  sprintf(buf, "  <-  PageUp");
  draw_text_row(buf, i);
  i ++;

  sprintf(buf, "  ->  PageDown");
  draw_text_row(buf, i);
  i ++;

  sprintf(buf, "  0-9 Choose");
  draw_text_row(buf, i);
  i ++;

  NDL_Render();

  printf("========================================\n");
  printf("Please Choose.\n");
  fflush(stdout);
}
