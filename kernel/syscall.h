/*
 * System call handler for Fusion
 * (c) 1998 Jimmy Larsson
 *
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include "process.h"

/* For storing syscall arguments */
typedef struct 
{
  unsigned   call_num;

  unsigned   arg1;
  unsigned   arg2;
  unsigned   arg3;
} syscall_args_t;


/*
 * system call handler
 *
 */


int master_system_call_handler (unsigned int   call_num, 
				unsigned int   arg1, 
				unsigned int   arg2, 
				unsigned int   arg3);

/*
 * Put return value into caller
 *
 */

void put_return (process_t *proc, unsigned val);


#endif
