/*
 * Fusion fork library call
 *
 *
 */

#include <fusion/syscalls.h>
#include <unistd.h>

/*
 * FORK syscall 
 *
 * pid_t fork (void);
 */
SYSTEM_CALL_TYPE0(pid_t,fork)
