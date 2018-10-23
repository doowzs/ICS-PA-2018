#include <am.h>
#include <amdev.h>
#include <klib.h>

int main(){
  _RTCReg rtc;
  _ioe_init();
  int sec = uptime() / 1000;
  while (1) {
    printf("%d\n", uptime());
    while(uptime() < 1000 * sec) ;
    get_timeofday(&rtc);
    printf("%d-%d-%d %02d:%02d:%02d GMT (", rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
    if (sec == 1) {
      printf("%d second).\n", sec);
    } else {
      printf("%d seconds).\n", sec);
    }
    sec ++;
  }
  return 0;
}
