#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // 初始化函数留空即可，不需要在这里画画
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  // 1. 读取 VGA 控制寄存器 (32位)
  uint32_t screen_info = inl(VGACTL_ADDR);
  
  // 2. 提取宽和高
  uint32_t w = screen_info >> 16;      // 高16位是宽度
  uint32_t h = screen_info & 0xffff;   // 低16位是高度 (你之前写成了 0xffff0000)

  // 3. 一次性赋值给结构体，顺便算出显存大小
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, 
    .has_accel = false,
    .width = w, 
    .height = h,
    .vmemsz = w * h * sizeof(uint32_t) // 这一步很重要，否则系统认为显存大小为0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // 1. 取出绘图参数
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;

  // 2. 获取屏幕宽度（用于计算换行）
  int screen_w = inl(VGACTL_ADDR) >> 16;

  // 3. 逐行拷贝像素
  for (int j = 0; j < h; j ++) {
    for (int i = 0; i < w; i ++) {
      // 计算显存对应的物理地址
      // 显存地址 = 基址 + (当前行 * 屏幕总宽 + 当前列) * 4字节
      uintptr_t addr = FB_ADDR + ((y + j) * screen_w + (x + i)) * sizeof(uint32_t);
      
      // 将像素写入显存
      outl(addr, pixels[j * w + i]);
    }
  }

  // 4. 同步屏幕
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}