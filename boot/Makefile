#
# Makefile for Fusion
# boot parts...
# (c) Jimmy Larsson 1998-1999
#

NASM86	=nasm -f bin

LD	=ld
LDFLAGS	=-Ttext 0xa000 -e start_loader -oformat binary -nostdlib
OBJCOPY =objcopy
CC	=gcc
CFLAGS	=-Wall -m386 -O2 -fstrength-reduce -fomit-frame-pointer

all:    boot loader32


# Boot stage 1, bootblock
boot:	boot.S
	$(NASM86) -o boot.b boot.S


# 32 bit c-version of stage 2

loader32: loader32.c loader32.h printk.c printk.h fat.c fat.h bios_read.c bios_read.h disk_io.c disk_io.h vga.h vga.c pcx.h pcx.c
	$(CC) $(CFLAGS) -c -o loader32.o loader32.c
	$(CC) $(CFLAGS) -c -o printk.o printk.c 
	$(CC) $(CFLAGS) -c -o fat.o fat.c 
	$(CC) $(CFLAGS) -c -o bios_read.o bios_read.c 
	$(CC) $(CFLAGS) -c -o disk_io.o disk_io.c 
	$(CC) $(CFLAGS) -c -o vga.o vga.c 
	$(CC) $(CFLAGS) -c -o pcx.o pcx.c 
	$(LD) $(LDFLAGS) -o loader loader32.o printk.o fat.o bios_read.o disk_io.o vga.o pcx.o


# helpers to debug stage 2 loader
loader32_bochs: loader32
		mount /mnt/hos.floppy
		cp -f loader /mnt/hos.floppy/loader
		umount /mnt/hos.floppy
		bochs


clean:
	rm -f boot.b *~ loader *.o core

install: boot
	(dd if=boot.b of=/dev/fd0 bs=1 count=3 ; dd if=boot.b of=/dev/fd0 bs=1 seek=62 skip=62)
	(sync)

install_bochs: boot
	(dd if=boot.b of=/home/jimmy/tmp/floppy.tmp bs=1 count=3)
	(dd if=/home/jimmy/tmp/floppy of=/home/jimmy/tmp/floppy.tmp bs=1 seek=3 skip=3 count=62)
	(dd if=boot.b of=/home/jimmy/tmp/floppy.tmp bs=1 seek=62 skip=62)
	(dd if=/home/jimmy/tmp/floppy of=/home/jimmy/tmp/floppy.tmp bs=512 skip=1 seek=1)
	(cp -f /home/jimmy/tmp/floppy.tmp /home/jimmy/tmp/floppy ; rm -f /home/jimmy/tmp/floppy.tmp)
	(bochs)



