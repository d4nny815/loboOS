#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "vga.h"
#include "print.h"
#include "keyboard.h"

void printk_tests();

void kmain(void) {
  bool db_flag = 0;
  while (!db_flag);
  
  VGA_clear();

  VGA_display_char('c');
  VGA_display_str("Testing a string\n");

  printk_tests();

  keyboard_init();

  while(1) {
    // echo
  }
}

void printk_tests() {
  printk("%c", 'a'); // should be "a"
  printk("%c", 'Q'); // should be "Q"
  printk("%c", 256 + '9'); // Should be "9"
  printk("\n");
  printk("%s\n", "test string"); // "test string"
  printk("foo%sbar\n", "blah"); // "fooblahbar"
  printk("foo%%sbar\n"); // "foo%bar" // TODO
  printk("%d\n", 815); // "815"
  printk("%d\n", INT_MIN); // "-2147483648"
  printk("%d\n", INT_MAX); // "2147483647"
  printk("%u\n", 0); // "0"
  printk("%u\n", UINT_MAX); // "4294967295"
  printk("%x\n", 0xDEADbeef); // "deadbeef"
  printk("%p\n", (void*)UINTPTR_MAX); // "0xFFFFFFFFFFFFFFFF"
  printk("%hd\n", 0x8000); // "-32768"
  printk("%hd\n", 0x7FFF); // "32767"
  printk("%hu\n", 0xFFFF); // "65535"
  printk("%ld\n", LONG_MIN); // "-9223372036854775808"
  printk("%ld\n", LONG_MAX); // "9223372036854775807"
  printk("%lu\n", ULONG_MAX); // "18446744073709551615"
  printk("%qd\n", (long long int)LONG_MIN); // "-9223372036854775808"
  printk("%qd\n", (long long int)LONG_MAX); // "9223372036854775807"
  printk("%qu\n", (long long unsigned int)ULONG_MAX); // "18446744073709551615"
}



