#include "myint.h"
#include "mybool.h"
#include "vga.h"

void kmain(void) {
  int db_flag = 1;
  while (!db_flag);
  
  VGA_clear();

  VGA_display_char('c');
  VGA_display_str("Testing a string\n");

  bool k = 0;
  for (int j = 0; j < 18; j++) {
    if (k)
    for (int i = 0; i < 10; i++) VGA_display_char('X');
    
    else
    for (int i = 0; i < 10; i++) VGA_display_char('Y');
  
    VGA_display_char('\n');
    k = !k;
  }
  VGA_display_str("Testing1 a string\n");

  VGA_display_str("Testing2 a string\n");

  VGA_display_str("Testing3 a string\n");


  while(1);
}
