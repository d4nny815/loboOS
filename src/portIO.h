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

// static inline void outb(uint16_t port, uint8_t val)
// {
//     __asm__ volatile (
//         ".intel_syntax noprefix\n\t"
//         "out dx, al\n\t"          /* DX = port, AL = value */
//         ".att_syntax prefix"
//         :
//         : "a"(val), "d"(port)
//         : "memory");
// }

// static inline uint8_t inb(uint16_t port)
// {
//     uint8_t ret;
//     __asm__ volatile (
//         ".intel_syntax noprefix\n\t"
//         "in al, dx\n\t"           /* AL ← byte from port in DX */
//         ".att_syntax prefix"
//         : "=a"(ret)
//         : "d"(port)
//         : "memory");
//     return ret;
// }


#endif /* portIO.h */