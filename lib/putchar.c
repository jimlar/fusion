/*
 * Fusion putchar library call
 *
 * (Only for debug, will not have this call later on)
 */

#include <fusion/syscalls.h>
#include <stdio.h>

/* PUTCHAR syscall */
SYSTEM_CALL_TYPE1(int,putchar,int,chr)
