/****************************************************************************
 *  pic.c
 ****************************************************************************/

/* === Public headers ===================================================== */

/* === Project headers ==================================================== */
#include "pic.h"
#include "portIO.h"
#include "print.h"

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

/* === Diagnostics switches (undef to disable) ============================ */

/* === File-scope constants ============================================== */

/* === File-scope types =================================================== */

/* === File-scope (static) variables ===================================== */

/* === Forward (static) function prototypes ============================== */

/* === Public function definitions ======================================= */
void init_PICs(void) {
	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
  outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, PIC_OFF1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, PIC_OFF2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	// mask both PICs
  PICs_disable();

  printk("[PIC] PICs initialized\n");
}

void PICs_disable(void) {
  outb(PIC1_DATA, 0xff);
	outb(PIC2_DATA, 0xff);
}

void PIC_enable(uint8_t irq_num) {
  uint16_t port = PIC1_DATA;
  if (irq_num > 7) {
    irq_num -= 8;
    port = PIC2_DATA;
  }
  uint8_t mask = inb(port) & ~(1 << irq_num);
  outb(port, mask);
}

void PIC_disable(uint8_t irq_num) {
  uint16_t port = PIC1_DATA;
  if (irq_num > 7) {
    irq_num -= 8;
    port = PIC2_DATA;
  }
  uint8_t mask = inb(port) | (1 << irq_num);
  outb(port, mask);
}

void PIC_sendEOI(uint8_t irq_num) {
  if(irq_num > 7) {
    outb(PIC2_CMD, PIC_EOI);
  } 
    
  outb(PIC1_CMD, PIC_EOI);
}

uint16_t PIC_get_irq_reg(int ocw3) {
  /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
   * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
  outb(PIC1_CMD, ocw3);
  outb(PIC2_CMD, ocw3);
  return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

/* === Private (static) function definitions ============================= */


/* === End of file ======================================================= */
