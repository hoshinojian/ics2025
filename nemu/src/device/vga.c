/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>

#define SCREEN_W (MUXDEF(CONFIG_VGA_SIZE_800x600, 800, 400))
#define SCREEN_H (MUXDEF(CONFIG_VGA_SIZE_800x600, 600, 300))

static uint32_t screen_width() {
  return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).width, SCREEN_W);
}

static uint32_t screen_height() {
  return MUXDEF(CONFIG_TARGET_AM, io_read(AM_GPU_CONFIG).height, SCREEN_H);
}

static uint32_t screen_size() {
  return screen_width() * screen_height() * sizeof(uint32_t);
}

static void *vmem = NULL;
static uint32_t *vgactl_port_base = NULL;

#ifdef CONFIG_VGA_SHOW_SCREEN
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL; //texture是一个像素缓冲区, 可以往里面写像素

//创建 SDL 窗口（标题带模拟器的 ISA 架构，比如 x86/NEMU），初始化渲染相关的资源
static void init_screen() {
  // typedef struct SDL_Window SDL_Window;
  SDL_Window *window = NULL; //在这里, 只是相当于一个指向sdl窗口对象的指针

  char title[128];
  sprintf(title, "%s-HOSHI", str(__GUEST_ISA__));

  SDL_Init(SDL_INIT_VIDEO);
// #define SDL_INIT_VIDEO          0x00000020u  /**< SDL_INIT_VIDEO implies SDL_INIT_EVENTS */ 位标志

  SDL_CreateWindowAndRenderer(
      SCREEN_W * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      SCREEN_H * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      0, &window, &renderer);//创建窗口的渲染器. 第三个0本来可以是一些flag, 比如说开全屏,这里什么都没开.
      //后面两个地址, 分别是窗口和渲染器. window指向了一个真实存在的窗口对象, renderer指向一个绘图上下文, 一个新创建的渲染器

  SDL_SetWindowTitle(window, title);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);//这个缓冲区属于renderer渲染器, SDL_PIXELFORMAT_ARGB8888代表每一个像素32位, ARGB 8888. 
      //SDL_TEXTUREACCESS_STATIC代表要手动更新这一块内存. 后面的w和h是逻辑屏幕大小, 不是真实窗口大小.
    //创建窗口缓冲区

  SDL_RenderPresent(renderer);//把renderer现在的内容显示到窗口啥功能, 但是这个时候没有像素, 只是做了一次空刷新.
}

//vmem里面的像素数据更新到sdl，再渲染到窗口
static inline void update_screen() {
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint32_t));//vmem替换texture的内容
  SDL_RenderClear(renderer);//清空当前渲染的内容, 避免上一帧干扰这一帧
  SDL_RenderCopy(renderer, texture, NULL, NULL);//t拷贝到r, 
  SDL_RenderPresent(renderer);//呈现出来, 提交这一帧
}
#else
static void init_screen() {}//AM 平台不需要 SDL（AM 框架自己处理显示），所以init_screen()为空

//update_screen()直接调用 AM 框架的io_write，把显存数据传给 AM 的 GPU 模块显示。
static inline void update_screen() {
  io_write(AM_GPU_FBDRAW, 0, 0, vmem, screen_width(), screen_height(), true);
}
#endif
#endif

void vga_update_screen() {
  // TODO: call `update_screen()` when the sync register is non-zero,
  // then zero out the sync register

  // 1. 检查 Sync 寄存器 (vgactl_port_base[1] 对应偏移量 4)
  uint32_t sync = vgactl_port_base[1];

  if (sync != 0) {
    // 2. 调用具体的绘图函数 (将 vmem 投射到 SDL 窗口)
    update_screen();

    // 3. 清除 Sync 寄存器 (归零，等待下一次同步)
    vgactl_port_base[1] = 0;//cpu负责写非0到这里，表示同步寄存器
  }
}

void init_vga() {
  vgactl_port_base = (uint32_t *)new_space(8);//定义第一个控制寄存器
  vgactl_port_base[0] = (screen_width() << 16) | screen_height();//分辨率寄存器
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("vgactl", CONFIG_VGA_CTL_PORT, vgactl_port_base, 8, NULL);
#else
  add_mmio_map("vgactl", CONFIG_VGA_CTL_MMIO, vgactl_port_base, 8, NULL);
#endif

  vmem = new_space(screen_size());
  add_mmio_map("vmem", CONFIG_FB_ADDR, vmem, screen_size(), NULL);
  IFDEF(CONFIG_VGA_SHOW_SCREEN, init_screen());
  IFDEF(CONFIG_VGA_SHOW_SCREEN, memset(vmem, 0, screen_size()));
}
