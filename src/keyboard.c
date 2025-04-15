#include "keyboard.h"

#include "print.h"
#include "portIO.h"


static inline uint8_t poll_status(uint8_t bit) {
  uint8_t status = inb(PS2_STATUS);
  while (!(status & bit)) {
    status = inb(PS2_STATUS);
  }
  return inb(PS2_STATUS);
}

static inline void wait_input_clear() {
  while (inb(PS2_STATUS) & PS2_STATUS_INPUT);
}

static inline void wait_output_set() {
  while (!(inb(PS2_STATUS) & PS2_STATUS_OUTPUT));
}

void ps2_ctrl_init() {
  printk("[KEYBOARD] initing ps2 controller\n");
  // ? do i check if ps2 controller exists even on qemu?

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
  config_byte &= ~(1 << 0); // clear first port interrupt
  config_byte &= ~(1 << 1); // clear second port interrupt
  config_byte &= ~(1 << 6); // clear translation

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
  
  printk("[KEYBOARD] Done initializing PS2 Controller\n");
}

void keyboard_init() {
  ps2_ctrl_init();
  // init ps2 ctrl

  // reset keyboard
  
  // set keyboard to scan code

  // enable keyboard
}