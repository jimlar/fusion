#
# Makefile for Fusion
#

AR	=ar
NASM	=nasm -f elf
AS	=as
LD	=ld

# The kernel is loaded at 0x100000, so we'll link for that
LDFLAGS	=-s -x -Ttext 0x100000 -e system_entry -nostdlib

CC	=g++
CFLAGS	=-Wall -O2 -fstrength-reduce -fomit-frame-pointer -m386
CPP	=gcc -E
OBJDUMP =objdump
OBJCOPY =objcopy
OCFLAGS = --remove-section=.note --remove-section=.comment --output-target=binary


# Objects of the system
SYSTEM_OBJS = kernel/entry.o kernel/libkernel.a mm/libmm.a drv/libdrv.a

# All subdirs
SUBDIRS = kernel mm drv


all:	dep system

system:	$(SYSTEM_OBJS)
	$(LD) $(LDFLAGS) -o system $(SYSTEM_OBJS)


# The kernel code
kernel/libkernel.a:
	(cd kernel; make)


# Memory management
mm/libmm.a:
	(cd mm; make)


# Drivers
drv/libdrv.a:
	(cd drv; make)


# The system entry code
kernel/entry.o:
	(cd kernel; make entry;)



clean:
	for i in $(SUBDIRS); do (cd $$i && make clean); done;
	(cd boot;make clean; cd ..)
	rm -f core system.elf system.bin *~ tmp_make system

dep:
	for i in $(SUBDIRS); do (cd $$i && make dep); done;


bochs:
	(make clean;make;mount /mnt/hos.floppy;cp -f system init/init.sys /mnt/hos.floppy/;umount /mnt/hos.floppy;bochs)



