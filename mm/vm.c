/*
 * vm.c
 * Virtual memory support functions for the kernel
 * (c) Jimmy Larsson 1998
 *
 */


#include "vm.h"
#include "pages.h"
#include "../kernel/types.h"
#include "../kernel/printk.h"
#include "../kernel/panic.h"
#include "../kernel/elf.h"
#include "../kernel/gdt.h"
#include "../kernel/idt.h"
#include "../kernel/interrupt.h"
#include "../kernel/process.h"

/* max/min macros */
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)


/*
 * initialize virtual memory
 * (mark OS image areas as allocated)
 *
 */

loader_args_t *init_vmem (loader_args_t   *args)
{
  loaded_module_t   *mod_ptr = args->loaded_modules;
  int                tmp_addr, tmp_len;
  page_frame_t      *tmp_frame;
  int                kernel_module = 1;

  /* Setup paging, might move args, save ptr for return */
  args = init_paging (args);


  /* Make the arguments into allocated kernel memory */
  tmp_len = args->arguments_size / PAGE_SIZE;
  //Round upwards
  if (args->arguments_size % PAGE_SIZE)
    tmp_len++;

  tmp_addr = (int) args;

  while (tmp_len > 0)
  {
    tmp_frame = get_page_at (tmp_addr);
    if (tmp_frame == NULL)
      panic ("VM: conflicting memory addresses while allocating args space!");

    add_to_kernel_pages (tmp_frame);
    tmp_len--;
    tmp_addr += PAGE_SIZE;
  }


  /* list loaded modules */
  while (mod_ptr != NULL)
  {
    int pages = mod_ptr->loaded_pages;
    int address = mod_ptr->load_address;
    mod_ptr->page_frames = NULL;
    tmp_frame = NULL;

    //Mark pages allocated
    while (pages > 0)
    {
      page_frame_t *page_frame = get_page_at (address);

      if (page_frame == NULL)
	panic ("VM: loaded modules overlap, or has invalid addresses!");

      //Add the page to the kernel page list
      //First module is always kernel
      if (kernel_module)
      {
	add_to_kernel_pages (page_frame);
	kernel_module = 0;
      } else
      {

	page_frame->next = NULL;
	page_frame->prev = tmp_frame;

	if (tmp_frame != NULL)
	  tmp_frame->next = page_frame;
	else
	  mod_ptr->page_frames = page_frame;

	tmp_frame = page_frame;
      }

      pages--;
      address += PAGE_SIZE;
    }

    
    printf ("VM: %s was loaded into 0x%x (%u page(s) used)\n", 
    	    mod_ptr->filename, 
    	    mod_ptr->load_address, 
    	    mod_ptr->loaded_pages);
    

    mod_ptr = mod_ptr->next_module;
  }


  printf ("VM: %ukB physical memory, %ukB free.\n",
	  (get_total_phys_pages() * PAGE_SIZE) / 1024,
	  (get_avail_phys_pages() * PAGE_SIZE) / 1024);



  return args;
}

/*
 * Setup the init task
 *
 * * Create a page table
 * * Alloc stack and put int page table
 * * setup esp and ss
 */

