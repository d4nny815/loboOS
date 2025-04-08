#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_BASE  (0xb8000)
#define FG(x)     (x & 0xf)
#define BG(x)     ((x & 0x7) << 4)

void VGA_set_color(uint8_t opts);
void VGA_clear(void);
void VGA_display_char(char c);
void VGA_display_str(const char* str);

#endif /* vga.h */