/*
 * Loader for FUSION
 * This is the rewritten 32bit version in C
 * I had to do this since the assembler version was
 * starting to get out of hand, with this version I can lift
 * the loading to a more useable level with dynamic configuration etc.
 *
 * Uses a jump to 16-bit and bios calls to actually load stuff
 *
 * 
 *
 * (c) 1998 Jimmy Larsson
 *
 */

#ifndef LOADER32_H
#define LOADER32_H

/* NULL */
#define NULL 0L

/* max/min macros */
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

/* Boot logo file */
#define BOOT_LOGO_FILE      "FUSION.PCX"
/* load at 2Mb */
#define BOOT_LOGO_LOAD_ADDR (0x100000 * 2)
/* decode at 3Mb */
#define BOOT_LOGO_DECODE_ADDR (0x100000 * 3)

/* Here I store the boot drive number from the boot block */
#define BIOS_LOAD_DRIVE    0x99999

#define BB_LOAD_ADDR       0x7c00

/* Where to store temporary data when loading */
#define LOAD_BUFFER_ADDR   0x40000

/* Assumes loader32 will not be larger than 24kb */
#define DIR_LOAD_ADDR      0x10000

/* Where to start loading system (at 1MB) */
#define SYSTEM_LOAD_ADDR   0x100000

#define PAGE_SIZE          0x1000

/* this should be more than enough, 2 should actually do the trick */
#define MAX_PHT_ENTRIES    10

#define outb(value,port) \
__asm__ ("outb %%al,%%dx \
         jmp     $+2"::"a" (value),"d" (port))

#define outw(value,port) \
__asm__ ("outw %%ax,%%dx \
          jmp     $+2"::"a" (value),"d" (port))

#define inb(port) ({ \
   unsigned char _v; \
   __asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
   _v; \
})    

#define inw(port) ({ \
   unsigned short _v; \
   __asm__ volatile ("inw %%dx,%%ax":"=a" (_v):"d" (port)); \
   _v; \
})    

/*
 * panic
 * Print message and halt
 *
 */

void panic (char *mess);


void main (void);

#endif




