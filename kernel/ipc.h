/*
 * ipc.h
 *
 * Kernel IPC mechanisms
 * These are only used inside the kernel itself, 
 * not in user space
 *
 * (c) Jimmy Larsson 1999
 *
 */

#ifndef IPC_H
#define IPC_H

/** The spin lock type */
typedef unsigned char   spin_lock_t;

	       
/*
 * test_and_set 
 *
 * Tests if byte *b is zero and sets it to 1 if so.
 * Returns:  0 - if byte was set
 *           1 - if byte was not set 
 * 
 */

int test_and_set (unsigned char *b);


/*
 * get_spin_lock
 * Wait for spin lock and grab it
 *
 */

void get_spin_lock (spin_lock_t *lock);

/*
 * release_spin_lock
 * Give back the lock
 *
 */

void release_spin_lock (spin_lock_t  *lock);


#endif
