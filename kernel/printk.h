/*
 * Printing utilities for Fusion
 * (c) Jimmy Larsson 1998
 * 
 */

#ifndef PRINT_K_H
#define PRINT_K_H

/* include to read 'MSGS' flags */
#include "config.h"
/* for va_list's */
#include <stdarg.h>


/*
 * set_wrapline
 * Set wrap line to current line
 *
 */

void set_wrapline (void);


/* 
 * fill screen with spaces 
 * 
 */

void clear_screen (void);


/**
 ** Printf functions (minimal implementation of them)
 **
 ** %s - string     %d - signed int    %x - 32bit hex number (0 padded)
 ** %c - character  %u - unsigned int  %X -  8bit hex number (0 padded) 
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

