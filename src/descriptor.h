#ifndef DESCRIPTOR_H
#define DESCRIPTER_H

#define NUM_DESCRIPTORS         (8)
#define IST_STACK_SIZE          (1 << 12)
#define DF_IST                  (1)
#define PF_IST                  (2)
#define GP_IST                  (3)

void add_descriptor();
void init_gdt(void);
void check_ist_stack(int ist);

#endif /* descriptor.h */