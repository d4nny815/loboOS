#ifndef KEYBOARD_H
#define KEYBOARD_H

#define PS2_DATA            (0x60)
#define PS2_CMD             (0x64)
#define PS2_STATUS          (PS2_CMD)
#define PS2_STATUS_OUTPUT   (1)
#define PS2_STATUS_INPUT    (1 << 1)

#define KB_ACK              (0xFA)
#define KB_RESEND           (0xFE)
#define KB_TESTPASSED       (0xAA)


void ps2_ctrl_init();
void keyboard_init();

char get_key();

#endif /* keyboard.h */