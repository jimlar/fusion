/*
 * switching.h
 *
 * $Id$
 *
 * Support for switching to a tss
 * (c) Jimmy Larsson 1998
 *
 */

#ifndef SWITCHING_H
#define SWITCHING_H

#include "types.h"

#define BUSY_CLEAR  0
#define BUSY_SET    1
#define BUSY_IGNORE 2

/*
 * initialize switching module
 *
 */

void init_switching (void);


/* 
 * switch task
 *
 */

void switch_task (tss_386_t   *tss);


/* 
 * load task, prepare a task to be switched (not needed if switching directly)
 *
 */

void load_task (tss_386_t   *tss, int busy);

/*
 * load idle task
 * Prepare to idle
 *
 */


void load_idle_task (int busy);

#endif









