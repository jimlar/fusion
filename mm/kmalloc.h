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

#ifndef KMALLOC_H
#define KMALLOC_H

/*
 * memory block header type
 */
typedef union kmalloc_header
{
  struct 
  {
    /** next block in list */
    union kmalloc_header   *next_free;

    /** size of this block */
    unsigned int            size;
  } s;

  /*
   * This union member is never used it just guarantees
   * long integer alignment (32-bit align on i386)
   */
  long   align_dummy;

} kmalloc_header_t;


  
/*
 * kmalloc
 * Allocate kernel memory for general use
 *
 * size - wanted size in bytes
 *   size is always rounded up to a multiple of sizeof(kmalloc_header_t)
 */

void *kmalloc (unsigned int   size);


/*
 * kfree
 * free a block allocated with kmalloc
 *
 */

void kfree (void   *addr);

#endif
