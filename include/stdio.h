/*
 * FUSION stdio
 * (c) Jimmy Larsson 1999
 *
 */

#ifndef _STDIO_H
#define _STDIO_H

/* VA_LIST */
#include <stdarg.h>


/** 
 ** putchar 
 **
 ** For easy debugging, this is now a direct system call.
 ** It will not be that later, then well user STDIN and STDOUT.
 ** 
 **/
int putchar (int chr);


/**
 ** Printf functions (minimal implementation of them)
 **
 ** %s - string     %d - signed int    %x - 32bit hex number (0 padded)
 ** %c - character  %u - unsigned int  %X -  8bit hex number (0 padded) 
 **
 ** No formatting supported right now
 **
 **/

/*
 * snprintf
 *
 */
 
void snprintf(char *str, int maxlen, char *fmt, ...);

/*
 * sprintf
 *
 */
 
void sprintf(char *str, char *fmt, ...);


/*
 * nprintf
 *
 */
 
void nprintf(int maxlen, char *fmt, ...);

/*
 * printf
 *
 */
 
void printf(char *fmt, ...);




#endif
