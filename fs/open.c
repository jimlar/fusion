/*
 * open.c
 *
 * Fusion open filesystem function
 *
 * (c) 1999 Jimmy Larsson
 *
 */

#include "../kernel/process.h"


int sys_open (const char *pathname, int flags, int mode)
{
  int i;

  /* mask mode with current umask */
  mode &= 0777 & ~(current_proc->umask);

  /* Get free file pointer */
  for (i = 0; i < MAX_OPEN; i++) {
    if (!current_proc->files[i])
      break;
  }

  /* No free found */
  if (i >= MAX_OPEN)
    return -1;

  
  
  
}
