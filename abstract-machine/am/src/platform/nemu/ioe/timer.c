#include <am.h>
#include <nemu.h>
#include <klib.h>

void __am_timer_init() {
  outl(RTC_ADDR,0);
  outl(RTC_ADDR+4,0);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // printf("%u\n", inl());
  uint64_t high = (uint64_t)inl(RTC_ADDR+4);
  uint64_t low = (uint64_t)inl(RTC_ADDR);
  uptime->us = (high<<32)+low;
  // outl(RTC_ADDR, uptime->us+500);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
