#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stddef.h>

#include "vga.h"
#include "print.h"
#include "keyboard.h"
#include "interupt.h"
#include "pic.h"
#include "portIO.h"
#include "descriptor.h"

void cause_pf_fault();
void cause_gpf_fault();
void cause_df_fault();

void df_isr(void* args) {
  (void)args;
  printk("[EXCEPTION] Double Fault\n");
  __asm__ volatile ("hlt");
}

void pf_isr(void* args) {
  (void)args;
  uintptr_t cr2;
  __asm__ volatile("mov %0, cr2" : "=r"(cr2));
  printk("[EXCEPTION] Page Fault at address 0x%lx\n", cr2);
  check_ist_stack(PF_IST);
}

void gpf_isr(void* args) {
  (void)args;
  printk("[EXCEPTION] General Protection Fault\n");
  const size_t PF_ADDR = 0x00007FFFFFFFF000;
  *((volatile int*)(PF_ADDR)) = 0;
  check_ist_stack(GP_IST);
}



bool db_flag = 0;
void kmain(void) {
  
  // init vga
  VGA_clear();

  // init gdt
  init_gdt();

  // init idt
  init_idt();

  // init pic
  init_PICs();

  // init keyboard
  keyboard_init();
  
  register_irq(DF_INTR_NUM, &df_isr, NULL);
  register_irq(GPF_INTR_NUM, &gpf_isr, NULL);
  register_irq(PF_INTR_NUM, &pf_isr, NULL);

  while (!db_flag);

  sti();
  printk("[KERNEL] interupts enabled\n");
  
  
  printk("[KERNEL] Done initializing\n");

  // cause_pf_fault();
  // cause_gpf_fault();
  cause_df_fault();
  
  // *((volatile int*)(0x00007FFFFFFFF000)) = 0;
  
  // __asm__ volatile ("hlt");
  while (1);
}

void cause_pf_fault() {
  const size_t PF_ADDR = 0x00007FFFFFFFF000;
  *((volatile int*)(PF_ADDR)) = 0;
}

void cause_gpf_fault() {
  __asm__ volatile (
    "mov ax, 0x23\n\t"  // Invalid or ring-3 selector
    "mov ss, ax\n\t"    // Causes GPF in ring 0
  );
}

void cause_df_fault() {
  // Trigger a fault in GPF handler to cause a double fault
  cause_gpf_fault();  // The GPF handler then triggers a PF
}