void vm_setup_init_process (process_t         *init_proc, 
			    Elf32_Phdr        *pht, 
			    int                pht_entries,
			    loaded_module_t   *init_mod)
{
  page_frame_t         *stack_page;
  page_frame_t         *kernel_stack_page;
  int                   i;
  unsigned int          data_min = 0xffffffff;
  unsigned int          data_max = 0;
  unsigned int          code_min = 0xffffffff;
  unsigned int          code_max = 0;
  

  //
  // Create a page directory
  //
  if (create_page_dir (init_proc))
  {
    panic ("Cant setup page dir for init task!\n");
  }

  //
  // HAS TO CHECK PHT AND MAP ACCORDINGLY!!
  //

  //Get page table and insert loaded pages into it
  for (i = 0; i < pht_entries; i++)
  {
    if (pht[i].p_flags & PF_X)
    {
      //It's a code entry
      if (pht[i].p_memsz > PAGE_SIZE)
	panic ("INIT bigger tham one page, not implemented yet!");
      
      if (pht[i].p_memsz > 0)
      {
	code_max = MAX(code_max,(pht[i].p_vaddr + pht[i].p_memsz));
	code_min = MIN(code_min,pht[i].p_vaddr);

	map_page (init_proc, 
		  init_mod->page_frames->page_frame_nr * PAGE_SIZE, 
		  pht[i].p_vaddr, 
		  PAGE_PRESENT | PAGE_USER);
      }

    } else
    {
      //It's a data entry
      if (pht[i].p_memsz > PAGE_SIZE)
	panic ("INIT bigger tham one page, not implemented yet!");
      
      if (pht[i].p_memsz > 0)
      {
	data_max = MAX(data_max,(pht[i].p_vaddr + pht[i].p_memsz));
	data_min = MIN(data_min,pht[i].p_vaddr);

	map_page (init_proc, 
		  init_mod->page_frames->page_frame_nr * PAGE_SIZE, 
		  pht[i].p_vaddr, 
		  PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER);
      }
    }
  }

  //Set the areas in proc table
  init_proc->vm_code.vm_start = code_min;
  init_proc->vm_code.vm_end = code_max;
  init_proc->vm_code.vm_flags = VM_READONLY | VM_CODE;

  init_proc->vm_data.vm_start = data_min;
  init_proc->vm_data.vm_end = data_max;
  init_proc->vm_data.vm_flags = VM_READWRITE;


  //Setup a new stack page
  stack_page = get_free_page ();
  kernel_stack_page = get_free_page ();

  init_proc->tss.ss0 = DS_SELECTOR;
  init_proc->tss.esp0 = PROC_KERNEL_STACK_TOP;

  init_proc->tss.ss = USER_DS_SELECTOR;
  init_proc->tss.esp = USER_STACK_TOP;

  map_page (init_proc, 
	    kernel_stack_page->page_frame_nr * PAGE_SIZE, 
	    PROC_KERNEL_STACK_TOP, 
	    PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER);

  map_page (init_proc, 
	    stack_page->page_frame_nr * PAGE_SIZE, 
	    USER_STACK_TOP, 
	    PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER);

  //vm addresses are linear
  init_proc->vm_kernel_stack.vm_start = PROC_KERNEL_STACK_TOP & PHYS_PAGE_MASK;
  init_proc->vm_kernel_stack.vm_end = (PROC_KERNEL_STACK_TOP & PHYS_PAGE_MASK) + PAGE_SIZE;
  init_proc->vm_kernel_stack.vm_flags = VM_READWRITE;
 
  init_proc->vm_stack.vm_start = USER_STACK_TOP & PHYS_PAGE_MASK;
  init_proc->vm_stack.vm_end = (USER_STACK_TOP & PHYS_PAGE_MASK) + PAGE_SIZE;
  init_proc->vm_stack.vm_flags = VM_READWRITE;
    

  map_standard_pages (init_proc);
}


/*
 * do_fork_vm
 * Do memory manager part of fork
 *  Setup pagetables for CopyOnWrite and create pg-dir and pg tables for new process
 *
 */

void do_fork_vm (process_t   *parent,
		 process_t   *child)
{
  //printf ("FORK_VM: forking memory tables\n");

  // Create a page directory
  if (create_page_dir (child))
  {
    panic ("Cant setup page dir for new child!\n");
  }

  //printf ("FORK_VM: preparing copy on write\n");

  //Setup pagetables
  prepare_copy_on_write (parent, child);

  //printf ("FORK_VM: setting mem-blocks\n");

  //Set the vm_blocks in proc table
  child->vm_code.vm_start = parent->vm_code.vm_start;
  child->vm_code.vm_end = parent->vm_code.vm_end;
  child->vm_code.vm_flags = parent->vm_code.vm_flags;

  child->vm_data.vm_start = parent->vm_data.vm_start;
  child->vm_data.vm_end = parent->vm_data.vm_end;
  child->vm_data.vm_flags = parent->vm_data.vm_flags;

  child->vm_stack.vm_start = parent->vm_stack.vm_start;
  child->vm_stack.vm_end = parent->vm_stack.vm_end;
  child->vm_stack.vm_flags = parent->vm_stack.vm_flags;

  child->vm_kernel_stack.vm_start = parent->vm_kernel_stack.vm_start;
  child->vm_kernel_stack.vm_end = parent->vm_kernel_stack.vm_end;
  child->vm_kernel_stack.vm_flags = parent->vm_kernel_stack.vm_flags;

  //printf ("FORK_VM: done\n");
}

/*
 * default page fault handling
 * - the copy on write code
 *
 */

