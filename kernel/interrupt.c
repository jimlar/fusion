/*
 * interrupt.c
 * (c) Jimmy Larsson 1998
 * Abstraction of the IDT and other shit to "interrupt-handlers" and "exception-handlers"
 * for Fusion
 *
 */

#include "printk.h"
#include "types.h"
#include "panic.h"
#include "interrupt.h"
#include "config.h"
#include "idt.h"
#include "8259.h"
#include "process.h"
#include "util.h"
#include "../mm/vm.h"


/* constants */
#define NO_OF_IRQS 16

/* Table with interrupt handling routines */
irq_handler_t    irq_handler_table[NO_OF_IRQS];

/* Prototypes for local functions */
static int default_irq_handler (int irq);

/*
 * initialize everything concerning interrupts
 *
 */

void init_interrupts (void)
{
  int i;

  init_idt();
  init_8259();

  for (i = 0; i < NO_OF_IRQS; i++)
  {
    irq_handler_table[i] = default_irq_handler;
  }
}


/*
 * register_irq_handler
 * Sets up an handler for IRQ's
 * returns nonzero on failure (eg. a handler already registered)
 *
 */

int register_irq_handler (irq_handler_t  handler, int   irq)
{
  /* valid IRQ# ? */
  if (irq < 0 || irq >= NO_OF_IRQS)
    return 1;

  /* reregistering different handler NOT allowed, would mess up everything */
  if (irq_handler_table[irq] != default_irq_handler && irq_handler_table[irq] != handler)
    return 2;

  disable_8259_irq (irq);
  irq_handler_table[irq] = handler;
  enable_8259_irq (irq);

  return 0;
}

/*
 * default interrupt handler
 * Just print some shit and die, cause this shall never happen!
 *
 */

static int default_irq_handler (int irq)
{
  
  switch (irq)
  {
  case 0:
    panic ("Spurious interrupt 0!");
    break;
  case 1:
    panic ("Spurious interrupt 1!");
    break;
  case 2:
    panic ("Spurious interrupt 2!");
    break;
  case 3:
    panic ("Spurious interrupt 3!");
    break;
  case 4:
    panic ("Spurious interrupt 4!");
    break;
  case 5:
    panic ("Spurious interrupt 5!");
    break;
  case 6:
    panic ("Spurious interrupt 6!");
    break;
  case 7:
    panic ("Spurious interrupt 7!");
    break;
  case 8:
    panic ("Spurious interrupt 8!");
    break;
  case 9:
    panic ("Spurious interrupt 9!");
    break;
  case 10:
    panic ("Spurious interrupt 10!");
    break;
  case 11:
    panic ("Spurious interrupt 11!");
    break;
  case 12:
    panic ("Spurious interrupt 12!");
    break;
  case 13:
    panic ("Spurious interrupt 13!");
    break;
  case 14:
    panic ("Spurious interrupt 14!");
    break;
  case 15:
    panic ("Spurious interrupt 15!");
    break;
  }

  return 0;
}


/*
 * master irq handler
 * This dispatches all interrupts and calls the real handlers
 *
 */

void master_irq_handler (long irq)
{
  irq_handler_table[irq](irq);

  /* Enable interrupts again, this will be fixed up
   * so that we enable every other interrupt first, then just enable
   * this particular interrupt at end here
   */

  /*
   * should this be here?
   */
  schedule();


  /* Send EOI to controllers */
  eoi_8259 ((int) irq);
}


/*
 * master exception handler
 * This dispatches all exeptions and calls handlers
 *
 */

void master_exception_handler (long exception, long error)
{
  switch (exception)
  {
  case 0:
    panic ("Exception: divide error!");
  case 1:
    panic ("Exception: debug!");
  case 2:
    panic ("Exception: NMI!");
  case 3:
    panic ("Exception: breakpoint!");
  case 4:
    panic ("Exception: overflow!");
  case 5:
    panic ("Exception: bounds check!");
  case 6:
    panic ("Exception: invalid opcode!");
  case 7:
    panic ("Exception: coprocessor not availiable!");
  case 8:
    panic ("Exception: double fault!");
  case 9:
    panic ("Exception: 9 (reserved)!");
  case 10:
    panic ("Exception: invalid TSS!");
  case 11:
    panic ("Exception: segment not present!");
  case 12:
    panic ("Exception: stack!");
  case 13:
    panic ("Exception: general protection!");
  case 14:

    //  
    //printf ("PF(pid=%u,", (int) current_proc->pid);
    //
    //    if (error & PAGE_FAULT_P)
    //  printf ("present,");
    //else
    //  printf ("not present,");
    //
    //if (error & PAGE_FAULT_RW)
    //  printf ("write,");
    //else
    //  printf ("read,");
    //  
    //if (error & PAGE_FAULT_US)
    //  printf ("user,");
    //else
    //  printf ("supervisor,");
    //
    //printf ("0x%x) ", (int) get_cr2());
    //

    do_page_fault (current_proc, get_cr2(), error);

    break;

  case 16:
    panic ("Exception: coprocessor error!");
  case BAD_INT:
    warning ("Unallowed int# used!");

  }
}











