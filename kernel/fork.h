/*
 *
 * Fork - for fusion
 * (c) 1999 Jimmy Larsson
 *
 */

#ifndef FORK_H
#define FORK_H

#include "process.h"


/*
 * Fork 'source' and put the new process in
 * run queue
 *
 * returns child pid 
 */

int do_fork (process_t *source);


#endif
