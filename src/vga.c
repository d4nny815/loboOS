#include "vga.h"

#include "string.h"

static uint16_t* vga_buffer = (uint16_t*)VGA_BASE;
static int cursor = 0;
static uint8_t color = FG(0xf) | BG(0);
const int WIDTH = 80;
const int HEIGHT = 20;
const int BUF_SIZE = WIDTH * HEIGHT;

// scroll the screen up 1 line
static inline void scroll() {
  memcpy(vga_buffer, vga_buffer + WIDTH, (BUF_SIZE - WIDTH) * sizeof(uint16_t));
  memset(vga_buffer + BUF_SIZE - WIDTH, 0, WIDTH * sizeof(uint16_t));
  cursor = BUF_SIZE - WIDTH;
} 

// get the current line of the cursor
static inline int get_line(int cursor) {
  return cursor / WIDTH;
}

// set color properties for the VGA screen
void VGA_set_color(uint8_t opts) {
  color = opts;
}

// clear the VGA screen
void VGA_clear(void) {
  memset(vga_buffer, 0, BUF_SIZE * sizeof(uint16_t));
}

// write a char to the screen
void VGA_display_char(char c) {
  if (c == '\n') {
    cursor = (get_line(cursor) + 1) * WIDTH;
    if (cursor >= BUF_SIZE) scroll();
  }
  else if (c == '\r')
    cursor = get_line(cursor) * WIDTH;
  else {
    vga_buffer[cursor] = (color << 8) | c;
    if ((cursor % WIDTH) < (WIDTH - 1)) cursor++;
  }
}

// write a string to the screen
void VGA_display_str(const char* str) {
  while (*str) {
    VGA_display_char(*str);
    str++;
  }
}