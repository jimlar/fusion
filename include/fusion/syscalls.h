/*
 * System call numbers for Fusion
 * (c) 1998 Jimmy Larsson
 *
 */

#ifndef FUSION_SYSCALLS_H
#define FUSION_SYSCALLS_H


/* 
 * System call numbers
 *
 */

/* Crappy debug call */
#define _SYSCALL_NR_putchar 1000


/* 
 * Process management syscalls
 */
#define _SYSCALL_NR_fork    101

/*
 * Signal syscalls
 */

/*
 * File management calls
 */
#define _SYSCALL_NR_open    301


/*
 * Protection syscalls
 */

#define _SYSCALL_NR_umask   401

/*
 * Time management syscalls
 */


/*
 * System call macros. Used to produce userside of systemcall
 *
 * TYPE0:
 *   This is a systemcall without parameters.
 *   Returns -1 on errors, status code otherwise.
 *
 * TYPE1:
 *   This is a systemcall with one parameter.
 *   Returns -1 on errors, status code otherwise.
 *
 * TYPE2:
 *   This is a systemcall with two parameter.
 *   Returns -1 on errors, status code otherwise.
 *
 * TYPE3:
 *   This is a systemcall with three parameter.
 *   Returns -1 on errors, status code otherwise.
 *
 *
 */

#define SYSTEM_CALL_TYPE0(type,name) \
type name(void) \
{ \
  type result; \
  __asm__ volatile ("int $0x40" \
		    : "=a" (result) \
		    : "0" (_SYSCALL_NR_##name)); \
  if (result >= 0) \
    return result; \
  \
  return -1; \
}

#define SYSTEM_CALL_TYPE1(type,name,atype,a) \
type name(atype a) \
{ \
  type result; \
  __asm__ volatile ("int $0x40" \
		    : "=eax" (result) \
		    : "0" (_SYSCALL_NR_##name),"ebx" (a)); \
  if (result >= 0) \
    return result; \
\
  return -1; \
}        

#define SYSTEM_CALL_TYPE2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
  type result; \
  __asm__ volatile ("int $0x40" \
		    : "=eax" (result) \
		    : "0" (_SYSCALL_NR_##name),"ebx" (a),"ecx" (b)); \
  if (result >= 0) \
    return result; \
\
  return -1; \
} 

#define SYSTEM_CALL_TYPE3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
  type result; \
  __asm__ volatile ("int $0x40" \
		    : "=eax" (result) \
		    : "0" (_SYSCALL_NR_##name),"ebx" (a),"ecx" (b),"edx" (c)); \
  if (result < 0) \
  { \
    result = -1; \
  } \
 \
  return result; \
}



#endif
