#
# Drivers makefile
# (c) Jimmy Larsson 1999
#



AR	=ar
NASM	=nasm -f elf
CC	=gcc
CFLAGS	=-Wall -O2 -m386 -nostdlib -nostdinc -I../include -DASSERT
CPP	=gcc -E -nostdlib -nostdinc -I../include -DASSERT
RANLIB	=ranlib

MM_OBJS = keyboard.o


.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.S.o:
	$(NASM) -o $*.o $<


all:	drv

drv:	$(MM_OBJS)
	$(AR) rs libdrv.a $(MM_OBJS)
	$(RANLIB) libdrv.a


clean:
	rm -f *~ *.o $(MM_OBJS) tmp_make libdrv.a

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make

### Dependencies:
keyboard.s keyboard.o: keyboard.c ../kernel/port_io.h ../kernel/interrupt.h \
 ../kernel/types.h ../kernel/printk.h ../kernel/config.h \
 ../include/stdarg.h
