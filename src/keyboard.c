#include "keyboard.h"

#include "print.h"
#include "portIO.h"
#include <stdbool.h>

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

void ps2_ctrl_init() {
  printk("[KEYBOARD] Initing ps2 controller\n");
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
  
  printk("[KEYBOARD] Done initing PS2 Controller\n");
}

void keyboard_init() {
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
}

char get_key() {
  wait_output_set();
  uint8_t scan_code = inb(PS2_DATA);

  return scan_code;
}
