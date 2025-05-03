/****************************************************************************
 *  descriptor.c
 ****************************************************************************/

/* === Public headers ===================================================== */
#include <stdint.h>

/* === Project headers ==================================================== */
#include "descriptor.h"
#include "string.h"
#include "print.h"

/* === Private (module-only) headers ====================================== */
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

typedef struct {
  uint16_t seg_limit1;
  uint16_t base_addr1;
  uint8_t base_addr2;
  uint8_t type    : 4;
  uint8_t zero1   : 1;
  uint8_t dpl     : 2;
  uint8_t present : 1;
  uint8_t seg_limit : 4;
  uint8_t avail   : 1;
  uint8_t res1    : 2;
  uint8_t g       : 1;
  uint8_t base_addr3;
  uint32_t base_addr4;
  uint32_t res2;
} __attribute__((packed)) tssDescriptor_t;

typedef struct {
  uint32_t reserved0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap_base;
} __attribute__((packed)) TSS_t;

typedef struct {
  uint16_t length;
  uintptr_t base;
} __attribute__((packed)) GDTR_t;

/* === Compile-time configuration macros ================================= */

/* === Diagnostics switches (undef to disable) ============================ */

/* === File-scope constants ============================================== */

/* === File-scope types =================================================== */

/* === File-scope (static) variables ===================================== */
static csDescriptor_t GDT[NUM_DESCRIPTORS] = {0};
static GDTR_t gdtr;
static TSS_t tss __attribute__((aligned(16))) = {0};

static uint8_t df_stack[IST_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t pf_stack[IST_STACK_SIZE] __attribute__((aligned(16)));
static uint8_t gpf_stack[IST_STACK_SIZE] __attribute__((aligned(16)));


/* === Forward (static) function prototypes ============================== */
static void set_tss_descriptor(csDescriptor_t* gdt, int index, void* tss_ptr);


/* === Public function definitions ======================================= */
void init_gdt(void) {
  // make default code desc
  GDT[1].one1 = 1;
  GDT[1].one2 = 1;
  GDT[1].present = 1;
  GDT[1].long_mode = 1;

  // tss desc
  tss.iomap_base = sizeof(TSS_t);
  // stack grows down so move ptr to the top
  tss.ist1 = (uint64_t)(df_stack + IST_STACK_SIZE); 
  tss.ist2 = (uint64_t)(pf_stack + IST_STACK_SIZE);
  tss.ist3 = (uint64_t)(gpf_stack + IST_STACK_SIZE);

  set_tss_descriptor(GDT, 2, &tss);

  // make gdt from descs
  gdtr.base = (uintptr_t)GDT;
  gdtr.length = sizeof(GDT) - 1;

  // load gdt
  __asm__ volatile ("lgdt %0" : : "m"(gdtr));
  
  // long return and reload data seg regs
  __asm__ volatile (
    "push 0x08\n\t"               // Push code segment to stack, 0x08 is a stand-in for your code segment
    "lea rax, [rip + 1f]\n\t"     // Load address of label 1 into RAX
    "push rax\n\t"
    "lretq\n\t"                   // Far return to reload CS
    
    "1:\n\t"                      // Label 1: reload data segments
    "mov ax, 0\n\t"               // Data Segment selector
    "mov ss, ax\n\t"
    "mov ds, ax\n\t"
    "mov es, ax\n\t"
    "mov fs, ax\n\t"
    "mov gs, ax\n\t"
    :
    :
    : "rax", "memory"
  );

  // load tss
  __asm__ volatile ("ltr %%ax" : : "a"(0x10));

  printk("[DESCRIPTOR] GDT and TSS initialized\n");
}

void check_ist_stack(int ist) {
  uintptr_t rsp;
  __asm__ volatile ("mov %0, rsp" : "=r"(rsp));

  uintptr_t ist_stack;
  switch (ist) {
    case DF_IST:
      ist_stack = (uintptr_t)df_stack;
      break;
    case PF_IST:
      ist_stack = (uintptr_t)pf_stack;
      break;
    case GP_IST:
      ist_stack = (uintptr_t)gpf_stack;
      break;
  } 

  if ((rsp >= ist_stack) && (rsp < (ist_stack + IST_STACK_SIZE))) {
    printk("Proper IST stack\n");
  } else {
    printk("did NOT use IST stack\n");
    __asm__ volatile ("hlt");
  }

}

/* === Private (static) function definitions ============================= */
static void set_tss_descriptor(csDescriptor_t* gdt, int index, void* tss_ptr) {
  if (index + 1 >= NUM_DESCRIPTORS) {
    printk("[ERROR] GDT: not enough space for TSS descriptor\n");
    return;
  }

  uintptr_t base = (uintptr_t)tss_ptr;
  tssDescriptor_t desc = {0};

  desc.seg_limit1   = sizeof(TSS_t) & 0xFFFF;
  desc.base_addr1   = base & 0xFFFF;
  desc.base_addr2   = (base >> 16) & 0xFF;
  desc.type         = 0x9; // 64-bit TSS (Available)
  desc.dpl          = 0;
  desc.present      = 1;
  desc.seg_limit    = (sizeof(TSS_t) >> 16) & 0xF;
  desc.g            = 0;
  desc.base_addr3   = (base >> 24) & 0xFF;
  desc.base_addr4   = (base >> 32) & 0xFFFFFFFF;
  desc.res2         = 0;

  memcpy(&gdt[index], &desc, sizeof(desc));
}

/* === End of file ======================================================= */
