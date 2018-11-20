#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ndl.h>

int main() {
  int *a = (int *)malloc(sizeof(int)*5);
  a[0] = 0;
  a[1] = 1;
  assert(!a);
PASS(0);

  NDL_Bitmap *bmp = (NDL_Bitmap*)malloc(sizeof(NDL_Bitmap));
  printf("*bmp = %p\n", bmp);
  printf("malloc ok!\n");
  NDL_LoadBitmap(bmp, "/share/pictures/projectn.bmp");
  printf("bmp loaded!\n");
  printf("bmp->pixels is %p\n", bmp->pixels);
  assert(bmp->pixels);
  printf("assert ok!\n");
  NDL_OpenDisplay(bmp->w, bmp->h);
  printf("display opened!\n");
  NDL_DrawRect(bmp->pixels, 0, 0, bmp->w, bmp->h);
  printf("rectangle drawed!\n");
  NDL_Render();
  NDL_CloseDisplay();
  while (1);
  return 0;
}
