#ifndef PORTIO_H
#define PORTIO_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile ( "outb %w1, %b0" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile ( "inb %b0, %w1"
                  : "=a"(ret)
                  : "Nd"(port)
                  : "memory");
  return ret;
}


#endif /* portIO.h */