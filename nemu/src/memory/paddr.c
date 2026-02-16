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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>
#include <stdio.h>

#include <common.h> // 【必须加这行，否则看不见 Kconfig 定义的宏】
#include <memory/host.h>


#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
//大小128M的内存条,pg_align和页表有关,指示按照4kB对齐,初始化全0
#endif

//guest address:从0x8000 0000到0x8800 0000, 在si调试时候看到的pc
//host address :真实内存

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
//nemu的地址转化成为c语言能直接用的指针, paddr是0x8000 0100的时候, 偏移量是100, 对应pmem[100]
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }
//haddr-pmem计算当前指针到开头的距离, 加上MBASE对应到nemu地址


static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

//使用在log.c里面定义的变量
extern FILE* mm_log_fp;

#define MTRACE_READ 0
#define MTRACE_WRITE 1

void write2mmlog_read(paddr_t addr, int len, int type){
  fprintf(mm_log_fp, "Addr: " FMT_PADDR "  Len: %d  Type: %s\n",
            addr, 
            len, 
            type == MTRACE_READ ? "READ" : "WRITE"
    );
}

void write2mmlog_write(paddr_t addr, int len, int type, word_t data){
  fprintf(mm_log_fp, "Addr: " FMT_PADDR "  Len: %d  Type: %s  Data: " FMT_WORD "\n",
            addr, 
            len, 
            type == MTRACE_READ ? "READ" : "WRITE",
            data
    );
}


word_t paddr_read(paddr_t addr, int len) {
  IFDEF(CONFIG_MM_TRACE, write2mmlog_read(addr, len, MTRACE_READ)); 
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  IFDEF(CONFIG_MM_TRACE, write2mmlog_write(addr, len, MTRACE_WRITE, data));
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