void do_page_fault (process_t     *proc,
		    unsigned int   linear_addr,
		    long           error_code)
{
  page_frame_t       *new_page, *page_frame;
  unsigned int        phys_frame_addr;
  pagetable_entry_t  *pte;
  int                 pf_ok;


  //Only handle usermode writes to present pages (not present ones later (pagein))
  //if ((error_code & PAGE_FAULT_P) &&
  //    (error_code & PAGE_FAULT_RW) &&
  //    (error_code & PAGE_FAULT_US))
  
  if (error_code & PAGE_FAULT_P) {
    
    //
    // CHECK IF WE REALLY SHOULD COPY THIS
    //  (with vm_blocks)
    //

    pf_ok = 0;

    //Only write pagefaults handled
    if (error_code & PAGE_FAULT_RW) {
      
      if ((linear_addr >= proc->vm_data.vm_start) &&
	  (linear_addr < proc->vm_data.vm_end) &&
	  (proc->vm_data.vm_flags & VM_READWRITE))
	pf_ok = 1;
      
      if ((linear_addr >= proc->vm_stack.vm_start) &&
	  (linear_addr < proc->vm_stack.vm_end) &&
	  (proc->vm_stack.vm_flags & VM_READWRITE))
	pf_ok = 1;
      
      if ((linear_addr >= proc->vm_kernel_stack.vm_start) &&
	  (linear_addr < proc->vm_kernel_stack.vm_end) &&
	  (proc->vm_kernel_stack.vm_flags & VM_READWRITE))
	pf_ok = 1;
    }


    if (!pf_ok)
    {
      printf ("*real* page fault (out of bounds), should terminate task!\n");
      printf ("present=%s, write=%s, usermode=%s, address=0x%x, dir=0x%x\n",
	      ((int) error_code & PAGE_FAULT_P) ? "true" : "false",
	      ((int) error_code & PAGE_FAULT_RW) ? "true" : "false",
	      ((int) error_code & PAGE_FAULT_US) ? "true" : "false",
	      linear_addr,
	      (int) (proc->tss.cr3));
      
      printf ("data start 0x%x, data end 0x%x, flags 0x%x\n",
	      (int) proc->vm_data.vm_start,
	      (int) proc->vm_data.vm_end,
	      (int) proc->vm_data.vm_flags);

      printf ("stack start 0x%x, stack end 0x%x, flags 0x%x\n",
	      (int) proc->vm_stack.vm_start,
	      (int) proc->vm_stack.vm_end,
	      (int) proc->vm_stack.vm_flags);

      while(1);
    }


    //Get dir
    pte = (pagetable_entry_t *) proc->tss.cr3;

    //Get table from dir
    pte = (pagetable_entry_t *) PTE_TO_PHYSICAL(pte[PAGE_DIR_INDEX(linear_addr)]);
    
    //Get page from table
    phys_frame_addr = PTE_TO_PHYSICAL(pte[PAGE_TABLE_INDEX(linear_addr)]);
 
    //Check use count of this page frame
    page_frame = &page_frames[PHYS_TO_FRAME_NR (phys_frame_addr)];
    
    if (page_frame->count > 1)
    {
      //Page in use by others, we need to copy
      new_page = get_free_page();

      if (new_page == NULL)
	panic ("Can't COW, no free pages!");

      //Copy page
      //printf ("COW(copy, 0x%x->0x%x)\n", phys_frame_addr, new_page->page_frame_nr * PAGE_SIZE);
 
      copy_page (phys_frame_addr, new_page->page_frame_nr * PAGE_SIZE);

      //Remap pagetable
      map_page (proc, new_page->page_frame_nr * PAGE_SIZE, 
		linear_addr & PHYS_PAGE_MASK, PAGE_PRESENT | PAGE_USER | PAGE_WRITEABLE);
    
      //Decrease use count
      page_frame->count--;

    } else if (page_frame->count == 1)
    {
      //The page is not in use by others, just remap
      //printf ("COW(remap, 0x%x)\n", linear_addr);
 
      //Remap pagetable
      map_page (proc, phys_frame_addr, 
		linear_addr & PHYS_PAGE_MASK, PAGE_PRESENT | PAGE_USER | PAGE_WRITEABLE);
  
    } else
    {
      printf ("Page frame has invalid use count!\n");
      while (1);
    }

    //Schedule next process
    schedule();

  } else {

    printf ("*real* page fault (page not present), should terminate task!\n");
 
    printf ("present=%s, write=%s, usermode=%s, address=0x%x, dir=0x%x\n",
	    ((int) error_code & PAGE_FAULT_P) ? "true" : "false",
	    ((int) error_code & PAGE_FAULT_RW) ? "true" : "false",
	    ((int) error_code & PAGE_FAULT_US) ? "true" : "false",
	    linear_addr,
	    (int) (proc->tss.cr3));
 
    printf ("data start 0x%x, data end 0x%x, flags 0x%x\n",
	      (int) proc->vm_data.vm_start,
	      (int) proc->vm_data.vm_end,
	      (int) proc->vm_data.vm_flags);

    printf ("stack start 0x%x, stack end 0x%x, flags 0x%x\n",
	    (int) proc->vm_stack.vm_start,
	    (int) proc->vm_stack.vm_end,
	    (int) proc->vm_stack.vm_flags);

    while(1);
  }
}
		    
