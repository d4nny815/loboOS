#include "string.h"

#include <stdint.h>

void* memset(void* dst, int c, size_t n) {
  uint8_t* dest = (uint8_t*)dst;
  for (size_t i = 0; i < n; i++) {
    dest[i] = (uint8_t)c;
  }
  
  return dst;
}

void* memcpy(void* dst, const void* src, size_t n) {
  uint8_t* dest = (uint8_t*)dst;
  uint8_t* src2 = (uint8_t*)src;
  for (size_t i = 0; i < n; i++) {
    dest[i] = src2[i];
  }

  return dst;
}
