/*
 * switching.c
 * Support for switching to a tss
 * (c) 1998 Jimmy Larsson
 *
 *
 */

#include "types.h"
#include "config.h"
#include "gdt.h"
#include "idt.h"
#include "switching.h"
#include "printk.h"
#include "util.h"

/* private variables */
static tss_descriptor_t   *gdt;
static tss_386_t  tss0;

/*
 * initialize switching module
 *
 */

void init_switching (void)
{
  short tss_selector = TSS_SELECTOR;

  gdt = (tss_descriptor_t *) &_gdt;

  gdt[TSS_INDEX].g_limit_high = 0;
  gdt[TSS_INDEX].limit_low = sizeof (tss0) - 1;
  gdt[TSS_INDEX].p_dpl_type = D_TSS | D_PRESENT;

  /* setup an initial tss */
  gdt[TSS_INDEX].base_low = (short int) &tss0 & 0xffff;
  gdt[TSS_INDEX].base_mid = (unsigned char) (((long) &tss0) >> 16) & 0xff;
  gdt[TSS_INDEX].base_high = (unsigned char) (((long) &tss0) >> 24) & 0xff;

  tss0.backlink = TSS_SELECTOR;
  tss0.cr3 = get_cr3();

  /* load task register */
  asm ("ltr %0": :"r" (tss_selector));

}


/* 
 * switch task
 *
 */

void switch_task (tss_386_t   *tss)
{
  unsigned int  selector[2];

  selector[1] = TSS_SELECTOR; 
  /* selector[0] (offset) is irrelevant for tasks */

  load_task (tss, BUSY_CLEAR);

  asm ("ljmp %0": :"m" (*selector));

  /* when this task gets switched back to we get here */
}


/* 
 * load task, prepare a task to be switched
 *
 */

void load_task (tss_386_t  *tss, int busy)
{
  unsigned int  selector[2];

  selector[1] = TSS_SELECTOR; 
  /* selector[0] (offset) is irrelevant for tasks */

  /* this should already be done */
  //tss->backlink = TSS_SELECTOR;

  /* modify GDT entry to point to new task */
  gdt[TSS_INDEX].base_low = (short int) ((long)tss & 0xffff);
  gdt[TSS_INDEX].base_mid = (unsigned char) (((long)tss) >> 16) & 0xff;
  gdt[TSS_INDEX].base_high = (unsigned char) (((long)tss) >> 24) & 0xff;

  /* remove busy bits */
  if (busy == BUSY_CLEAR)
  {
    gdt[TSS_INDEX].p_dpl_type = gdt[TSS_INDEX].p_dpl_type & ~D_TSS_BUSY;
 
  } else if (busy == BUSY_SET)
  {
    gdt[TSS_INDEX].p_dpl_type = gdt[TSS_INDEX].p_dpl_type | D_TSS_BUSY;
  }

 
  /* set NT flag to cause switching to correct task on next iretd (assumes backlink correct) */
  __asm__("pushf
           orl   $0x00004000, (%esp)
           popf");

}

/*
 * load idle task
 * Prepare to idle
 *
 */


void load_idle_task (int busy)
{
  load_task (&tss0, busy);
}
