/*
 * vm.h
 * Virtual memory support functions for the kernel
 * (c) Jimmy Larsson 1998
 *
 */

#ifndef VM_H
#define VM_H

#include "../kernel/types.h"
#include "../kernel/elf.h"
#include "pages.h"


/*
 * initialize virtual memory
 * (mark OS image areas as allocated)
 *
 */

loader_args_t *init_vmem (loader_args_t   *args);


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
			    loaded_module_t   *init_mod);



/*
 * do_fork_vm
 * Do memory manager part of fork
 *  Setup pagetables for CopyOnWrite and create pg-dir and pg tables for new process
 *
 */

void do_fork_vm (process_t   *parent,
		 process_t   *child);

/*
 * default page fault handling
 * - the copy on write code
 *
 */

void do_page_fault (process_t     *proc,
		    unsigned int   linear_addr,
		    long           error_code);

#endif
