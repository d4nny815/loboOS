/****************************************************************************
 *  keyboard.c
 ****************************************************************************/

/* === Public headers ===================================================== */
#include <stdbool.h>

/* === Project headers ==================================================== */
#include "keyboard.h"
#include "print.h"
#include "portIO.h"

/* === Private (module-only) headers ====================================== */


/* === Compile-time configuration macros ================================= */
#define PS2_DATA            (0x60)
#define PS2_CMD             (0x64)
#define PS2_STATUS          (PS2_CMD)
#define PS2_STATUS_OUTPUT   (1)
#define PS2_STATUS_INPUT    (1 << 1)

#define KB_ACK              (0xFA)
#define KB_RESEND           (0xFE)
#define KB_TESTPASSED       (0xAA)

/* === Diagnostics switches (undef to disable) ============================ */
// #if defined(DEBUG) && DEBUG
// #   define DBG(fmt, ...) printf(MODULE_TAG ": " fmt "\n", ##__VA_ARGS__)
// #else
// #   define DBG(...)      do {} while (0)
// #endif

/* === File-scope constants ============================================== */
static const char scancode_map_unshifted[128] = {
  [0x1C] = 'a',
  [0x32] = 'b',
  [0x21] = 'c',
  [0x23] = 'd',
  [0x24] = 'e',
  [0x2B] = 'f',
  [0x34] = 'g',
  [0x33] = 'h',
  [0x43] = 'i',
  [0x3B] = 'j',
  [0x42] = 'k',
  [0x4B] = 'l',
  [0x3A] = 'm',
  [0x31] = 'n',
  [0x44] = 'o',
  [0x4D] = 'p',
  [0x15] = 'q',
  [0x2D] = 'r',
  [0x1B] = 's',
  [0x2C] = 't',
  [0x3C] = 'u',
  [0x2A] = 'v',
  [0x1D] = 'w',
  [0x22] = 'x',
  [0x35] = 'y',
  [0x1A] = 'z',
  [0x45] = '0',
  [0x16] = '1',
  [0x1E] = '2',
  [0x26] = '3',
  [0x25] = '4',
  [0x2E] = '5',
  [0x36] = '6',
  [0x3D] = '7',
  [0x3E] = '8',
  [0x46] = '9',
  [0x0E] = '`',
  [0x4E] = '-',
  [0x55] = '=',
  [0x5D] = '\\',
  [0x66] = '\b',
  [0x0D] = '\t',
  [0x29] = ' ',
  [0x5A] = '\n',
  [0x54] = '[',
  [0x5B] = ']',
  [0x4C] = ';',
  [0x52] = '\'',
  [0x41] = ',',
  [0x49] = '.',
  [0x4A] = '/'
};

static const char scancode_map_shifted[128] = {
  [0x1C] = 'A',
  [0x32] = 'B',
  [0x21] = 'C',
  [0x23] = 'D',
  [0x24] = 'E',
  [0x2B] = 'F',
  [0x34] = 'G',
  [0x33] = 'H',
  [0x43] = 'I',
  [0x3B] = 'J',
  [0x42] = 'K',
  [0x4B] = 'L',
  [0x3A] = 'M',
  [0x31] = 'N',
  [0x44] = 'O',
  [0x4D] = 'P',
  [0x15] = 'Q',
  [0x2D] = 'R',
  [0x1B] = 'S',
  [0x2C] = 'T',
  [0x3C] = 'U',
  [0x2A] = 'V',
  [0x1D] = 'W',
  [0x22] = 'X',
  [0x35] = 'Y',
  [0x1A] = 'Z',
  [0x45] = ')',
  [0x16] = '!',
  [0x1E] = '@',
  [0x26] = '#',
  [0x25] = '$',
  [0x2E] = '%',
  [0x36] = '^',
  [0x3D] = '&',
  [0x3E] = '*',
  [0x46] = '(',
  [0x0E] = '~',
  [0x4E] = '_',
  [0x55] = '+',
  [0x5D] = '|',
  [0x0D] = '\t',
  [0x66] = '\b',
  [0x29] = ' ',
  [0x5A] = '\n',
  [0x54] = '{',
  [0x5B] = '}',
  [0x4C] = ':',
  [0x52] = '"',
  [0x41] = '<',
  [0x49] = '>',
  [0x4A] = '?'
};

