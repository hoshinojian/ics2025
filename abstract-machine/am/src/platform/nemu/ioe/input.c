#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

  // AM_INPUT_KEYBRD, AM键盘控制器, 可读出按键信息. keydown为true时表示按下按键, 否则表示释放按键. keycode为按键的断码, 没有按键时, keycode为AM_KEY_NONE.
void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t state = inl(KBD_ADDR);
  if(state & KEYDOWN_MASK >> 31){
    kbd->keydown = 1;
    kbd->keycode = state & 0x7fff;
  }else{
    kbd->keydown = 0;
    kbd->keycode = AM_KEY_NONE;
  }
}
