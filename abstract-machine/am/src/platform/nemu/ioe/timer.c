#include <am.h>
#include <nemu.h>

#include <nemu.h>
//#include <riscv.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = 0;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  //1. 总线上从io寄存器里面拿到数据
  // map_write(CONFIG_RTC_MMIO, 0, *rtc_port_base, *rtc);
  //2. 拿到数据之后完成计算
  //extern uint32_t *rtc_port_base;
  //paddr_write(RTC_ADDR, 8, *rtc_port_base);
  

  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
