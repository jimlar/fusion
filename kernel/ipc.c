/*
 * Kernel IPC mechanisms
 * These are only used inside the kernel itself, 
 * not in user space
 *
 * (c) Jimmy Larsson 1999
 *
 */

#include "ipc.h"


/*
 * test_and_set 
 *
 * Tests if byte *b is zero and sets it to 1 if so.
 * Returns:  0 - if byte was set
 *           1 - if byte was not set 
 * 
 */

int test_and_set (unsigned char *b)
{
  unsigned char result = 1;
  __asm__ ("xchgb %1, %0"
	   :"=m" (*b),"=r" (result)
	   :"1" (result) :"memory");
  return result;
}

	       
/*
 * get_spin_lock
 * Wait for spin lock and grab it
 *
 */

void get_spin_lock (spin_lock_t *lock)
{
  while (test_and_set((unsigned char *) lock));
}

/*
 * release_spin_lock
 * Give back the lock
 *
 */

void release_spin_lock (spin_lock_t  *lock)
{
  *lock = 0;
}
