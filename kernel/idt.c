/*
 * idt.c
 *
 * Support functions for managing IDT structures
 * (c) Jimmy Larsson 1998
 *
 */

#include "config.h"
#include "printk.h"
#include "types.h"
#include "idt.h"
#include "gdt.h"
#include "8259.h"
#include "panic.h"
#include "stack.h"
#include "../mm/pages.h"
#include "util.h"

/* External vars, funcs. from idt_s.S */
extern void load_idtr (void);

/* Our variables */
static gate_t   *idt;
static tss_386_t  *int_tss[NO_OF_IRQS];
static tss_386_t  *exception_tss[NO_OF_EXCEPTIONS];
static tss_386_t  *system_call_tss;


/*
 * init_idt
 * Boot-time initialization
 *
 */

void init_idt (void)
{
  int i, num_pages_needed;
  page_frame_t   *tmp_page;


  //
  // Alloc pages and put TSS'es onto them, we need to share them
  //  between all processes
  //

  num_pages_needed = INT_TSS_NUM_PAGES;

  for (i = INT_TSS_START_PAGE; i < INT_TSS_START_PAGE + num_pages_needed; i++)
  {
    tmp_page = get_page_at (i * PAGE_SIZE);
    add_to_kernel_pages (tmp_page);
  }

  for (i = 0; i < NO_OF_IRQS; i++)
  {
    int_tss[i] = (tss_386_t *) (INT_TSS_START_PAGE * PAGE_SIZE + sizeof(tss_386_t) * i);
  }

  for (i = 0; i < NO_OF_EXCEPTIONS; i++)
  {
    exception_tss[i] = (tss_386_t *) (INT_TSS_START_PAGE * PAGE_SIZE + sizeof(tss_386_t) * (i + NO_OF_IRQS));
  }

  system_call_tss = (tss_386_t *) (INT_TSS_START_PAGE * PAGE_SIZE + sizeof(tss_386_t) * (NO_OF_IRQS + NO_OF_EXCEPTIONS)); 

  /* Set pointer */
  idt = &_idt;

  /* set default to bad */
  for (i = 0; i <= 255; i++)
  {
    set_int_gate (__bad_int, i, D_DPL0);
  }

  /* add hardware irq stubs */
  set_task_int_gate (__hw_irq_0, IRQ0_VECTOR,     D_DPL0, int_tss[0], GDT_INT_START_INDEX);
  set_task_int_gate (__hw_irq_1, IRQ0_VECTOR + 1, D_DPL0, int_tss[1], GDT_INT_START_INDEX + 1);
  set_task_int_gate (__hw_irq_2, IRQ0_VECTOR + 2, D_DPL0, int_tss[2], GDT_INT_START_INDEX + 2);
  set_task_int_gate (__hw_irq_3, IRQ0_VECTOR + 3, D_DPL0, int_tss[3], GDT_INT_START_INDEX + 3);
  set_task_int_gate (__hw_irq_4, IRQ0_VECTOR + 4, D_DPL0, int_tss[4], GDT_INT_START_INDEX + 4);
  set_task_int_gate (__hw_irq_5, IRQ0_VECTOR + 5, D_DPL0, int_tss[5], GDT_INT_START_INDEX + 5);
  set_task_int_gate (__hw_irq_6, IRQ0_VECTOR + 6, D_DPL0, int_tss[6], GDT_INT_START_INDEX + 6);
  set_task_int_gate (__hw_irq_7, IRQ0_VECTOR + 7, D_DPL0, int_tss[7], GDT_INT_START_INDEX + 7);

  set_task_int_gate (__hw_irq_8,  IRQ8_VECTOR,     D_DPL0, int_tss[8],  GDT_INT_START_INDEX + 8); 
  set_task_int_gate (__hw_irq_9,  IRQ8_VECTOR + 1, D_DPL0, int_tss[9],  GDT_INT_START_INDEX + 9);
  set_task_int_gate (__hw_irq_10, IRQ8_VECTOR + 2, D_DPL0, int_tss[10], GDT_INT_START_INDEX + 10);
  set_task_int_gate (__hw_irq_11, IRQ8_VECTOR + 3, D_DPL0, int_tss[11], GDT_INT_START_INDEX + 11);
  set_task_int_gate (__hw_irq_12, IRQ8_VECTOR + 4, D_DPL0, int_tss[12], GDT_INT_START_INDEX + 12);
  set_task_int_gate (__hw_irq_13, IRQ8_VECTOR + 5, D_DPL0, int_tss[13], GDT_INT_START_INDEX + 13);
  set_task_int_gate (__hw_irq_14, IRQ8_VECTOR + 6, D_DPL0, int_tss[14], GDT_INT_START_INDEX + 14);
  set_task_int_gate (__hw_irq_15, IRQ8_VECTOR + 7, D_DPL0, int_tss[15], GDT_INT_START_INDEX + 15);

  /* add exception stubs */
  set_task_int_gate (__exception_0, 0, D_DPL0, exception_tss[0], GDT_INT_START_INDEX + 16);
  set_task_int_gate (__exception_1, 1, D_DPL0, exception_tss[1], GDT_INT_START_INDEX + 17);
  set_task_int_gate (__exception_2, 2, D_DPL0, exception_tss[2], GDT_INT_START_INDEX + 18);
  set_task_int_gate (__exception_3, 3, D_DPL0, exception_tss[3], GDT_INT_START_INDEX + 19);
  set_task_int_gate (__exception_4, 4, D_DPL0, exception_tss[4], GDT_INT_START_INDEX + 20);
  set_task_int_gate (__exception_5, 5, D_DPL0, exception_tss[5], GDT_INT_START_INDEX + 21);
  set_task_int_gate (__exception_6, 6, D_DPL0, exception_tss[6], GDT_INT_START_INDEX + 22);
  set_task_int_gate (__exception_7, 7, D_DPL0, exception_tss[7], GDT_INT_START_INDEX + 23);

  set_task_int_gate (__exception_8, 8,   D_DPL0, exception_tss[8],  GDT_INT_START_INDEX + 24); 
  set_task_int_gate (__exception_9, 9,   D_DPL0, exception_tss[9],  GDT_INT_START_INDEX + 25);
  set_task_int_gate (__exception_10, 10, D_DPL0, exception_tss[10], GDT_INT_START_INDEX + 26);
  set_task_int_gate (__exception_11, 11, D_DPL0, exception_tss[11], GDT_INT_START_INDEX + 27);
  set_task_int_gate (__exception_12, 12, D_DPL0, exception_tss[12], GDT_INT_START_INDEX + 28);
  set_task_int_gate (__exception_13, 13, D_DPL0, exception_tss[13], GDT_INT_START_INDEX + 29);
  set_task_int_gate (__exception_14, 14, D_DPL0, exception_tss[14], GDT_INT_START_INDEX + 30);
  set_task_int_gate (__exception_16, 16, D_DPL0, exception_tss[15], GDT_INT_START_INDEX + 31);


  /* 
   * add system call handler 
   *
   * This is not a TSS gate since it only 
   * switches the current task to kernel-mode
   *
   * -- My stacks between user/kernel mode are probably messed up
   *
   * OLD: set_int_gate (__system_call_entry, SYSCALL_VECTOR, D_DPL3, system_call_tss, GDT_INT_START_INDEX + 32);
   *
   *
   */

  //set_task_int_gate (__system_call_entry, SYSCALL_VECTOR, D_DPL3, system_call_tss, GDT_INT_START_INDEX + 32);
  set_int_gate (__system_call_entry, SYSCALL_VECTOR, D_DPL3);

  /* set the IDTR register */
  load_idtr();
}


