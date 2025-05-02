#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC_READ_IRR      (0x0a)    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR      (0x0b)    /* OCW3 irq service next CMD read */
#define PIC_OFF1          (0x20)
#define PIC_OFF2          (0x28)
#define IRQ_KEYBOARD      (1)
#define KEYBOARD_INTR_NUM (PIC_OFF1 + IRQ_KEYBOARD)

// static bool PIC_within_bounds(uint8_t intr_num) {
  // return (PIC_OFF1 <= intr_num && intr_num >= PIC_OFF2 + 7);
// }

void init_PICs(void);
uint16_t PIC_get_irq_reg(int ocw3);
void PIC_sendEOI(uint8_t irq);


void PICs_disable(void);
void PIC_enable(uint8_t irq_num);
void PIC_disable(uint8_t irq_num);

#endif /* pic.h */