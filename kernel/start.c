/*
 * C entry point for Fusion
 * (System entry in entry.S)
 * (c) Jimmy Larsson 1998-1999
 * 
 *
 */


#include "types.h"
#include "config.h"
#include "process.h"
#include "printk.h"
#include "interrupt.h"
#include "panic.h"
#include "elf.h"
#include "../mm/vm.h"
#include "../mm/kmalloc.h"
#include "port_io.h"
#include "util.h"
#include "assert.h"
#include "vga.h"

/*
 * decode_elf
 *
 * Decode elfdata for the init module
 */

void decode_elf  (loaded_module_t    *init_mod,
		  Elf32_Phdr        **program_header_table,
		  int                *num_pht_entries,
		  unsigned int       *entry)
{
  Elf32_Ehdr      *elf_header;


  if (init_mod->page_frames == NULL)
    panic ("INIT module is empty!");

  elf_header = (Elf32_Ehdr *) (init_mod->page_frames->page_frame_nr * PAGE_SIZE);


  if (elf_header->e_ident[EI_MAG0] != ELFMAG0 ||
      elf_header->e_ident[EI_MAG1] != ELFMAG1 ||  
      elf_header->e_ident[EI_MAG2] != ELFMAG2 ||  
      elf_header->e_ident[EI_MAG3] != ELFMAG3)
  {
    panic ("BAD ELF Magic numbers!");
  }

  if (elf_header->e_type != ET_EXEC)
    panic ("BAD ELF File! (non executable)");

  if (elf_header->e_machine != EM_386)
    panic ("BAD ELF File! (non i386)");

  if (elf_header->e_version != EV_CURRENT)
    panic ("BAD ELF File (invalid version)!");

  *entry = elf_header->e_entry;

  *num_pht_entries = elf_header->e_phnum;
  *program_header_table = (Elf32_Phdr *) (elf_header->e_phoff + (unsigned int) elf_header);
}

/*
 * Startup the init module
 *
 *  * Get the pages where the loader has loaded the INIT.SYS
 *  * Alloc pages and setup pagetables
 *  * Set eip, cs, ds, fs and gs.
 *  * Alloc a stack page and set esp + ss;
 *  * 
 *
 */

void start_initmodule (loaded_module_t   *init_mod)
{
  process_t      *new_process;
  //int             i;

  /** ELF Programr header table */
  int             pht_entries;
  Elf32_Phdr     *pht;
  unsigned int    entry_addr;

  /** Alloc a new process entry */
  new_process = get_proc_entry();
  decode_elf (init_mod, &pht, &pht_entries, &entry_addr);

  //for (i = 0; i < pht_entries; i++)
  //{
  //  printf ("INIT PHT %u: v_addr 0x%x memsz 0x%x\n",
  //	    i,
  //	    pht[i].p_vaddr,
  //	    pht[i].p_memsz); 
  //}
  //printf ("INIT entry address: 0x%x init proc: 0x%x\n", entry_addr, (int) new_process);

  //Set eip to entry address
  new_process->tss.eip = (unsigned long) entry_addr;
  
  //Setup segemnt registers
  new_process->tss.fs = USER_DS_SELECTOR;
  new_process->tss.gs = USER_DS_SELECTOR;
  new_process->tss.ds = USER_DS_SELECTOR;
  new_process->tss.es = USER_DS_SELECTOR;
  new_process->tss.cs = USER_CS_SELECTOR;

  //Setup flags (IF=1, enable interrupts) and misc
  new_process->tss.eflags = 2 + 512;
  new_process->tss.trace = 0;
  new_process->tss.backlink = TSS_SELECTOR;  
  new_process->tss.ldt = 0;

  //Setup page table and stack
  vm_setup_init_process (new_process, pht, pht_entries, init_mod);

  
  //Set UID/GID
  new_process->gid = 0;
  new_process->uid = 0;

  //Set umask
  new_process->umask = 022;

  make_process_ready (new_process);

  vga_set_wrapline();

  //printf ("Starting init process...\n");
  
  //START!!!
  sti();
}


/* 
 * System startup
 *
 */

void system_start (loader_args_t   *loader_arguments)
{
  loaded_module_t   *mod_ptr;
  char              *test1,*test2,*test3,*test4,*test5;

  vga_init (loader_arguments);

  printf ("---------------------------------------\n");
  printf (VERSION_STRING);
  printf (COPYRIGHT_STRING);
  printf ("---------------------------------------\n");

  /* Memory system might want to move arguments */
  loader_arguments = init_vmem (loader_arguments);

  /* setup interrupt system */
  init_interrupts();

  /* intialize processes and scheduler */
  init_processses ();

  /* Init keyboard */
  keyboard_init ();

  /* test kmalloc */
  test1 = (char *) kmalloc (4);
  if (test1 == NULL)
    panic ("KMALLOC: nothing alloc'd");
  test2 = (char *) kmalloc (120);
  if (test2 == NULL)
    panic ("KMALLOC: nothing alloc'd");
  test3 = (char *) kmalloc (120);
  if (test3 == NULL)
    panic ("KMALLOC: nothing alloc'd");
  test4 = (char *) kmalloc (12);
  if (test4 == NULL)
    panic ("KMALLOC: nothing alloc'd");
  test5 = (char *) kmalloc (120);
  if (test5 == NULL)
    panic ("KMALLOC: nothing alloc'd");
  
  kfree (test1);
  kfree (test3);
  kfree (test5);
  kfree (test2);
  kfree (test4);

  /* find and startup the (hopefully) loaded init module */
  mod_ptr = loader_arguments->loaded_modules;
  while (mod_ptr != NULL)
  {
    //Is it the init module?
    if (string_cmp (mod_ptr->filename, "INIT.SYS") == 0)
      break; //Yes

    mod_ptr = mod_ptr->next_module;
  }

  if (mod_ptr != NULL)
    start_initmodule (mod_ptr);
  else
    panic ("INIT Module not loaded!");
}


