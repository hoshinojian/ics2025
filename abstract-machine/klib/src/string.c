#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  if (!s)
    return 0;
  size_t i = 0; // 用int可能因为过长字符串导致错误， size_t可能是long类型
  while (s[i] != '\0')
    i++;
  return i;
}
/*
bool check_overlap(char *dst, char* src){
  //如果dst在src的前面, 那么其实无所谓的
  if(dst < src)return true;
  else if(dst == src) return false;//交给上层
  else{
    if(src + strlen(src) >= dst)return false;
  }
}*/

// 两个重叠是mmemove的工作,不是strcpy的工作.
// 但是即使有位置重叠, 也有解决办法.
// 当dst在src的前面的时候, 从前往后拷贝
// src在前面的时候, 从后往前拷贝
/*
char *strcpy(char *dst, const char *src) {
  if(dst == src)return dst;
  uint64_t i = 0;
  uint64_t srcsize = strlen(src);
  for(; i <srcsize; i++){
    if(&dst[i] == src){
      LOG("dst复制到了src的位置导致停止\n");
      dst[i - 1] = '\0';
      return dst;
    }
    if(&src[i] == dst){
      LOG("src占用到了dst的位置导致停止\n");
      dst[i] = '\0';
      return dst;
    }
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}
*/
char *strcpy(char *dst, const char *src)
{
  if (src == dst)
    return dst;
  if (src < dst)
  {
    uint64_t i = strlen(src);
    for (;; i--)
    {
      dst[i] = src[i];
      if (i == 0)
        break;
    }
    return dst;
  }
  // src > dst
  uint64_t i = 0;
  uint64_t len = strlen(src);
  for (; i < len + 1; i++)
    dst[i] = src[i];
  return dst;
}

// 最多赋值n个字节, src长度小于n, 那么填充null, 大于n, 那么dst不以null结尾
char *strncpy(char *dst, const char *src, size_t n)
{
  if (dst == src)
    return dst;
  if (src < dst)
  {
    uint64_t i = strlen(src);
    if (i < n)
    {
      for (;; i--)
      {
        dst[i] = src[i];
        if (i == 0)
          break;
      }
      for(i = strlen(src);i < n; i++){
        dst[i] = '\0';
      }
    }else{
      uint64_t max_ = n - 1;
      for(; ; max_--){
        dst[max_] = src[max_];
        if(!max_)break;
      }
    }
    return dst;
  }
  uint64_t i = 0;
  uint64_t len = strlen(src);
  uint64_t max_ = n;
  if(len < max_){
    for (; i < len + 1; i++)
    dst[i] = src[i];
    for(; i < max_; i++)dst[i] = '\0';
    return dst;
  }
  for(; i < max_; i++)dst[i] = src[i];
  return dst;
}

char *strcat(char *dst, const char *src)
{
  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2)
{
  panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n)
{
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n)
{
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n)
{
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  panic("Not implemented");
}

#endif
