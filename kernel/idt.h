/*
 * idt.h
 *
 * Support functions for managing IDT structures
 * (c) Jimmy Larsson 1998
 *
 */

#ifndef IDT_H
#define IDT_H

#include "config.h"
#include "../mm/pages.h"

/* Where TSS'es for int's are */
#define INT_TSS_START_PAGE  1

#define INT_TSS_NUM_PAGES  (((sizeof (tss_386_t) * (NO_OF_IRQS + NO_OF_EXCEPTIONS + 1)) % PAGE_SIZE) ?\
                    ((sizeof (tss_386_t) * (NO_OF_IRQS + NO_OF_EXCEPTIONS + 1)) / PAGE_SIZE) + 1 : \
                    ((sizeof (tss_386_t) * (NO_OF_IRQS + NO_OF_EXCEPTIONS + 1)) / PAGE_SIZE))


/* Descriptor constants */
#define D_PRESENT    0x80
#define D_DPL3       0x60
#define D_DPL2       0x40
#define D_DPL1       0x20
#define D_DPL0       0x00
#define D_INT_TYPE   0x0e
#define D_TRAP_TYPE  0x0f
#define D_TASK_TYPE  0x05
#define D_TSS        0x09
#define D_TSS_BUSY   0x02

/* signals bad int */
#define BAD_INT      0x1024

/* system call vector */
#define SYSCALL_VECTOR 0x40


#define IDT_SIZE    256*8
extern gate_t         _idt;


/*
 * external functions from idt_s.S
 *
 */

extern void __hw_irq_0 (void);
extern void __hw_irq_1 (void);
extern void __hw_irq_2 (void);
extern void __hw_irq_3 (void);
extern void __hw_irq_4 (void);
extern void __hw_irq_5 (void);
extern void __hw_irq_6 (void);
extern void __hw_irq_7 (void);
extern void __hw_irq_8 (void);
extern void __hw_irq_9 (void);
extern void __hw_irq_10 (void);
extern void __hw_irq_11 (void);
extern void __hw_irq_12 (void);
extern void __hw_irq_13 (void);
extern void __hw_irq_14 (void);
extern void __hw_irq_15 (void);

extern void __exception_0 (void);
extern void __exception_1 (void);
extern void __exception_2 (void);
extern void __exception_3 (void);
extern void __exception_4 (void);
extern void __exception_5 (void);
extern void __exception_6 (void);
extern void __exception_7 (void);
extern void __exception_8 (void);
extern void __exception_9 (void);
extern void __exception_10 (void);
extern void __exception_11 (void);
extern void __exception_12 (void);
extern void __exception_13 (void);
extern void __exception_14 (void);
extern void __exception_16 (void);

extern void __bad_int (void);
extern void __system_call_entry (void);

/*
 * boot-time init. of idt
 *
 */

void init_idt (void);


/* 
 * Setup one IDT entry 
 * 
 */

void set_int_gate (void (*handler)(void), int int_no, int dpl);


/*
 * set_task_int_gate
 * Sets up a Task gate and and a TSS descriptor for serving and interrupt
 * 
 */

void set_task_int_gate (void (*handler)(void), int int_no, int dpl, tss_386_t  *tss, int  gdt_index);

#endif



