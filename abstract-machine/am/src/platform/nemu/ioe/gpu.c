#include <am.h>
#include <nemu.h>

// 定义同步寄存器地址
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // 1. 清除之前的测试代码
  // 这里的初始化什么都不用做，因为我们不需要在启动时就画图
  // 画图的任务全权交给 __am_gpu_fbdraw
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  // 读取屏幕大小信息
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
  // 1. 获取绘图区域参数
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;

  // 2. 获取屏幕宽度 (用于计算显存的换行偏移)
  int screen_w = inl(VGACTL_ADDR) >> 16;

  // 3. 逐行拷贝像素
  for (int j = 0; j < h; j ++) {
    for (int i = 0; i < w; i ++) {
      // 目标地址 = 显存基址 + (当前绝对行 * 屏幕宽 + 当前绝对列) * 4
      uintptr_t dest = FB_ADDR + ((y + j) * screen_w + (x + i)) * sizeof(uint32_t);
      
      // 写入显存
      outl(dest, pixels[j * w + i]);
    }
  }

  // 4. 处理同步
  // 只有当应用程序要求 sync 时才同步，不要每次 draw 都同步（否则会很慢）
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}