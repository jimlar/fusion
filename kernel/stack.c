/*
 * kernel stack space
 *
 */

#include "config.h"
#include "stack.h"


char    kernel_stack[KERNEL_STACK_SIZE];
char   *kernel_stacktop = &kernel_stack[KERNEL_STACK_SIZE - 1];


char    int_stack[INT_STACK_SIZE];
char   *int_stacktop = &int_stack[INT_STACK_SIZE - 1];
