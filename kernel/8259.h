/*
 * 8259 - Interrupt controller functions for Fusion
 * (c) Jimmy Larsson
 * 
 */


#ifndef H_8259_H
#define H_8259_H

/* Where in IDT the IRQ's are */
#define IRQ0_VECTOR  0x28
#define IRQ8_VECTOR  0x30

/* init. the hardware crap */
void init_8259 (void);


/*
 * enable_8259_irq
 * Unmask an irq on the controller
 *
 */

void enable_8259_irq (int irq);


/*
 * disable_8259_irq
 * mask off an irq on the controller
 *
 */

void disable_8259_irq (int irq);


/*
 * eoi_8259_irq
 * EndOfInterrupt, finished with interrupt handling
 * send ack. to controllers (eg. turn them on again)
 */

void eoi_8259 (int irq);


#endif
