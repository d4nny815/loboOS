#ifndef KEYBOARD_H
#define KEYBOARD_H

#define PS2_DATA            (0x60)
#define PS2_CMD             (0x64)
#define PS2_STATUS          (PS2_CMD)
#define PS2_STATUS_OUTPUT   (1)
#define PS2_STATUS_INPUT    (1 << 1)

void ps2_ctrl_init();
void keyboard_init();

#endif /* keyboard.h */