#ifndef INTERUPT_H
#define INTERUPT_H

#include <stdint.h>

#define IDT_MAX_DESCRIPTORS     (256)
#define GDT_OFFSET_KERNEL_CODE  (8)
#define NUM_IRQS                (IDT_MAX_DESCRIPTORS)
#define DF_INTR_NUM             (8)
#define GPF_INTR_NUM            (13)
#define PF_INTR_NUM             (14)


static inline void __attribute__((always_inline)) cli() {
  __asm__ volatile ("cli");
} 

static inline void __attribute__((always_inline)) sti() {
  __asm__ volatile ("sti");
}

typedef void (*irqHandler_t)(void*);

void register_irq(uint8_t intr_num, irqHandler_t callback, void* state);
void init_idt(void);

#endif /* interupt.h */