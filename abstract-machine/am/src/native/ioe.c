#include <am.h>
#include <klib-macros.h>

bool __am_has_ioe = false;
static bool ioe_init_done = false;

void __am_timer_init();
void __am_gpu_init();
void __am_input_init();
void __am_uart_init();
void __am_audio_init();
void __am_disk_init();
//接受一个AM_INPUT_CONFIG_T类型的指针
void __am_input_config(AM_INPUT_CONFIG_T *);
void __am_timer_config(AM_TIMER_CONFIG_T *);
void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
void __am_gpu_config(AM_GPU_CONFIG_T *);
void __am_gpu_status(AM_GPU_STATUS_T *);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
void __am_uart_config(AM_UART_CONFIG_T *);
void __am_uart_tx(AM_UART_TX_T *);
void __am_uart_rx(AM_UART_RX_T *);
void __am_audio_config(AM_AUDIO_CONFIG_T *);
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
void __am_audio_status(AM_AUDIO_STATUS_T *);
void __am_audio_play(AM_AUDIO_PLAY_T *);
void __am_disk_config(AM_DISK_CONFIG_T *cfg);
void __am_disk_status(AM_DISK_STATUS_T *stat);
void __am_disk_blkio(AM_DISK_BLKIO_T *io);
static void __am_net_config (AM_NET_CONFIG_T *cfg)    { cfg->present = false; }

// *handler_t代表一个指针! 不加括号, void* hanler_t(void*)可以理解成为一个返回void地址的函数
//但是加上括号之后, 这就是一个指针. 
//tdef  返回值  表明是一个指针  表明接受什么变量
typedef void (*handler_t)(void *buf);
//没有typedef, 就是定义了一个handler_t的变量, 这是一个函数指针.
    //对于int* a, 也是a这个类型是一个指针, 而不是*a是一个指针
//加上之后, 这就是一个新类型,类型名字叫做handler_t


//是一个存放函数指针的数组, 每一个元素接受在amdev.h里面的enum出来的元素,
//然后给这个元素, 也就是int, 在数组里面找到一个位置, 存放函数的指针
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __am_timer_config,//下标为AM_~ 的地址里面, 存放了这个函数的指针
  [AM_TIMER_RTC   ] = __am_timer_rtc,
  [AM_TIMER_UPTIME] = __am_timer_uptime,
  [AM_INPUT_CONFIG] = __am_input_config,
  [AM_INPUT_KEYBRD] = __am_input_keybrd,
  [AM_GPU_CONFIG  ] = __am_gpu_config,
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,
  [AM_GPU_STATUS  ] = __am_gpu_status,
  [AM_UART_CONFIG ] = __am_uart_config,
  [AM_UART_TX     ] = __am_uart_tx,
  [AM_UART_RX     ] = __am_uart_rx,
  [AM_AUDIO_CONFIG] = __am_audio_config,
  [AM_AUDIO_CTRL  ] = __am_audio_ctrl,
  [AM_AUDIO_STATUS] = __am_audio_status,
  [AM_AUDIO_PLAY  ] = __am_audio_play,
  [AM_DISK_CONFIG ] = __am_disk_config,
  [AM_DISK_STATUS ] = __am_disk_status,
  [AM_DISK_BLKIO  ] = __am_disk_blkio,
  [AM_NET_CONFIG  ] = __am_net_config,
};

bool ioe_init() {
  panic_on(cpu_current() != 0, "call ioe_init() in other CPUs");
  panic_on(ioe_init_done, "double-initialization");
  __am_has_ioe = true;
  return true;
}

static void fail(void *buf) { panic("access nonexist register"); }

void __am_ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  __am_timer_init();
  __am_gpu_init();
  __am_input_init();
  __am_uart_init();
  __am_audio_init();
  __am_disk_init();
  ioe_init_done = true;
}

static void do_io(int reg, void *buf) {
  if (!ioe_init_done) {
    __am_ioe_init();
  }
  ((handler_t)lut[reg])(buf);
}

void ioe_read (int reg, void *buf) { do_io(reg, buf); }
void ioe_write(int reg, void *buf) { do_io(reg, buf); }
