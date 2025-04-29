/****************************************************************************
 *  descriptor.c
 ****************************************************************************/

/* === Public headers ===================================================== */
#include <stdint.h>

/* === Project headers ==================================================== */
#include "descriptor.h"
#include "string.h"

/* === Private (module-only) headers ====================================== */
typedef struct {
  uint16_t length;
  uintptr_t base;
} __attribute__((packed)) GDTR_t;

typedef struct {
  uint16_t seg_limit1;
  uint16_t base_addr1;
  uint8_t base_addr2;
  uint8_t a          : 1;
  uint8_t r          : 1;
  uint8_t c          : 1;
  uint8_t one1       : 1;
  uint8_t one2       : 1;
  uint8_t dpl        : 2;
  uint8_t present    : 1;
  uint8_t seg_limit  : 4;
  uint8_t avail      : 1;
  uint8_t long_mode  : 1;
  uint8_t d          : 1;
  uint8_t g          : 1;
  uint8_t base_addr3;
} __attribute__((packed)) csDescriptor_t;

/* === Compile-time configuration macros ================================= */

/* === Diagnostics switches (undef to disable) ============================ */

/* === File-scope constants ============================================== */

/* === File-scope types =================================================== */

/* === File-scope (static) variables ===================================== */
static csDescriptor_t GDT[NUM_DESCRIPTORS] = {0};
static GDTR_t gdtr;

/* === Forward (static) function prototypes ============================== */

/* === Public function definitions ======================================= */
void init_gdt(void) {
  // make default code desc
  GDT[1].one1 = 1;
  GDT[1].one2 = 1;
  GDT[1].present = 1;
  GDT[1].long_mode = 1;

  // TODO: make tss desc

  // make gdt from descs
  gdtr.base = (uintptr_t)GDT;
  gdtr.length = sizeof(GDT) - 1;

  // load gdt
  __asm__ volatile ("lgdt %0" : : "m"(gdtr));
}


/* === Private (static) function definitions ============================= */


/* === End of file ======================================================= */
