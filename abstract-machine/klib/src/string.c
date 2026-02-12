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

//将 src 字符串追加到 dst 字符串的末尾。src 的第一个字符会覆盖 dst末尾的 null 字节，结果末尾会添加一个新的 null 字节。
//这个函数不准备安全, 因为本来就不安全. 标准库就是这样实现的
char *strcat(char *dst, const char *src)
{
  if (!dst || !src) return dst;
  //内存里面的值不等于内存的所有权. 检查到一个位置有值之后不能确定这个地方再写入会不会导致越界
  char *ptr = dst + strlen(dst);
  while (*src != '\0') {
        *ptr = *src;
        ptr++;
        src++;
    }
  *ptr = '\0';
  return dst;
}

//比较s1和s2, s1小返回小于零, s1大返回大于零
/*
int strcmp(const char *s1, const char *s2)
{
  if(!s1 && !s2)return 0;
  if(!s1)return -1;
  if(!s2)return 1;
  int s1len = strlen(s1), s2len = strlen(s2);
  int len = (s1len <= s2len ? s1len : s2len);
  for(int i = 0; i < len; i++){
    if(s1[i] < s2[i])return -1;
    if(s1[i] > s2[i])return 1;
  }
  //这个时候, 可能一个结束了一个没有: 也就是说一个是另一个的前缀
  if(s1len == s2len)return 0;
  if(s1len < s2len)return -1;
  return 1;
}
  */
 //高校版本
 int strcmp(const char *s1, const char *s2){
  if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
  while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    // 此时 s1 和 s2 指向第一个不相同的字符，或者结尾的 \0
    // 直接做减法即可返回 <0, 0, >0 的值
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
 }

//最多比较前 n 个字节
int strncmp(const char *s1, const char *s2, size_t n)
{
  if (n == 0) return 0;
  if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);

  while (n > 0 && *s1 && (*s1 == *s2)) {
        n--;
        // 如果 n 减到 0 了，说明前 n 个都相等，此时 s1 和 s2 还停在相等的字符上
        // 判断循环退出的原因
        if (n == 0) break; 
        s1++;
        s2++;
    }
    //如果是因为 n==0 退出的，说明前 n 个完全一样
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

//将 s 指向的内存区域的前 n 个字节设置为常量字节 c
void *memset(void *s, int c, size_t n)
{
  unsigned char *p = (unsigned char *)s;//强制转换类型, 因为不能对void* 进行运算
  if(n == 0)return s;
  uint64_t i = 0;
  while(n--){
    p[i++] = (unsigned char)c;
  }
  return s;
}

//描述： 从 src 复制 n 个字节到 dst。
void *memmove(void *dst, const void *src, size_t n)
{
    // 1. 强转为 char* 以便进行字节级操作
    char *d = (char *)dst;
    const char *s = (const char *)src;

    if (d == s) return dst; // 自身复制，直接返回

    // 2. 判断复制方向
    if (d < s) {
        // 情况 A: 目标在前 (安全，或者无重叠)，从前往后拷
        // 这里的逻辑和 memcpy 一样
        while (n--) {
            *d++ = *s++;
        }
    } else {
        // 情况 B: 目标在后 (可能重叠)，从后往前拷
        // 先把指针移动到末尾
        d += n;
        s += n;
        // 倒序复制
        while (n--) {
            *--d = *--s; // 先减指针再赋值，对应 d[n-1]
        }
    }

    return dst;
}

//描述： 从内存区域 src 复制 n 个字节到内存区域 dst。
//src 和 dst 的内存区域不能重叠。如果重叠，结果未定义（应使用 memmove）。通常 memcpy 比 memmove 稍快。3
//假设内存不重叠（这是调用者的责任）。
void *memcpy(void *out, const void *in, size_t n)
{
    // 1. 强转为 char*
    char *d = (char *)out;
    const char *s = (const char *)in;

    // 2. 简单粗暴的单向复制
    while (n--) {
        *d++ = *s++;
    }

    return out;
}

//比较内存区域 s1 和 s2 的前 n 个字节。
int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    // 循环 n 次
    while (n--) {
        // 一旦发现不相等，立即返回差值
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    // 全部比完都一样，返回 0
    return 0;
}

#endif
