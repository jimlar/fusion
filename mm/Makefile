#
# Memeory management Makefile 
# (c) Jimmy Larsson 1998
#



AR	=ar
NASM	=nasm -f elf
CC	=gcc
CFLAGS	=-Wall -O2 -m386 -nostdlib -nostdinc -I../include -DASSERT
CPP	=gcc -E -nostdlib -nostdinc -I../include -DASSERT
RANLIB	=ranlib

MM_OBJS = pages.o vm.o kmalloc.o


.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.S.o:
	$(NASM) -o $*.o $<


all:	mm

mm:	$(MM_OBJS)
	$(AR) rs libmm.a $(MM_OBJS)
	$(RANLIB) libmm.a


clean:
	rm -f *~ *.o $(MM_OBJS) tmp_make libmm.a

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make

### Dependencies:
kmalloc.s kmalloc.o: kmalloc.c pages.h ../kernel/types.h ../kernel/printk.h \
 ../kernel/config.h ../include/stdarg.h kmalloc.h
pages.s pages.o: pages.c pages.h ../kernel/types.h ../kernel/printk.h \
 ../kernel/config.h ../include/stdarg.h ../kernel/panic.h \
 ../kernel/gdt.h ../kernel/idt.h ../kernel/../mm/pages.h \
 ../kernel/process.h ../kernel/../mm/vm.h \
 ../kernel/../mm/../kernel/types.h ../kernel/../mm/../kernel/elf.h
vm.s vm.o: vm.c vm.h ../kernel/types.h ../kernel/elf.h pages.h \
 ../kernel/printk.h ../kernel/config.h ../include/stdarg.h \
 ../kernel/panic.h ../kernel/gdt.h ../kernel/idt.h \
 ../kernel/../mm/pages.h ../kernel/interrupt.h ../kernel/process.h \
 ../kernel/../mm/vm.h
