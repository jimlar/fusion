/*
 * 8259 - Interrupt controller functions for Fusion
 * (c) Jimmy Larsson
 * 
 */

#include "config.h"
#include "8259.h"
#include "port_io.h"
#include "printk.h"

/* Crappy 8259 constants, won't put the in .h file */
#define CASCADE_IRQ  2
/* ICW4 needed, cascade, 8 byte int. vec, edge triggered. */
#define ICW1         0x11
/* Defines which IRQ connects slave/master */
#define MASTER_ICW3  (1 << CASCADE_IRQ)
#define SLAVE_ICW3   CASCADE_IRQ
/* 80x86 mode */
#define ICW4         0x01

/* EOI Command */
#define ENABLE_INT       0x20

/* IO Ports */
#define MASTER_CTL       0x20
#define SLAVE_CTL        0xa0
#define MASTER_CTLMASK   0x21
#define SLAVE_CTLMASK    0xa1
                                

void init_8259 (void)
{
  /* Start initialization */
  outb (ICW1, MASTER_CTL);
  outb (ICW1, SLAVE_CTL);

  /* Set IRQ vectors */
  outb (IRQ0_VECTOR, MASTER_CTLMASK);
  outb (IRQ8_VECTOR, SLAVE_CTLMASK);

  /* Connect master/slave */
  outb (MASTER_ICW3, MASTER_CTLMASK);
  outb (SLAVE_ICW3, SLAVE_CTLMASK);

  /* finish initialization */
  outb (ICW4, MASTER_CTLMASK);
  outb (ICW4, SLAVE_CTLMASK);

  /* mask off all interrupts, but the cascade */
  outb (~(1 << CASCADE_IRQ), MASTER_CTLMASK);
  outb (~0, SLAVE_CTLMASK);

}


/*
 * enable_8259_irq
 * Unmask an irq on the controller
 *
 */

void enable_8259_irq (int irq)
{
  if (irq > -1 && irq < 8)
  {
    __asm__ ("pushf");
    outb ((inb (MASTER_CTLMASK) & ~(1 << irq)), MASTER_CTLMASK);
    __asm__ ("popf");
  }
  else if (irq > 7 && irq < 16)
  {
    __asm__ ("pushf");
    outb ((inb (SLAVE_CTLMASK) & ~(1 << (irq - 8))), SLAVE_CTLMASK);
    __asm__ ("popf");
  }
}                 


/*
 * disable_8259_irq
 * mask off an irq on the controller
 *
 */

void disable_8259_irq (int irq)
{
  
  if (irq > -1 && irq < 8)
  {
    __asm__ ("pushf");
    outb ((inb (MASTER_CTLMASK) | (1 << irq)), MASTER_CTLMASK);
    __asm__ ("popf");
  } else if (irq > 7 && irq < 16)
  {
    __asm__ ("pushf");
    outb ((inb (SLAVE_CTLMASK) | (1 << (irq - 8))), SLAVE_CTLMASK);
    __asm__ ("popf");
  }
}

/*
 * eoi_8259_irq
 * EndOfInterrupt, finished with interrupt handling
 * send ack. to controllers (eg. turn them on again)
 */

void eoi_8259 (int irq)
{
  if (irq > -1 && irq < 8)
  {
    outb (ENABLE_INT, MASTER_CTL);

  } else if (irq > 7 && irq < 16)
  {
    outb (ENABLE_INT, MASTER_CTL);
    outb (ENABLE_INT, SLAVE_CTL);
  }
}

