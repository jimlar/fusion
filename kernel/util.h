/*
 * Kernel utilities
 * (c) Jimmy LArsson 1999
 *
 */

#ifndef UTIL_H
#define UTIL_H



/*
 * small string compare
 *
 */ 

int string_cmp (char   *str1, 
		char   *str2);


/* 
 * get CR3 utility 
 *
 */
long get_cr3 (void);

/* 
 * get CR2 utility 
 *
 */
long get_cr2 (void);

/*
 * Enable interrupts
 */

#define sti() __asm__("sti")

/*
 * Disable interrupts
 */

#define cli() __asm__("cli")



#endif
