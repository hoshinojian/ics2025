#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)



 void __am_gpu_init() {
  int i;
  int w = io_read(AM_GPU_CONFIG).width / 32;
  int h = io_read(AM_GPU_CONFIG).height / 32;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);
 }

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = 0, .height = 0,
    .vmemsz = 0
  };
  cfg->width = inl(VGACTL_ADDR) >> 16;
  cfg->height = inl(VGACTL_ADDR) & 0xffff0000;
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // 1. 取出绘图参数
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;

  // 2. 获取屏幕宽度（用于计算换行）
  // 注意：这里读取的是 VGACTL_ADDR 的高 16 位
  int screen_w = inl(VGACTL_ADDR) >> 16;

  // 3. 逐行拷贝像素
  for (int j = 0; j < h; j ++) {
    for (int i = 0; i < w; i ++) {
      // 计算显存对应的物理地址
      // 基地址 (FB_ADDR) + 偏移量 ((行 * 宽 + 列) * 4字节)
      uintptr_t addr = FB_ADDR + ((y + j) * screen_w + (x + i)) * sizeof(uint32_t);
      
      // 将像素写入显存
      outl(addr, pixels[j * w + i]);
    }
  }

  // 4. 同步屏幕 (Sync)
  // 如果 ctl->sync 为真，说明这一帧画完了，通知 NEMU 刷新画面
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
