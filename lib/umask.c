/*
 * Fusion umask library call
 *
 *
 */

#include <fusion/syscalls.h>
#include <sys/stat.h>

/*
 * UMASK syscall 
 *
 * int umask (int mask);
 */
SYSTEM_CALL_TYPE1(int,umask,int,mask)
