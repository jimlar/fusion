#
# FUSION library Makefile 
# (c) Jimmy Larsson 1998-1999
#



AR	=ar
NASM	=nasm -f elf
CC	=gcc
CFLAGS	=-Wall -O2 -m386 -nostdlib -nostdinclude -I../include
CPP	=gcc -E -nostdinclude -I../include
RANLIB	=ranlib

LIB_OBJS = putchar.o printf.o fork.o umask.o


.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.S.o:
	$(NASM) -o $*.o $<


all:	lib

lib:	$(LIB_OBJS)
	$(AR) rs libfusion.a $(LIB_OBJS)
	$(RANLIB) libfusion.a


clean:
	rm -f *~ *.o $(LIB_OBJS) tmp_make libfusion.a

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make

### Dependencies:
fork.s fork.o: fork.c ../include/fusion/syscalls.h ../include/unistd.h
printf.s printf.o: printf.c ../include/stdio.h ../include/stdarg.h
putchar.s putchar.o: putchar.c ../include/fusion/syscalls.h ../include/stdio.h \
 ../include/stdarg.h
umask.s umask.o: umask.c ../include/fusion/syscalls.h ../include/sys/stat.h
