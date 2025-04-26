#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "vga.h"
#include "print.h"
#include "keyboard.h"

void printk_tests();

void kmain(void) {
  bool db_flag = 1;
  while (!db_flag);
  
  VGA_clear();

  VGA_display_char('c');
  VGA_display_str("Testing a string\n");

  printk_tests();

  keyboard_init();

  printk("Press SPACE to clear screen\n");
  while (get_key() != ' '); 
  VGA_clear();

  while(1) {
    // echo
    printk("%c", get_key());
  }
}

void printk_tests() {
  // %c tests
  printk("%c => 'a', ", 'a');
  printk("%c => 'Q', ", 'Q');
  printk("%c => '9'\n", 256 + '9'); // 313 wraps to ASCII 57 = '9'

  // %s and %% tests
  printk("%s => 'test string'\n", "test string");
  printk("foo%sbar => 'fooblahbar'\n", "blah");
  printk("foo%%sbar => 'foo%%sbar'\n"); // Literal %

  // %d tests
  printk("%d => '-2147483648', ", INT_MIN);
  printk("%d => '2147483647'\n", INT_MAX);

  // %u tests
  printk("%u => '0', ", 0);
  printk("%u => '4294967295'\n", UINT_MAX);

  // %x tests
  printk("%x => 'deadbeef'\n", 0xDEADbeef);

  // %p test
  printk("%p => '0xFFFFFFFFFFFFFFFF'\n", (void*)UINTPTR_MAX);

  // %h[dux] tests
  printk("%hd => '-32768', ", (short)0x8000);
  printk("%hd => '32767', ", (short)0x7FFF);
  printk("%hu => '65535', ", (unsigned short)0xFFFF);
  printk("%hx => 'ffff'\n", (unsigned short)0xFFFF); 

  // %l[dux] tests
  printk("%ld => '-9223372036854775808'\n", LONG_MIN);
  printk("%ld => '9223372036854775807'\n", LONG_MAX);
  printk("%lu => '18446744073709551615'\n", ULONG_MAX);
  printk("%lx => 'ffffffffffffffff'\n", ULONG_MAX);

  // %q[dux] tests
  printk("%qd => '-9223372036854775808'\n", (long long)LONG_MIN);
  printk("%qd => '9223372036854775807'\n", (long long)LONG_MAX);
  printk("%qu => '18446744073709551615'\n", (unsigned long long)ULONG_MAX);
  printk("%qx => 'ffffffffffffffff'\n", (unsigned long long)ULONG_MAX);

  printk("[PRINT] Done with tests");
}



