;
; entry.S
; ------------------------------
; System entry point for Fusion
; (c) Jimmy Larsson 1998
;

GLOBAL   system_entry, _gdt
EXTERN   kernel_stacktop, system_start, init_paging

	
system_entry:
	pop	long [arguments]	; Save loader args

	lgdt	[gdt_ptr]		;  Load with our descriptors

	;; Set segment registers

	mov	ax, flat_data
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax

	;; Set code segment with a jump
	jmp	0x8:next

next:		
	push	ds			; Set stack-segment to DS
	pop	ss		
	mov	esp, kernel_stacktop	; Setup stack pointer

	;; Paging setup moved to "start.c"

	;; Go C

	push	long [arguments]	; Put args on stack
	call    system_start		; Jump to 'start.c'
	
	;; Halt if system_start returns
	jmp	$

;------------------------------------------------------
; The GDT. Should not be dependant of bootblock anymore
;------------------------------------------------------

	
	;; GDT indices

flat_code  equ  0x08
flat_data  equ  0x10

	;; The loader args

arguments:
	dd	0 

	;; Global Descriptor Table
	
	ALIGN	2
	dw	0	

gdt_ptr:
	dw	8 * 256    	; gdt limit
	dd	null_seg	; gdt base

_gdt:	
null_seg:							
	dw	0,0,0,0		; dummy

flat_code_seg:
	dw	0xFFFF		; 4Gb - limit
	dw	0x0000		; base address=0
	dw	0x9A00		; code read/exec, dpl = 0
	dw	0x00CF		; granularity=4096, 386

flat_data_seg:
	dw	0xFFFF		; 4Gb - limit
	dw	0x0000		; base address=0
	dw	0x9200		; data read/write, dpl = 0
	dw	0x00CF		; granularity=4096, 386

flat_user_code_seg:
	dw	0xFFFF		; 4Gb - limit
	dw	0x0000		; base address=0
	dw	0xFA00		; code read/exec, dpl = 3
	dw	0x00CF		; granularity=4096, 386

flat_user_data_seg:
	dw	0xFFFF		; 4Gb - limit
	dw	0x0000		; base address=0
	dw	0xF200		; data read/write, dpl = 3
	dw	0x00CF		; granularity=4096, 386
	
tss_seg:
	dw	0,0,0,0		; Filled in later

        times 252 dw 0,0,0,0	;  Reserve space for more descriptors (total of 256)