/*
 * set_int_gate
 * sets up an int. gate descriptor and puts it in idt
 *
 */

void set_int_gate (void (*handler)(void), int int_no, int dpl)
{
  gate_t   *idt_ptr;
  long      int_handler = (long) handler;

  idt_ptr = &idt[int_no]; 

  idt_ptr->p_dpl_type = D_PRESENT | D_INT_TYPE | dpl;
  idt_ptr->offset_low = (int_handler & 0xFFFF);
  idt_ptr->offset_high = (int_handler >> 16);
  idt_ptr->selector = CS_SELECTOR;
  idt_ptr->pad = 0;
}


/*
 * set_task_int_gate
 * Sets up a Task gate and and a TSS descriptor for serving and interrupt
 * 
 */


void set_task_int_gate (void (*handler)(void), int int_no, int dpl, tss_386_t  *tss, int  gdt_index)
{
  tss_descriptor_t   *gdt;  
  gate_t             *idt_ptr;

  
  /* bail if bad gdt index */
  if ((gdt_index < GDT_INT_START_INDEX) || (gdt_index > GDT_SIZE-1))
    panic ("set_task_int_gate: GDT index out of bounds!");

  /* bail if bad int_no */
  if ((int_no < 0) || (int_no > MAX_INTS))
    panic ("set_task_int_gate: INT no out of bounds!");

  gdt = (tss_descriptor_t *) &_gdt;

  /* Setup the descriptor */
  gdt[gdt_index].g_limit_high = 0;
  gdt[gdt_index].limit_low = sizeof (*tss) - 1;
  gdt[gdt_index].p_dpl_type = D_TSS | D_PRESENT;

  gdt[gdt_index].base_low = (short int) ((long) tss & 0xffff);
  gdt[gdt_index].base_mid = (unsigned char) (((long) tss) >> 16) & 0xff;
  gdt[gdt_index].base_high = (unsigned char) (((long) tss) >> 24) & 0xff;

  /* Setup task gate */
  idt_ptr = &idt[int_no]; 

  idt_ptr->p_dpl_type = D_PRESENT | D_TASK_TYPE | dpl;
  idt_ptr->offset_low = 0;
  idt_ptr->offset_high = 0;

  /* make selector of index (always in GDT) */
  idt_ptr->selector = (gdt_index << 3);
  idt_ptr->pad = 0;

  /* setup TSS */
  tss->ds = DS_SELECTOR;
  tss->es = DS_SELECTOR;
  tss->fs = DS_SELECTOR;
  tss->gs = DS_SELECTOR;
  tss->cs = CS_SELECTOR;
  tss->ss = DS_SELECTOR;
  tss->esp = (long) int_stacktop;

  /* same address space for int-handlers */
  tss->cr3 = get_cr3 ();
  tss->eflags = 2;
  tss->eip = (long) handler;
  tss->trace = 0;
  tss->backlink = TSS_SELECTOR;
}



