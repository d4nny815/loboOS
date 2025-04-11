#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "vga.h"
#include "print.h"

void kmain(void) {
  bool db_flag = 1;
  while (!db_flag);
  
  VGA_clear();

  VGA_display_char('c');
  VGA_display_str("Testing a string\n");

  printk_tests();

  while(1);
}



