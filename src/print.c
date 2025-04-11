#include "print.h"

#include "vga.h"
#include <stdarg.h>
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>

#define PRINT_BUF_SIZE  (100) // TODO: make actual length
static char* DIGIT_LUT = "0123456789abcdef";

typedef long long ll;
typedef unsigned long long ull;

void print_char(char c);
void print_str(const char *str);
void print_integer(ll i);
void print_unsigned_integer(ull i);
void print_long_hex(unsigned long l, bool prefix);

int printk(const char *fmt, ...) {
  va_list arg; 
  va_start(arg, fmt); 

  for (size_t i = 0; fmt[i] != '\0'; i++) {
    if (fmt[i] != '%') {
      if (fmt[i] == '\0') goto printk_end;
      print_char(fmt[i]);
    } 
    else {
      switch (fmt[++i]) {
        case 'c':
          print_char((int)(va_arg(arg, int)));
          break;

        case 's':
          print_str((char*)(va_arg(arg, char*)));
          break;

        case 'd':
          print_integer((int)(va_arg(arg, int)));
          break;

        case 'u':
          print_integer((unsigned int)(va_arg(arg, unsigned int)));
          break;

        case 'x':
        case 'p':
          print_long_hex((unsigned long)(va_arg(arg, unsigned long)), fmt[i] == 'p');
          break;

        case 'h':
          switch (fmt[++i]) {
            case 'd':
              print_integer((short)(va_arg(arg, int)));
              break;
  
            case 'u':
              print_integer((unsigned short)(va_arg(arg, unsigned int)));
              break;
          }
          break;
        
        case 'q':
        case 'l':
          switch (fmt[++i]) {
            case 'd':
              print_integer((ll)(va_arg(arg, ll)));
              break;
  
            case 'u':
              print_unsigned_integer((ull)(va_arg(arg, ull)));
              break;
          }
          break;

        default:
          print_char(fmt[i]);
      }
    }
  }

  printk_end:
  va_end(arg); 

  return 0;
}

inline void print_char(char c) {
  VGA_display_char(c);
}

inline void print_str(const char* str) {
  VGA_display_str(str);
}

void print_integer(ll i) {
  char buf[PRINT_BUF_SIZE];
  char* p_buf = &buf[PRINT_BUF_SIZE - 1];
  *p_buf = '\0';

  unsigned long u;

  if (i < 0) {
    print_char('-');
    u = (unsigned long)(-(i + 1)) + 1;
  } else {
    u = (unsigned long)i;
  }

  if (u == 0) {
    print_char('0');
    return;
  }

  while (u > 0) {
    p_buf--;
    *p_buf = DIGIT_LUT[u % 10];
    u /= 10;
  }

  print_str(p_buf);
}

void print_unsigned_integer(ull i) {
  char buf[PRINT_BUF_SIZE];
  char* p_buf = &buf[PRINT_BUF_SIZE - 1];
  *p_buf = '\0';

  if (i == 0) {
    print_char('0');
    return;
  }

  while (i > 0) {
    p_buf--;
    *p_buf = DIGIT_LUT[i % 10];
    i /= 10;
  }

  print_str(p_buf);

}

void print_long_hex(unsigned long l, bool prefix) {
  char buf[PRINT_BUF_SIZE];

  char* p_buf = &buf[PRINT_BUF_SIZE - 1];
  *p_buf = '\0';

  if (prefix) {
    print_str("0x");
  }

  if (l == 0) {
    print_char('0');
    return;
  } 

  while (l) {
    p_buf--;
    *p_buf = DIGIT_LUT[l & 0xf];
    l >>= 4;
  }

  print_str(p_buf);
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