/* === File-scope types =================================================== */
// typedef struct {int x};

/* === File-scope  variables ===================================== */
// static int x;

/* === private function prototypes ============================== */
static void ps2_ctrl_init();
static char get_scancode();
static inline void wait_output_set();
static inline void wait_input_clear();
static inline void keyboard_send_cmd(uint8_t cmd);

/* === Public function definitions ======================================= */
void keyboard_init() {
  printk("[KEYBOARD] Initing keyboard driver\n");

  // init ps2 ctrl
  ps2_ctrl_init();

  // reset keyboard
  wait_input_clear();
  outb(PS2_DATA, 0xFF);

  wait_output_set();
  uint8_t ack = inb(PS2_DATA);
  
  wait_output_set();
  uint8_t self_test = inb(PS2_DATA);
  
  if (ack != KB_ACK || self_test != KB_TESTPASSED) {
    printk("[KEYBOARD] Keyboard self test failed\n");
    return;
  }

  // set keyboard to scan code
  keyboard_send_cmd(0xF0);
  keyboard_send_cmd(0x2);
  
  // enable keyboard
  keyboard_send_cmd(0xF4);

  printk("[KEYBOARD] Done initing Keyboard\n");
}

char get_key() {
  static bool shift_flag = false;
  
  while (1) {
    uint8_t scan_code = get_scancode();

    if (scan_code == 0x12) {
      shift_flag = true;
      continue;
    } 
    if (scan_code == 0xf0) {
      scan_code = get_scancode();
      if (scan_code == 0x12) shift_flag = false;
      continue;
    }

    if (shift_flag) {
      return scancode_map_shifted[scan_code];
    } else {
      return scancode_map_unshifted[scan_code];
    }
  }
  // printk("%hx     ", scan_code);
}

/* === Private (static) function definitions ============================= */
static inline void wait_input_clear() {
  while (inb(PS2_STATUS) & PS2_STATUS_INPUT);
}

static inline void wait_output_set() {
  while (!(inb(PS2_STATUS) & PS2_STATUS_OUTPUT));
}

static inline void keyboard_send_cmd(uint8_t cmd) {
  uint8_t ack;
  do {
    wait_input_clear();
    outb(PS2_DATA, cmd);
    
    wait_output_set();
    ack = inb(PS2_DATA);

  } while (ack == KB_RESEND);
}

static void ps2_ctrl_init() {
  printk("[KEYBOARD] Initing ps2 controller\n");

  // disable ps2 port1
  wait_input_clear();
  outb(PS2_CMD, 0xAD);
  
  // disable ps2 port2
  wait_input_clear();
  outb(PS2_CMD, 0xA7);
  
  // flush output buffer
  if (inb(PS2_STATUS) & PS2_STATUS_OUTPUT) inb(PS2_DATA);

  // write to control port to force ps2 status
  wait_input_clear(); 
  outb(PS2_CMD, 0x20);

  // read config byte
  wait_output_set();
  uint8_t config_byte = inb(PS2_DATA);

  // bit crunch
  config_byte = (1 << 1) | (1 << 6);
  
  // write config byte
  wait_input_clear();
  outb(PS2_CMD, 0x60);

  wait_input_clear();
  outb(PS2_DATA, config_byte);

  // controller self test
  wait_input_clear();
  outb(PS2_CMD, 0xAA);

  wait_output_set();
  uint8_t test_ctrl = inb(PS2_DATA);
  if (test_ctrl != 0x55) {
    printk("[KEYBOARD] Failed PS2 Controller Test\n");
    return;
  }

  // interface test
  wait_input_clear();
  outb(PS2_CMD, 0xAB);

  wait_output_set();
  uint8_t test_itf = inb(PS2_DATA);
  if (test_itf != 0x00) {
    printk("[KEYBOARD] Failed PS2 interface Port1\n");
    return;
  }

  // Enable PS/2 port 1
  wait_input_clear();
  outb(PS2_CMD, 0xAE);
  
  printk("[KEYBOARD] Done initing PS2 Controller\n");
}



static char get_scancode() {
  wait_output_set();
  uint8_t scan_code = inb(PS2_DATA);

  return scan_code;  
}

/* === End of file ======================================================= */
