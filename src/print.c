#include "print.h"

#include "vga.h"
#include <stdarg.h>
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>

#define PRINT_BUF_SIZE  (100) // TODO: make actual length
static char* DIGIT_LUT = "0123456789abcdef";
static char* DIGIT_LUT_CAP = "0123456789ABCDEF";

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

            case 'x':
              print_long_hex((unsigned short)(va_arg(arg, unsigned int)), false);
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
            
            case 'x':
              print_long_hex((ull)(va_arg(arg, ull)), false);
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
    *p_buf = prefix ? DIGIT_LUT_CAP[l & 0xf] : DIGIT_LUT[l & 0xf];
    l >>= 4;
  }

  print_str(p_buf);
}


