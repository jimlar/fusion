#
# Init Makefile 
# (c) Jimmy Larsson 1999
#

CC	=gcc
CFLAGS	=-Wall -O2 -m386 -I../include -nostdinc
CPP	=gcc -E -I../include -nostdinc
LD      =ld
LDFLAGS =-s -x -e init_main -nostdlib


FUSION_LIB = ../lib/libfusion.a
INIT_OBJS = init.o

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.S.o:
	$(NASM) -o $*.o $<


all:	dep init

init:   $(INIT_OBJS) $(FUSION_LIB)
	$(LD) $(LDFLAGS) -o init.sys $(INIT_OBJS) $(FUSION_LIB)


$(FUSION_LIB):
	cd ../lib; make dep; make

clean:
	rm -f *~ *.o $(INIT_OBJS) init.sys tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make

### Dependencies:
init.s init.o: init.c ../include/stdio.h ../include/stdarg.h \
 ../include/unistd.h
