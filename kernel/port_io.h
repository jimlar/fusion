/*
 * Port level IO for Fusion
 * (c) Jimmy Larsson 1998
 * 
 */

#ifndef PORT_IO_H
#define PORT_IO_H

#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

#define inb(port) ({ \
   unsigned char _v; \
   __asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
   _v; \
})    

#define outw(value,port) \
__asm__ ("outw %%ax,%%dx"::"a" (value),"d" (port))

#define inw(port) ({ \
   unsigned short _v; \
   __asm__ volatile ("inw %%dx,%%ax":"=a" (_v):"d" (port)); \
   _v; \
})    

#endif
