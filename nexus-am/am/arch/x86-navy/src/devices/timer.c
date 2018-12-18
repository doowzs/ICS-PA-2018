#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <ndl.h>

NDL_Event event;
uint32_t boot_time = 0;

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      _UptimeReg *uptime = (_UptimeReg *)buf;
      while (event.type != NDL_EVENT_TIMER) NDL_WaitEvent(&event);
      uptime->hi = 0;
      uptime->lo = event.data - boot_time;
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
  while (event.type != NDL_EVENT_TIMER) NDL_WaitEvent(&event);
  boot_time = event.data;
}
