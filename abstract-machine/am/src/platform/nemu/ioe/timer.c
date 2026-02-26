#include <am.h>
#include <nemu.h>

#include "../../../riscv/riscv.h"

//#include <riscv.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  //假设io寄存器这个时候已经有了数据, 那么直接从io寄存器取到总线上
  //uint64_t low = inl(RTC_ADDR);
  //uint64_t high = inl(RTC_ADDR + 4);

  //真傻逼 看update, 要调转先后顺序

  uint64_t high = inl(RTC_ADDR + 4);
  uint64_t low = inl(RTC_ADDR);

  //然后输出就好了

  uptime->us = (high << 32) + low;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
