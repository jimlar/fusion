/*
 * keyboard.c
 *
 * Low level keyboard code for fusion
 * (c) 1999 Jimmy Larsson
 *
 */

#include "../kernel/port_io.h"
#include "../kernel/interrupt.h"
#include "../kernel/printk.h"


/* Ports */
#define KEYB_COMMAND_PORT       0x64
#define KEYB_STATUS_PORT        0x64
#define KEYB_INPUT_BUFFER_PORT  0x60
#define KEYB_OUTPUT_BUFFER_PORT 0x60


/* Commands */
#define KEYB_COMMAND_READ_INPUT 0xc0
#define KEYB_COMMAND_SELFTEST   0xaa
#define KEYB_COMMAND_ENABLE     0xae
#define KEYB_COMMAND_DISABLE    0xad

/* Selftest expected result */
#define SELFTEST_OK             0x55

/* IRQ for keyboard */
#define KEYB_IRQ_NO             1

/* Keyboard buffer size */
#define KEYB_BUFFER_SIZE        1024


/*
 * the IRQ handler for keyboard
 *
 */

int keyboard_irq_handler (int irq)
{
  int scancode = inb (KEYB_OUTPUT_BUFFER_PORT);

  printf ("KEYBOARD IRQ! (%u) ", scancode);
  return 0;
}


/*
 * intialize keyboard module and keyboard
 *
 * returns nonzero on error
 */

int keyboard_init (void)
{
  /* Disable keyboard */
  outb (KEYB_COMMAND_DISABLE, KEYB_COMMAND_PORT);

  /* Enable keyboard */
  outb (KEYB_COMMAND_ENABLE, KEYB_COMMAND_PORT);

  register_irq_handler (keyboard_irq_handler, KEYB_IRQ_NO);

  return 0;
}
