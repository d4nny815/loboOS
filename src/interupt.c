/****************************************************************************
 *  module_name.c
 ****************************************************************************/

/* === Public headers ===================================================== */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* === Project headers ==================================================== */
#include "interupt.h"
#include "print.h"
#include "portIO.h"

/* === Private (module-only) headers ====================================== */

/* === Compile-time configuration macros ================================= */
#define PIC1_CMD        (0x20)
#define PIC1_DATA       (0x21)
#define PIC2_CMD        (0xA0)
#define PIC2_DATA       (0xA1)

#define PIC_EOI		      (0x20)


#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */


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
static uint16_t __pic_get_irq_reg(int ocw3);
static void PIC_sendEOI(uint8_t irq);

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

void pic_disable(void) {
  outb(PIC1_DATA, 0xff);
  outb(PIC2_DATA, 0xff);
}

void dummy() {
  printk("PIC intr\n");
}

void PIC_remap(int offset1, int offset2) {
	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(PIC2_DATA, ICW4_8086);

	// Unmask both PICs.
	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);

  register_irq(offset1, &dummy, NULL);
  register_irq(offset2, &dummy, NULL);

}


void register_irq(uint8_t intr_num, irqHandler_t callback, void* state) {
  irq_table[intr_num].callback = callback;
  irq_table[intr_num].cb_arg = state;
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}

/* === Private (static) function definitions ============================= */
void c_irq_handler(uint8_t intr_num, uint32_t err_code) {
  if (irq_table[intr_num].callback == NULL) {
    printk("[C_IRQ_HANDLER] BAD interrupt\n\
      \tintr : %hu, err : %u\n", intr_num, err_code);

      while (1);  // SPIN
  }

  irq_table[intr_num].callback(intr_num, err_code, irq_table[intr_num].cb_arg);
  
  PIC_sendEOI(intr_num);

}

static void idt_set_descriptor(uint8_t intr_num, uintptr_t isr_addr) {
  idtEntry_t* descriptor = &idt[intr_num];

  descriptor->isr_low     = isr_addr & 0xFFFF;
  descriptor->target_sel  = GDT_OFFSET_KERNEL_CODE;
  descriptor->ist         = 0;
  descriptor->res1        = 0;
  descriptor->type        = 0xE;
  descriptor->zero        = 0;
  descriptor->dpl         = 0; 
  descriptor->present     = 1;
  descriptor->isr_mid     = (isr_addr >> 16) & 0xFFFF;
  descriptor->isr_high    = (isr_addr >> 32) & 0xFFFFFFFF;
  descriptor->res2        = 0;
}

static void PIC_sendEOI(uint8_t irq) {
  if(irq >= 8) {
    outb(PIC2_CMD, PIC_EOI);
  } 
    
  outb(PIC1_CMD, PIC_EOI);
}

static uint16_t __pic_get_irq_reg(int ocw3) {
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_CMD, ocw3);
    outb(PIC2_CMD, ocw3);
    return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}




/* === End of file ======================================================= */
