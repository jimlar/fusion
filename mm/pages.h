/*
 * pages.h
 * Paging support functions for the kernel
 * (c) Jimmy Larsson 1998
 *
 */

#ifndef PAGES_H
#define PAGES_H

#include "../kernel/types.h"

/* This really should not be here */
#define NULL 0L

/* How many pages in virtual mem belongs to kernel (first 4MB) */
#define KERNEL_PAGES               1024


/* Pagedata and bits, machine dependant */
#define PAGE_SIZE                  4096
#define PAGES_PER_PAGETABLE        1024

/* How to get indices into page dir and table */
#define PAGE_DIR_MASK              0xffc00000
#define PAGE_DIR_SHIFT             22
#define PAGE_TABLE_MASK            0x003ff000
#define PAGE_TABLE_SHIFT           12
#define PAGE_OFFSET_MASK           0x00000fff
#define PAGE_OFFSET_SHIFT          0
#define PHYS_PAGE_MASK             0xfffff000

/* Use these macros to get indices and offset in page tables/pages */
#define PAGE_DIR_INDEX(a) ((a & PAGE_DIR_MASK) >> PAGE_DIR_SHIFT) 
#define PAGE_TABLE_INDEX(a) ((a & PAGE_TABLE_MASK) >> PAGE_TABLE_SHIFT) 
#define PAGE_OFFSET(a) ((a & PAGE_OFFSET_MASK) >> PAGE_OFFSET_SHIFT) 

/* Use this to get physical address from pte */
#define PTE_TO_PHYSICAL(pte) (pte & PHYS_PAGE_MASK)

/* Physical address to page frame number */
#define PHYS_TO_FRAME_NR(phys) (phys >> PAGE_TABLE_SHIFT)

#define PAGE_PRESENT               1
#define PAGE_WRITEABLE             2
#define PAGE_USER                  4
#define PAGE_WRITETHROUGH          8
#define PAGE_CACHEDISABLE          16
#define PAGE_ACCESS                32
#define PAGE_DIRTY                 64


/* Pages leaved with bios infomation, will use this later */
#define BIOS_RESERVED_PAGES        1

/* PC memory hole in first megabyte */
#define UMB_START_PAGE             160
#define UMB_LAST_PAGE              255


/* Where we build the kernels pagedir and pagetables (0-0xffff used by bios, leave it for now) */
#define PAGE_DIR_INIT_ADDRESS      0x10000
#define PAGE_TABLE_INIT_ADDRESS    PAGE_DIR_INIT_ADDRESS + 1024

/* How many free pages-lists? (Only one for now, maybe five later) */
#define NUM_PAGE_LISTS             1

/* Copy page functions */
#define copy_page(src_addr,dest_addr) copy_mem(src_addr,dest_addr,PAGE_SIZE)

#define copy_mem(src_addr,dest_addr,n) __asm__ ("cld; rep; movsb" \
                                 ::"D" ((long)(dest_addr)),"S" ((long)(src_addr)),"c" ((long) (n)) \
                                 :"di","si","cx");


typedef unsigned long    pagetable_entry_t;

/*
 * array, indexed on physical pagenumber
 * to keep track of count and age
 *
 * use like  page_frames[PAGE_FRAME_NR]
 *
 */

extern page_frame_t        *page_frames;


/*
 * Used for the list with free page blocks
 * 
 */

typedef struct free_pages_struct
{
  page_frame_t   *first;
  page_frame_t   *last;
} free_pages_t;



/*
 * init_paging, sets up paging
 * argument: total memory in bytes
 *
 */

loader_args_t *init_paging (loader_args_t  *args);


/*
 * The page allocation routine
 * returns a free page
 *
 * Will (in future) take care of swapping pages when nececary
 * 
 * 
 * 
 */

page_frame_t *get_free_page ();

/*
 *
 * Return a linear sequence of pages
 * needed for kmalloc since kernel has linear==physical mapping
 *
 * These pages cannot be swapped later since the are kernel memory
 *
 */

page_frame_t *get_free_linear_pages (int  num);

/*
 * make a page at a specified address allocated 
 *
 * Maily used when kernel initializes after beeing
 * loaded by OS loader
 *
 */

page_frame_t  *get_page_at (int page_addr);

/*
 * 
 * add a page to the kernel page list
 *
 */

void add_to_kernel_pages (page_frame_t *page);


/*
 *
 * Free pages
 * input is a list of pages (or one page)
 *
 */

void free_pages (page_frame_t *pages);

/*
 * return num of free physical pages
 *
 */

int get_avail_phys_pages();

/*
 * return num of total physical pages
 *
 */

int get_total_phys_pages();

/*
 * map page
 * Put a linear to physical mapping into process pagetable
 *
 * (assumes pagedir exists)
 */

void map_page (process_t     *proc, 
	       unsigned int   physical_addr,
	       unsigned int   linear_addr,
	       int            flags);


/*
 *
 * map_standard_pages
 * Put GDT, IDT and (if needed) LDT into processes 
 * address space
 */

void map_standard_pages (process_t *proc);


/*
 * Create a new page dir for a process
 * and initialize it (returns 0 on no error)
 */

int create_page_dir (process_t   *proc);


/*
 * prepare_copy_on_write
 * Prepare pagetables for copy on write, usually for a fork call
 * - marks all pages as readonly
 */

void prepare_copy_on_write (process_t   *parent,
			    process_t   *child);


#endif
