/*
 * interrupt.h
 * (c) Jimmy Larsson 1998
 * Abstraction of the IDT and other shit to "interrupt-handlers"
 * for Fusion
 *
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "types.h"


/** Page fault error code handling constants */ 
#define PAGE_FAULT_ERROR_MASK  0x7
#define PAGE_FAULT_P           0x1
#define PAGE_FAULT_RW          0x2
#define PAGE_FAULT_US          0x4



/*
 * initialize everything concerning interrupts
 *
 */

void init_interrupts (void);


/*
 * register_irq_handler
 * Sets up an handler for IRQ's
 * returns nonzero on failure (eg. a handler already registered)
 *
 */

int register_irq_handler (irq_handler_t   handler, int   irq);


#endif


