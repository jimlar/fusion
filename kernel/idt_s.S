;
; idt_s.S
; Assembler support functions for managing IDT
; Also contains stubs for exceptions, system call and hardware interrupts
; (c) Jimmy Larsson 1998
;
;

; Exported stuff
GLOBAL _idt, load_idtr, __bad_int

GLOBAL __hw_irq_0, __hw_irq_1, __hw_irq_2, __hw_irq_3, __hw_irq_4
GLOBAL __hw_irq_5, __hw_irq_6, __hw_irq_7, __hw_irq_8, __hw_irq_9
GLOBAL __hw_irq_10, __hw_irq_11, __hw_irq_12, __hw_irq_13, __hw_irq_14
GLOBAL __hw_irq_15

GLOBAL __exception_0, __exception_1, __exception_2, __exception_3, __exception_4
GLOBAL __exception_5, __exception_6, __exception_7, __exception_8, __exception_9
GLOBAL __exception_10, __exception_11, __exception_12, __exception_13, __exception_14
GLOBAL __exception_16, __system_call_entry

; Imported stuff
EXTERN master_irq_handler, master_exception_handler, master_system_call_handler


; Constants (must match idt.h)
BAD_INT    equ   0x1024

	
load_idtr:
	lidt	[idt_ptr]
	ret


; The Table
ALIGN 16

_idt:
	times 256  dd 0,0

; Pointer and limit
ALIGN 16

idt_ptr:
	dw	8*256-1
	dd	_idt


; Used by macros
pop_dummy:	
	dd	0
		
;
; Macro for harware irq handler stubs
; Everything serious is done by C-code in kernel/idt.c
; (stacks should be changed, maybe by a task switch TSS)
;

%macro hardware_irq_stub 1
.loop
       push long %1
       call master_irq_handler
       pop  long [pop_dummy]
       iretd
       jmp .loop
%endmacro

align 16
__hw_irq_0:
	hardware_irq_stub 0
	
align 16
__hw_irq_1:
	hardware_irq_stub 1

align 16
__hw_irq_2:
	hardware_irq_stub 2

align 16
__hw_irq_3:
	hardware_irq_stub 3

align 16
__hw_irq_4:
	hardware_irq_stub 4

align 16
__hw_irq_5:
	hardware_irq_stub 5

align 16
__hw_irq_6:
	hardware_irq_stub 6

align 16
__hw_irq_7:
	hardware_irq_stub 7

align 16
__hw_irq_8:
	hardware_irq_stub 8

align 16
__hw_irq_9:
	hardware_irq_stub 9

align 16
__hw_irq_10:
	hardware_irq_stub 10

align 16
__hw_irq_11:
	hardware_irq_stub 11

align 16
__hw_irq_12:
	hardware_irq_stub 12

align 16
__hw_irq_13:
	hardware_irq_stub 13

align 16
__hw_irq_14:
	hardware_irq_stub 14

align 16
__hw_irq_15:
	hardware_irq_stub 15


;
; Exception stubs
;
;
; Macros for exception handler stubs
; Everything serious is done by C-code in kernel/idt.c
; (stacks should be changed, maybe by a task switch TSS)
;
; No-error exception get error-code 0 for consistency

%macro exception_stub 1
.loop
       push long 0
       push long %1
       call master_exception_handler
       pop  long [pop_dummy]
       pop  long [pop_dummy]
       iretd
       jmp .loop
%endmacro

%macro error_exception_stub 1
.loop
       push long %1
       call master_exception_handler
       pop  long [pop_dummy]
       pop  long [pop_dummy]
       iretd
       jmp .loop
%endmacro

align 16
__exception_0:
	exception_stub 0

align 16
__exception_1:
	exception_stub 1

align 16
__exception_2:
	exception_stub 2

align 16
__exception_3:
	exception_stub 3

align 16
__exception_4:
	exception_stub 4

align 16
__exception_5:
	exception_stub 5

align 16
__exception_6:
	exception_stub 6

align 16
__exception_7:
	exception_stub 7

align 16
__exception_8:
	error_exception_stub 8

align 16
__exception_9:
	exception_stub 9

align 16
__exception_10:
	error_exception_stub 10

align 16
__exception_11:
	error_exception_stub 11

align 16
__exception_12:
	error_exception_stub 12

align 16
__exception_13:
	error_exception_stub 13

align 16
__exception_14:
	error_exception_stub 14

align 16
__exception_16:
	exception_stub 16


;
; This is used to signal BAD int!
;
	
align 16
__bad_int:
	exception_stub BAD_INT


;
; System call entry point
;
;


align 16
__system_call_entry:
.loop			

	push edx
	push ecx
	push ebx
	push eax

	call master_system_call_handler

	;; WHY! is this nececary???
	;pushf
	;or [esp], long 0x4000
	;popf

	pop long [__syscall_junk]
	pop long [__syscall_junk]
	pop long [__syscall_junk]
	pop long [__syscall_junk]

	; return value is put in eax by gcc, 
	; it will still be there after iretd
	

	iretd
	jmp .loop

__syscall_junk: 
	dd	0




