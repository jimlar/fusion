/*
 * Constants and other interesting stuff for Fusion
 * (c) Jimmy Larsson 1998
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

/* Debug messages control, undef this to remove debugging garabage */
#define __DEBUG_MSGS__

/* Shall we show boot messages?  undef if not */
#define __BOOT_MSGS__

/* GDT selectors */
#define CS_SELECTOR  0x8
#define DS_SELECTOR  0x10
#define TSS_SELECTOR 0x28

#define USER_CS_SELECTOR (0x18 + 3)
#define USER_DS_SELECTOR (0x20 + 3)

/* GDT indices */
#define TSS_INDEX       5
#define GDT_INT_START_INDEX 6

/* must match "entry.S" */
#define GDT_SIZE 256

/* interrupt constants */
#define MAX_INTS      256
#define RESERVED_INTS 32

#define NO_OF_IRQS       16
#define NO_OF_EXCEPTIONS 16

/* Kernel stack */
#define KERNEL_STACK_SIZE 4096

/* User stack init top (reserve 4k for kernel stack) */
#define USER_STACK_TOP   (0x80000000 - 4 - 4096)

/* User stack in kernel mode */
#define PROC_KERNEL_STACK_TOP (0x80000000 - 4)

/* Interrupt handler stack */
#define INT_STACK_SIZE 4096

/* Messages */
#define VERSION_STRING "Fusion - System start: version 0.0.0\n"
#define COPYRIGHT_STRING "(c) Jimmy Larsson 1998-1999\n"

#endif



