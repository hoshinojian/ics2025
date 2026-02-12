#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>



#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int int_to_char(int num, char* buf){
  int i = 0;
  int is_neg = 0;
  char* begin = buf;

  if(num == -2147483648){
    strcpy(buf, "-2147483648");
    return 11;
  }

  if(num == 0){
    *buf++ = '0';
    return 1;
  }

  if(num < 0){
    is_neg = 1;
    num = -num;
  }

    char temp_buf[32]; // 暂存反向的数 
    while (num > 0) {
        temp_buf[i++] = (num % 10) + '0';
        num = num / 10;
    }
    if (is_neg) {
        temp_buf[i++] = '-';
    }

    for(int j = 0; j < i; j++){
        *buf++ = temp_buf[i - 1 - j];
    }
    
    return buf - begin; // 返回写入的长度
}

//格式化之后的结果写到out数组里面去, 而不是输出到标准输出.
int sprintf(char *out, const char *fmt, ...) {
  char* begin_ = out;
  va_list ap;
  va_start(ap, fmt);
  //switch(va_arg(ap, type)){
  while(*fmt){
    if(*fmt == '%'){
      fmt++;
      switch (*fmt)
      {
      case 'd':{
        int num = va_arg(ap, int);
        int len = int_to_char(num, out);
        out += len;
        break;
      }
      case 'c':{
        //char c = va_arg(ap, char);
        /*
        error: ‘char’ is promoted to ‘int’ when passed through ‘...’ [-Werror]
   72 |         char c = va_arg(ap, char);
   */
        //char传递给...的时候, 在stack上面强制展开成为int
        //用 va_arg(ap, char) 去取，编译器会试图按 1 字节去读。这不仅读到的数据可能是错的（取决于大小端序），而且会导致 ap 指针移动的步长不对
        int c = va_arg(ap, int);
        *out++ = (char)c;
        break;
      }
      default:
        *out++ = *fmt;
        break;
      }
    }else{
      *out++ = *fmt;
    }
    fmt++;
  }
  *out = '\0';
  va_end(ap);
  return out - begin_;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
