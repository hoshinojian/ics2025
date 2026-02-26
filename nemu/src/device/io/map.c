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

#include <isa.h>
#include <memory/host.h>
#include <memory/vaddr.h>
#include <device/map.h>

/*
typedef struct {
  const char *name;
  // we treat ioaddr_t as paddr_t here
  paddr_t low;
  paddr_t high;
  void *space;
  io_callback_t callback;
} IOMap;
*/

//在map.c里面实现映射管理, 包括I/O空间的分配及其映射, 还有映射的访问接口.

//所有设备初始化的时候,从这一部分里面化出来一部分,并且把物理指针绑定到对应设备IOmap的space
#define IO_SPACE_MAX (32 * 1024 * 1024)//32MB的连续内存池

static uint8_t *io_space = NULL;
static uint8_t *p_space = NULL;

uint8_t* new_space(int size) {
  uint8_t *p = p_space;
  // 为设备分配能按页对齐的内存
  size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;
  p_space += size;
  assert(p_space - io_space < IO_SPACE_MAX);
  return p;
}

//接受一个具体的映射结构map和一个想要访问的地址addr，验证是不是在low到high中间
static void check_bound(IOMap *map, paddr_t addr) {
  if (map == NULL) {
    Assert(map != NULL, "address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
  } else {
    Assert(addr <= map->high && addr >= map->low,
        "address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
        addr, map->name, map->low, map->high, cpu.pc);
  }
}

//检查传入的回调函数是不是有效的，有效的话就带参数执行这个回调函数
//读写操作和设备逻辑之间的触发开关， cpu给一个地址写内存之后，用这个函数启动处理代码
static void invoke_callback(io_callback_t c, paddr_t offset, int len, bool is_write) {
  if (c != NULL) { c(offset, len, is_write); }
}

void init_map() {
  io_space = malloc(IO_SPACE_MAX);//空间起始指针
  assert(io_space);
  p_space = io_space;//对应当前可以分配的指针的位置
}

//从addr映射到map所指示的目标空间并且访问, 这个过程可能触发callback, 对设备和目标空间的状态更新
//每一次io读写的时候, 才会调用设备提供的callback
extern FILE* device_log_fp;
void device_log_write(IOMap *map){
  fprintf(device_log_fp, "%s\n", map->name);
}


word_t map_read(paddr_t addr, int len, IOMap *map) {
  assert(len >= 1 && len <= 8);
  check_bound(map, addr);
  paddr_t offset = addr - map->low;

  invoke_callback(map->callback, offset, len, false); // prepare data to read
  word_t ret = host_read(map->space + offset, len);
  return ret;
}

void map_write(paddr_t addr, int len, word_t data, IOMap *map) {
  assert(len >= 1 && len <= 8);
  check_bound(map, addr);
  paddr_t offset = addr - map->low;
  host_write(map->space + offset, len, data);
  
  invoke_callback(map->callback, offset, len, true);
}
