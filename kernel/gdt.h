/*
 * gdt.h
 *
 * Header file for accessing gdt, located in entry.S
 * (c) Jimmy Larsson 1998
 *
 */

#ifndef GDT_H
#define GDT_H

#define GDT_BYTE_SIZE 256*8
extern gate_t  _gdt;


#endif

