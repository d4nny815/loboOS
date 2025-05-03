/****************************************************************************
 *  module_name.c
 ****************************************************************************/

/* === Public headers ===================================================== */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* === Project headers ==================================================== */
#include "interupt.h"
#include "descriptor.h"
#include "print.h"
#include "portIO.h"

/* === Private (module-only) headers ====================================== */

/* === Compile-time configuration macros ================================= */
#define DOUBLE_FAULT_INTR       (8)
#define PAGE_FAULT_INTR         (14)
#define GEN_PROC_FAULT_INTR     (13)

/* === Diagnostics switches (undef to disable) ============================ */

/* === File-scope constants ============================================== */

/* === File-scope types =================================================== */
typedef struct {
  uint16_t isr_low;
  uint16_t target_sel;
  uint16_t ist      : 3;
  uint16_t res1     : 5;
  uint16_t type     : 4;
  uint16_t zero     : 1;
  uint16_t dpl      : 2;
  uint16_t present  : 1;
  uint16_t isr_mid;
  uint32_t isr_high;
  uint32_t res2;
} __attribute__((packed)) idtEntry_t;

typedef struct {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) IDTR_t;

typedef struct {
  irqHandler_t callback;
  void *cb_arg;
} irqTableEntry_t;

/* === File-scope (static) variables ===================================== */
irqTableEntry_t irq_table[IDT_MAX_DESCRIPTORS] = {0};
idtEntry_t idt[IDT_MAX_DESCRIPTORS] __attribute__((aligned(0x10))) ;
IDTR_t idescr;
static bool vectors[IDT_MAX_DESCRIPTORS];
extern uintptr_t isr_stub_table[];

/* === Forward (static) function prototypes ============================== */
static void idt_set_descriptor(uint8_t vector, uintptr_t isr_addr);

/* === Public function definitions ======================================= */
void init_idt() {
  idescr.base = (uintptr_t)idt;
  idescr.limit = sizeof(idt) - 1;

  for (int i = 0; i < IDT_MAX_DESCRIPTORS; i++) {
      idt_set_descriptor(i, isr_stub_table[i]);
      vectors[i] = true;
  }

  __asm__ volatile ("lidt %0" : : "m"(idescr));

  printk("[INTERUPTS] IDT initialized\n");
}

void register_irq(uint8_t intr_num, irqHandler_t callback, void* arg) {
  irq_table[intr_num].callback = callback;
  irq_table[intr_num].cb_arg = arg;
}

/* === Private (static) function definitions ============================= */
void c_irq_handler(uint8_t intr_num, uint32_t err_code) {
  if (irq_table[intr_num].callback == NULL) {
    printk("[C_IRQ_HANDLER] BAD interrupt\n\
      \tintr : %hu, err : %u\n", intr_num, err_code);

    cli();
    __asm__ volatile ("hlt");
    while (1);  // SPIN
  }

  printk("[C IRQ HANDLER] handling %hu with err: %u\n", intr_num, err_code);
  irq_table[intr_num].callback(irq_table[intr_num].cb_arg);
}

static void idt_set_descriptor(uint8_t intr_num, uintptr_t isr_addr) {
  idtEntry_t* descriptor = &idt[intr_num];

  descriptor->isr_low     = isr_addr & 0xFFFF;
  descriptor->target_sel  = GDT_OFFSET_KERNEL_CODE;
  
  switch (intr_num) {
    case DOUBLE_FAULT_INTR:
      descriptor->ist = DF_IST;
      break;
    case GEN_PROC_FAULT_INTR:
      descriptor->ist = GP_IST;
      break;
    case PAGE_FAULT_INTR:
      descriptor->ist = PF_IST;
      break;
    default:
      descriptor->ist = 0;
  }
  
  descriptor->res1        = 0;
  descriptor->type        = 0xE;
  descriptor->zero        = 0;
  descriptor->dpl         = 0; 
  descriptor->present     = 1;
  descriptor->isr_mid     = (isr_addr >> 16) & 0xFFFF;
  descriptor->isr_high    = (isr_addr >> 32) & 0xFFFFFFFF;
  descriptor->res2        = 0;
}

/* === End of file ======================================================= */
