/*
 * The processes of Fusion
 * 
 * (c) Jimmy Larsson 1998
 * 
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "../mm/vm.h"


#define MAX_PROCESSES    1024

#define STATE_RUNNING    1
#define STATE_READY      2
#define STATE_BLOCKED    3
#define STATE_FREE       4  /* this process_t is free */


/** The currently running process */
extern process_t   *current_proc;

/** The process entries */
extern process_t   processes[];

/*
 * Initialize process subsystem
 *
 *
 *
 */

void init_processses ();


/*
 * Get free process entry
 *
 * (initializes pid) 
 *
 * returns NULL if none availiable
 */

process_t *get_proc_entry ();


/*
 * Release a process entry
 *
 */

void put_proc_entry (process_t *proc);


/*
 * Add process onto run queue end
 *
 */

void make_process_ready (process_t  *proc);


/*
 * Schedule, pretty simple right now...
 * Plain old round robin, without priorities
 *
 */

void schedule ();


#endif
