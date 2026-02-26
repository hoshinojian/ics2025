#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  
  // 1. 获取屏幕大小
  // 方法一：直接读寄存器 (最稳妥)
  uint32_t screen_info = inl(VGACTL_ADDR);
  int w = screen_info >> 16;
  int h = screen_info & 0xffff;
  
  // 2. 获取显存起始地址
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  
  // 3. 填充显存 (这会生成一个颜色渐变的效果)
  for (i = 0; i < w * h; i ++) fb[i] = i;
  
  // 4. 强制同步 (让 NEMU 刷新屏幕)
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen_info = inl(VGACTL_ADDR);
  uint32_t w = screen_info >> 16;
  uint32_t h = screen_info & 0xffff;

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, 
    .has_accel = false,
    .width = w, 
    .height = h,
    .vmemsz = w * h * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;
  int screen_w = inl(VGACTL_ADDR) >> 16;

  for (int j = 0; j < h; j ++) {
    for (int i = 0; i < w; i ++) {
      uintptr_t addr = FB_ADDR + ((y + j) * screen_w + (x + i)) * sizeof(uint32_t);
      outl(addr, pixels[j * w + i]);
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}