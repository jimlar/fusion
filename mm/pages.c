/*
 *
 * pages.c
 * Paging support functions for the kernel
 * (c) Jimmy Larsson 1998
 *
 */


#include "pages.h"
#include "../kernel/printk.h"
#include "../kernel/types.h"
#include "../kernel/panic.h"
#include "../kernel/gdt.h"
#include "../kernel/idt.h"
#include "../kernel/process.h"

/*
 * Free pages list
 *
 */

static free_pages_t         free_page_lists[NUM_PAGE_LISTS];

static int                  num_page_frames;
static int                  num_free_page_frames;

/*
 * kernel allocated pages
 *  (unsorted list)
 */

static page_frame_t        *kernel_pages;

/*
 * array, indexed on physical pagenumber
 * to keep track of count and age
 *
 * use like  page_frames[PAGE_FRAME_NR]
 *
 */

page_frame_t        *page_frames;


/*
 * find_next_page
 * returns address of next page start
 *
 */

static unsigned int find_next_page (unsigned int   address)
{
  return address + (PAGE_SIZE - (address % PAGE_SIZE));
}


/*
 * init_paging, sets up paging
 * argument: total memory in bytes
 *
 */

loader_args_t *init_paging (loader_args_t   *args)
{
  pagetable_entry_t   *page_dir;
  pagetable_entry_t   *page_table;
  int                  i;
  int                  needed_pages;
  int                  num_dir_entries;
  page_frame_t        *tmp_frame;
  unsigned int         mem_size = args->mem_size_bytes;
  unsigned int         free_mem_start = 0;            
  loaded_module_t     *mod_ptr;
  char                *ptr_dest, *ptr_src;
  int                  tmp_len;
  int                  tmp_addr;

  /* empty kernel list */
  kernel_pages = NULL;

  /* find a safe place to move arg lists to, after all modules */
  mod_ptr = args->loaded_modules;
  while (mod_ptr != NULL)
  {
    free_mem_start = mod_ptr->load_address + PAGE_SIZE * mod_ptr->loaded_pages;
    mod_ptr = mod_ptr->next_module;
  }

  /* move args */
  ptr_dest = (char *) free_mem_start;
  ptr_src = (char *) args;
  
  for (i = 0; i < args->arguments_size; i++)
    ptr_dest[i] = ptr_src[i];

  /* set new args ptr */
  args = (loader_args_t *) free_mem_start;
  free_mem_start = find_next_page (args->arguments_size + free_mem_start);


  /* Page frames in system */
  num_page_frames = mem_size / PAGE_SIZE;
  num_free_page_frames = num_page_frames;

 
  /* How much do we need for our own lists? */
  needed_pages = (num_page_frames * sizeof (page_frame_t)) / PAGE_SIZE;
  /* Round upwards */
  if (((num_page_frames * PAGE_SIZE) % sizeof (page_frame_t)) > 0)
    needed_pages++;
  
  /* We put our lists first in available memory */
  tmp_frame = (page_frame_t *) free_mem_start;

  /* initialize page frame data elements */
  for (i = 0; i < num_page_frames; i++)
  {
    tmp_frame[i].count = 0;
    tmp_frame[i].age = 0;
    tmp_frame[i].page_frame_nr = i;
    tmp_frame[i].next = NULL;
    tmp_frame[i].prev = NULL;
  }

  page_frames = tmp_frame;

  /* 
   * Build free pages list, add PC-UMB (upper memory block: vga bios etc..), 
   *  will be marked allocated by vm.c's init later
   *
   */

  for (i = 0; i < NUM_PAGE_LISTS; i++)
  {
    free_page_lists[i].first = NULL;
    free_page_lists[i].last = NULL;
  }


  /* You cant have less than 4 page frames in a PC so this is ok */
  tmp_frame[num_page_frames - 1].next = NULL;
  tmp_frame[num_page_frames - 1].prev = &tmp_frame[num_page_frames - 2];
  
  tmp_frame[0].next = &tmp_frame[1];
  tmp_frame[0].prev = NULL;

  /* Free list head */
  free_page_lists[0].first = &tmp_frame[0];
  free_page_lists[0].last = &tmp_frame[num_page_frames - 1];

  /* Link list */
  for (i = 1; i < (num_page_frames - 1); i++)
  {
    tmp_frame[i].prev = &tmp_frame[i-1];
    tmp_frame[i].next = &tmp_frame[i+1];
  }

  free_mem_start = (unsigned int) &tmp_frame[num_page_frames];

  /*
   * Free pages list created, setup hardware page directory
   * for kernel
   *
   */

  page_dir = (pagetable_entry_t *) free_mem_start;
  page_table = (pagetable_entry_t *) (free_mem_start + PAGE_SIZE);

  /* how many page dir entries? */
  num_dir_entries = num_page_frames / PAGES_PER_PAGETABLE;
  /* round upwards */
  if (num_page_frames % PAGES_PER_PAGETABLE)
    num_dir_entries++;


  /* present, read/write, supervisor mark pages */

  /* First take care of page directory */
  for (i = 0; i < num_dir_entries; i++)
  {
    page_dir[i] = (unsigned long) page_table + i * PAGE_SIZE + PAGE_PRESENT + PAGE_WRITEABLE;
  }

  /* Map page table entries linear = physical */
  for (i = 0; i < num_page_frames; i++)
  {
    page_table[i] = PAGE_SIZE * i + PAGE_PRESENT + PAGE_WRITEABLE;
  }

  free_mem_start = (unsigned int) &page_table[num_page_frames];
  

  /* set new page directory */
  __asm__ ("mov  %%eax, %%cr3" : : "a" (page_dir) : "eax");


  /* turn on paging */
  __asm__ ("mov %%cr0, %%eax
            or  $0x80000000, %%eax
            mov %%eax, %%cr0" : : : "eax");


  /** Mark pages we used for lists as allocated */

  /* the page_frame_t list elements */
  tmp_len = needed_pages;
  tmp_addr = (int) free_page_lists[0].first;

  while (tmp_len > 0)
  {
    tmp_frame = get_page_at (tmp_addr);
    if (tmp_frame == NULL)
      panic ("MM: conflicting memory addresses while allocating memlist space!");

    add_to_kernel_pages (tmp_frame);
    tmp_len--;
    tmp_addr += PAGE_SIZE;
  }

  return args;
}


/*
 * The page allocation routine
 * returns a free page
 *
 * Will (in future) take care of swapping pages when nececary
 * 
 * 
 * 
 */

page_frame_t *get_free_page ()
{
  page_frame_t *ret;

  ret = free_page_lists[0].last;

  if (ret == NULL)
    return NULL;

  free_page_lists[0].last = ret->prev;

  if (ret->prev != NULL)
    ret->prev->next = NULL;
  else
    free_page_lists[0].first = NULL;

  ret->next = NULL;
  ret->prev = NULL;
  ret->count = 1;

  num_free_page_frames--;

  return ret;
}


/*
 *
 * Return a linear sequence of pages
 * needed for kmalloc since kernel has linear==physical mapping
 *
 * These pages cannot be swapped later since the are kernel memory
 *
 */

page_frame_t *get_free_linear_pages (int  num)
{

  if (num > 1 || num <= 0)
    return NULL;

  return get_free_page ();
}

/*
 * make a page at a specified address allocated 
 *
 * Mainly used when kernel initializes after beeing
 * loaded by OS loader
 *
 */

page_frame_t  *get_page_at (int page_addr)
{
  page_frame_t   *tmp = free_page_lists[0].first;
  int             page_no = page_addr / PAGE_SIZE;

  //Badly aligned addr?
  if (page_addr % PAGE_SIZE)
    return NULL;

  while (tmp != NULL)
  {
    if (tmp->page_frame_nr == page_no)
    {
      //It's a hit

      if (tmp->prev != NULL)
	tmp->prev->next = tmp->next;

      if (tmp->next != NULL)
	tmp->next->prev = tmp->prev;

      tmp->next = NULL;
      tmp->prev = NULL;
      tmp->count = 1;

      num_free_page_frames--;
      return tmp;
    }

    tmp = tmp->next;
  }

  return NULL;
}


/*
 * 
 * add a page to the kernel page list
 *
 */

void add_to_kernel_pages (page_frame_t *page)
{
  if (kernel_pages != NULL)
    kernel_pages->prev = page;

  page->next = kernel_pages;
  page->prev = NULL;
}


/*
 *
 * Free pages
 * input is a list of pages (or one page)
 *
 */

void free_pages (page_frame_t *pages)
{
  //This should sort lists and do stuff
  page_frame_t   *cur_frame, *next_frame, *tmp_frame;
  int             frame_num;

  cur_frame = pages;

  while (cur_frame != NULL)
  {
    cur_frame->count = 0;
    frame_num = cur_frame->page_frame_nr;
    tmp_frame = free_page_lists[0].first;
    next_frame = cur_frame->next;

    //First free page
    if (tmp_frame == NULL)
    {
      free_page_lists[0].first = cur_frame;
      free_page_lists[0].last = cur_frame;
      cur_frame->next = NULL;
      cur_frame->prev = NULL;
    } else
    {

      //Search for frame's place in free list
      while ((tmp_frame != NULL) && (frame_num <= tmp_frame->page_frame_nr))
      {
	tmp_frame = tmp_frame->next;
      }

      //Insert page
      // tmp_frame points to frame which we will insert before
 
      if (tmp_frame != NULL)
      {
	cur_frame->prev = tmp_frame->prev;

	if (tmp_frame->prev != NULL)
	  tmp_frame->prev->next = cur_frame;
	tmp_frame->prev = cur_frame;
      } else
      {
	cur_frame->prev = free_page_lists[0].last;

	if (free_page_lists[0].last != NULL)
	  free_page_lists[0].last->next = cur_frame;
      }

      cur_frame->next = tmp_frame;

    }

    num_free_page_frames++;
    cur_frame = next_frame;
  }
}

/*
 * return num of free physical pages
 *
 */

int get_avail_phys_pages()
{
  return num_free_page_frames;
}

/*
 * return num of total physical pages
 *
 */

int get_total_phys_pages()
{
  return num_page_frames;
}

/*
 * map page
 * Put a linear to physical mapping into process pagetable
 *
 * (assumes pagedir exists)
 */

void map_page (process_t     *proc, 
	       unsigned int   physical_addr,
	       unsigned int   linear_addr,
	       int            flags)
{
  unsigned int          page_table_num;
  unsigned int          pt_entry_num;

  pagetable_entry_t    *page_dir;
  pagetable_entry_t    *page_table;
  
  page_frame_t         *page_table_page;


  //Get process page directory
  page_dir = (pagetable_entry_t *) proc->tss.cr3;

  //Find page table number
  page_table_num = PAGE_DIR_INDEX(linear_addr);
  
  //Ok we know which page table number it is... find page num
  pt_entry_num = PAGE_TABLE_INDEX(linear_addr);

  //We now have the start page and page table, work the tables
  //Put page table into page dir

  if (page_dir[page_table_num] & PAGE_PRESENT)
  {
    //Already have the needed page table, get address
    page_table = (pagetable_entry_t *) PTE_TO_PHYSICAL(page_dir[page_table_num]);

  } else
  {
    //Allocate new page table
    page_table_page = get_free_page();

    /**
     * Should this be user and writeable??  yep seems like it...
     */
    page_dir[page_table_num] = page_table_page->page_frame_nr * PAGE_SIZE | 
      PAGE_PRESENT | 
      PAGE_USER | 
      PAGE_WRITEABLE;
    page_table = (pagetable_entry_t *) (page_table_page->page_frame_nr * PAGE_SIZE);
    //printf ("Allocated new page table at: 0x%x\n", page_table_page->page_frame_nr * PAGE_SIZE);
  }

  //Put physical into page table
  physical_addr = (physical_addr / PAGE_SIZE) * PAGE_SIZE;
  page_table[pt_entry_num] = physical_addr | flags; 
}


/*
 *
 * map_standard_pages
 * Put GDT, IDT and (if needed) LDT into processes 
 * address space
 */

void map_standard_pages (process_t *proc)
{
  int i;
  int pages;
  int dirs;

  dirs = (KERNEL_PAGES % PAGES_PER_PAGETABLE) + 1;
  pages = KERNEL_PAGES;

  //printf ("Mapping %u standard pages...", pages);

  if (dirs > 1)
    panic ("Kernel space bigger than 4MB not supported yet!");

  for (i = 0; i < KERNEL_PAGES; i++)
  {
    map_page (proc, i * PAGE_SIZE, i * PAGE_SIZE, PAGE_PRESENT);
  }
  
  //printf ("done\n");

  return;
}


/*
 * Create a new page dir for a process
 * and initialize it (returns 0 on no error)
 */

int create_page_dir (process_t   *proc)
{
  unsigned int   *ptr;
  page_frame_t   *page_dir;
  int             i;

  //Get a page dir
  page_dir = get_free_page();
  if (page_dir == NULL)
    return -1;

  ptr = (unsigned int *) (page_dir->page_frame_nr * PAGE_SIZE);
  //printf ("create_page_dir: Allocated new page directory at: 0x%x\n", page_dir->page_frame_nr * PAGE_SIZE);

  //
  // PAGE FAULT HERE!
  //

  //Make all page tables *not* present
  for (i = 0; i < PAGE_SIZE/4; i++)
    ptr[i] = 0;

  //printf ("cleared!\n");

  //Set CR3 to new page dir
  proc->tss.cr3 = page_dir->page_frame_nr * PAGE_SIZE;

  
  return 0;
}


/*
 * prepare_copy_on_write
 * Prepare pagetables for copy on write, usually for a fork call
 * - marks all pages as readonly
 */

void prepare_copy_on_write (process_t   *parent,
			    process_t   *child)
{
  pagetable_entry_t    *page_dir, *page_dir_parent;
  pagetable_entry_t    *page_table, *page_table_parent;
  int                   i,j;
  page_frame_t         *new_table;
  page_frame_t         *page_frame;


  //Get process page directory
  page_dir = (pagetable_entry_t *) child->tss.cr3;
  page_dir_parent = (pagetable_entry_t *) parent->tss.cr3;


  //Create new pagetable/page dir set for child
  for (i = 0; i < PAGES_PER_PAGETABLE; i++)
  {
    if (page_dir_parent[i] & PAGE_PRESENT)
    {
      new_table = get_free_page ();
      if (new_table == NULL)
	panic ("Cant alloc new page table");

      page_dir[i] = (page_dir_parent[i] & ~PHYS_PAGE_MASK) | (new_table->page_frame_nr * PAGE_SIZE);
      page_table = (pagetable_entry_t *) PTE_TO_PHYSICAL(page_dir[i]);
      page_table_parent = (pagetable_entry_t *) PTE_TO_PHYSICAL(page_dir_parent[i]);
   
      for (j = 0; j < PAGES_PER_PAGETABLE; j++)
      {
	//Mark all user pages read-only
	if (page_table_parent[j] & PAGE_PRESENT)
	{
	  if (page_table_parent[j] & PAGE_USER)
	    page_table[j] = page_table_parent[j] & ~PAGE_WRITEABLE;
	  else
	    page_table[j] = page_table_parent[j];
	}

	//Increase count of page frame
	page_frame = &page_frames[PHYS_TO_FRAME_NR (PTE_TO_PHYSICAL (page_table[j]))];
	page_frame->count++;
      }
    }
  }

  //Mark parents user pages readonly
  for (i = 0; i < PAGES_PER_PAGETABLE; i++)
  {
    //Mark parents user pages readonly
    if (page_dir_parent[i] & PAGE_PRESENT)
    {
      //We have a table in this index
      
      //Mark all user table entries readonly
      page_table_parent = (pagetable_entry_t *) PTE_TO_PHYSICAL(page_dir_parent[i]);
      
      for (j = 0; j < PAGES_PER_PAGETABLE; j++)
      {
	if (page_table_parent[j] & (PAGE_PRESENT | PAGE_USER))
	{
	  page_table_parent[j] = page_table_parent[j] & ~PAGE_WRITEABLE;
	}
      }
    }
  }
} 
