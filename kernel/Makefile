#
# Kernel Makefile 
# (c) Jimmy Larsson 1998
#

#
# The entry.S lies outside of the libkernel.a, 
# because I can't get ld to link only libraries...
#

AR	=ar
NASM	=nasm -f elf
CC	=gcc
CFLAGS	=-Wall -O2 -m386 -I../include -nostdinc -DASSERT
CPP	=gcc -E -I../include -nostdinc
RANLIB	=ranlib

KERNEL_OBJS = stack.o start.o printk.o 8259.o idt.o idt_s.o interrupt.o panic.o switching.o syscall.o process.o fork.o ipc.o util.o assert.o vga.o

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.S.o:
	$(NASM) -o $*.o $<


all:	kernel

kernel: $(KERNEL_OBJS)
	$(AR) rs libkernel.a $(KERNEL_OBJS)
	$(RANLIB) libkernel.a

entry:  entry.S
	$(NASM) -o entry.o entry.S

clean:
	rm -f *~ *.o $(KERNEL_OBJS) entry.o tmp_make libkernel.a

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make

### Dependencies:
8259.s 8259.o: 8259.c config.h 8259.h port_io.h printk.h ../include/stdarg.h
assert.s assert.o: assert.c assert.h util.h printk.h config.h \
 ../include/stdarg.h
fork.s fork.o: fork.c process.h types.h ../mm/vm.h ../mm/../kernel/types.h \
 ../mm/../kernel/elf.h ../mm/pages.h syscall.h printk.h config.h \
 ../include/stdarg.h fork.h
idt.s idt.o: idt.c config.h printk.h ../include/stdarg.h types.h idt.h \
 ../mm/pages.h ../mm/../kernel/types.h gdt.h 8259.h panic.h stack.h \
 util.h
interrupt.s interrupt.o: interrupt.c printk.h config.h ../include/stdarg.h types.h \
 panic.h interrupt.h idt.h ../mm/pages.h ../mm/../kernel/types.h \
 8259.h process.h ../mm/vm.h ../mm/../kernel/elf.h util.h
ipc.s ipc.o: ipc.c ipc.h
panic.s panic.o: panic.c panic.h printk.h config.h ../include/stdarg.h
printk.s printk.o: printk.c printk.h config.h ../include/stdarg.h port_io.h \
 vga.h types.h
process.s process.o: process.c switching.h types.h process.h ../mm/vm.h \
 ../mm/../kernel/types.h ../mm/../kernel/elf.h ../mm/pages.h \
 interrupt.h port_io.h printk.h config.h ../include/stdarg.h ipc.h
stack.s stack.o: stack.c config.h stack.h
start.s start.o: start.c types.h config.h process.h ../mm/vm.h \
 ../mm/../kernel/types.h ../mm/../kernel/elf.h ../mm/pages.h printk.h \
 ../include/stdarg.h interrupt.h panic.h elf.h ../mm/kmalloc.h \
 port_io.h util.h assert.h vga.h
switching.s switching.o: switching.c types.h config.h gdt.h idt.h ../mm/pages.h \
 ../mm/../kernel/types.h switching.h printk.h ../include/stdarg.h \
 util.h
syscall.s syscall.o: syscall.c ../include/fusion/syscalls.h \
 ../include/fusion/types.h types.h config.h gdt.h printk.h \
 ../include/stdarg.h syscall.h process.h ../mm/vm.h \
 ../mm/../kernel/types.h ../mm/../kernel/elf.h ../mm/pages.h \
 switching.h fork.h
util.s util.o: util.c util.h
vga.s vga.o: vga.c vga.h types.h port_io.h
