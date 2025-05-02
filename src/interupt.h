#ifndef INTERUPT_H
#define INTERUPT_H

#include <stdint.h>

#define IDT_MAX_DESCRIPTORS     (256)
#define GDT_OFFSET_KERNEL_CODE  (8)
#define NUM_IRQS                (IDT_MAX_DESCRIPTORS)

static inline void cli() {
  __asm__ volatile ("cli");
} 

static inline void sti() {
  __asm__ volatile ("sti");
}

typedef void (*irqHandler_t)(void*);

void register_irq(uint8_t intr_num, irqHandler_t callback, void* state);
void init_idt(void);

#endif /* interupt.h */