/*
 * kmalloc routines for FUSION
 *
 * kernel memory allocation, right now pretty simple
 * and inefficient I think. Have not spent so much 
 * time with this code. Could use improvements.
 *
 * (c) 1999 Jimmy Larsson
 *
 */

#include "pages.h"
#include "../kernel/printk.h"

#include "kmalloc.h"

/** Minimum of units to get from 'system' */
#define MIN_ALLOC_UNITS    UNITS_PER_PAGE
#define UNITS_PER_PAGE     (PAGE_SIZE / sizeof(kmalloc_header_t))


static kmalloc_header_t   *free_list = NULL;
static kmalloc_header_t    initial_list;


/*
 * get_more_pages
 * helper to allocate more pages when needed
 *
 */

static kmalloc_header_t *get_kmalloc_pages (unsigned int   n_units)
{
  page_frame_t       *new_pages;
  kmalloc_header_t   *new_block;
  int                 pages;


  if (n_units < MIN_ALLOC_UNITS)
    n_units = MIN_ALLOC_UNITS;

  /** How many pages? */
  pages = n_units / UNITS_PER_PAGE;
  if (n_units % UNITS_PER_PAGE)
    pages++;

  new_pages = get_free_linear_pages (pages);
  
  /** could not get more pages? */
  if (new_pages == NULL)
    return NULL;
  
  /** Adjust n_units to match pages */
  n_units = pages * UNITS_PER_PAGE;

  //printf ("kmalloc: got %u units from system\n", n_units);

  new_block = (kmalloc_header_t *) (new_pages->page_frame_nr * PAGE_SIZE);
  new_block->s.size = n_units;
  
  /** Put block in free_list */
  kfree ((void *) (new_block + 1));

  return free_list;
}


/*
 * kmalloc
 * Allocate kernel memory for general use
 *
 * size - wanted size in bytes
 *   size is always rounded up to a multiple of sizeof(kmalloc_header_t)
 */

void *kmalloc (unsigned int   size)
{
  kmalloc_header_t   *cur_block, *prev_block;
  int                 n_units;

  /** Calculate how many multiples of sizeof(kmalloc_header_t) we need */
  n_units = (size + sizeof (kmalloc_header_t) - 1) / sizeof (kmalloc_header_t) + 1;
  //printf ("kmallocing %u units\n", n_units); 

  /** On first call, build initial free list */
  if ((prev_block = free_list) == NULL)
  {
    initial_list.s.next_free = free_list = prev_block = &initial_list;
    initial_list.s.size = 0;
  }

  /** Walk through free list and look for block large enough (first fit) */
  for (cur_block = prev_block->s.next_free ;; 
       prev_block = cur_block, cur_block = cur_block->s.next_free)
  {
    if (cur_block->s.size >= n_units)
    {
      /** This block is big enough, is it exact? */
      if (cur_block->s.size == n_units)
      {
	/** detach block from free list */
	prev_block->s.next_free = cur_block->s.next_free; 

      } else
      {
	/** Remove as many units from this block as we need */
	cur_block->s.size -= n_units;

	cur_block = cur_block + cur_block->s.size;
	cur_block->s.size = n_units;
      }
      
      free_list = prev_block;
      return (void *)(cur_block + 1);
    }
    
    /** No block large enough has been found, get more */
    if (cur_block == free_list)
    {
      if ((cur_block == get_kmalloc_pages (n_units)) == NULL)
	return NULL;
    }
  } 
}

/*
 * kfree
 * free a block allocated with kmalloc
 *
 */

void kfree (void   *addr)
{
  kmalloc_header_t   *block, *tmp_block;

  /** Get pointer to the block header */
  block = (kmalloc_header_t *) addr - 1;

  //printf ("kfree: freeing %u units\n", block->s.size);

  /** Find where to insert this block */
  for (tmp_block = free_list; 
       !(block > tmp_block && block < tmp_block->s.next_free);
       tmp_block = tmp_block->s.next_free)
  {
    if (tmp_block >= tmp_block->s.next_free && 
	(block > tmp_block || block < tmp_block->s.next_free))
      break;
  }

  /** Check to see if we can join with upper neighbour */
  if (block + block->s.size == tmp_block->s.next_free)
  {
    block->s.size += tmp_block->s.next_free->s.size;
    block->s.next_free = tmp_block->s.next_free->s.next_free;
    //printf ("kfree: block joined (upper)\n");

  } else
    block->s.next_free = tmp_block->s.next_free;
  

  /** Check to see if we can join with lower neighbour */
  if (tmp_block + tmp_block->s.size == block)
  {
    tmp_block->s.size += block->s.size;
    tmp_block->s.next_free = block->s.next_free;
    //printf ("kfree: block joined (lower)\n");
 
  } else
    tmp_block->s.next_free = block;
  
  free_list = tmp_block;
}
