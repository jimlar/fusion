/*
 * kernel stack space
 *
 */


#ifndef STACK_H
#define STACK_H


/* initial stack */
extern char    kernel_stack[];
extern char   *kernel_stacktop;

/* used by int_handler tasks */
extern char    int_stack[];
extern char   *int_stacktop;



#endif
