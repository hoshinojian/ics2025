//PA3 的核心（上下文扩展）。包含了管理中断、异常和系统调用的 C 语言逻辑。

#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

// enum {
//     EVENT_NULL = 0,
//     EVENT_YIELD, EVENT_SYSCALL, EVENT_PAGEFAULT, EVENT_ERROR,
//     EVENT_IRQ_TIMER, EVENT_IRQ_IODEV,
//   } event;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 11: //ecall
        if(c->gpr[17] == -1){
          ev.event = EVENT_YIELD; break;
        }
      case 3: 
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

//保存寄存器, 构造上下文; 调用C函数, 回复寄存器并且返回
extern void __am_asm_trap(void);

//接收来自os的一个回调函数的指针. 事件发生的时候, CTE把事件和上下文作为参数, 调用这个回调函数
bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  //把asm_trap写到mtvec寄存器里面
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  //保存os传进来的handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else//-1丢到寄存器a7里面去. 是一个伪指令
  asm volatile("li a7, -1; ecall");//可能有其他调用ecall的时候,但是只有这个时候是yield
  //pc放到mepc, cause写入, pc跳转到mtvec
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
