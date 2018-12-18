#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <ndl.h>

NDL_Event event;
int cur_time;

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      _UptimeReg *uptime = (_UptimeReg *)buf;
      NDL_WaitEvent(&event);
      switch (event.type) {
        case NDL_EVENT_TIMER:
          uptime->hi = 0;
          uptime->lo = event.data;
          cur_time = event.data;
          break;
        default:
          uptime->hi = 0;
          uptime->lo = cur_time;
          break;
      }
      return sizeof(_UptimeReg);
    }
    case _DEVREG_TIMER_DATE: {
      _RTCReg *rtc = (_RTCReg *)buf;
      rtc->second = 0;
      rtc->minute = 0;
      rtc->hour   = 0;
      rtc->day    = 0;
      rtc->month  = 0;
      rtc->year   = 2018;
      return sizeof(_RTCReg);
    }
  }
  return 0;
}

void timer_init() {
}
