/*
 * System call handler for Fusion
 * (c) 1998 Jimmy Larsson
 *
 */


#include <fusion/syscalls.h>
#include <fusion/types.h>
#include "types.h"
#include "config.h"
#include "gdt.h"
#include "printk.h"
#include "syscall.h"
#include "switching.h"
#include "fork.h"

#include "process.h"




int master_system_call_handler (unsigned int   call_num, 
				unsigned int   arg1, 
				unsigned int   arg2, 
				unsigned int   arg3)
{
  char str[2];
  int  ret_value = -1;

  switch (call_num)
  {

    /** This is only a debug syscall, will be removed */
  case _SYSCALL_NR_putchar:
    str[0] = (char) arg1;
    str[1] = '\0';
    printf (str);
    ret_value = arg1;
    break;

  case _SYSCALL_NR_fork:
    ret_value = do_fork(current_proc);
    break;

  case _SYSCALL_NR_umask:
    ret_value = current_proc->umask;
    current_proc->umask = arg1;
    break;


    /* FILE SYSTEM */
  case _SYSCALL_NR_open:
    //put_return (current_proc, sys_open ((char *) (arg1), arg2, arg3));
    break;
  }

  return ret_value;
}




