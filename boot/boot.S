;
; Boot block for Fusion
; (c) Jimmy Larsson 1998
;
;-----------------------------------------------------------------------
;
; For now this loads a file in root of a FAT-16 filesys,
; file should be called 'loader'
;
;----------------------------------------------------------------------
;
; Loads image to 0xa000 and sets up a tiny protected mode environment
; then jumps to 0xa000
;
;-----------------------------------------------------------------------
;
; 980719 - It's now possible to boot all drives without reassembling.
;          Just change byte in end of the file with some poking tool...
;
; 980913 - Added retries to sector-read code seemed to bomb on real
;          machinery when only read once. (hdd boot code removed for now)
;
; 981220 - Removed the "unreal-mode" parts of the code since they are 
;          obsolete now when the loader32 is loaded to 0xa000
;          (The old code is stored in the archive tar's) 
;
; 981220 - Automatic sensing of the boot drive number added.
;
; 981220 - Added fix for floppy reads across cylinder/head boundaries
;
; 981229 - Rewrote much of the read_sectors() function, still not working
;          very good
;
; 981230 - Moved A20 enabling to loader32
;
; 990418 - Added FDC reset at start (bugged on some BIOS'es without it)
;
; 990418 - Changed read_sectors to read only a sector at a time with BIOS
;          call. Makes code simpler and it's fast enough anyway
;
; 990418 - Added print of "." for each sector read, nice for debugging
;
; 990428 - Removed some shit, reduced code size
;
;-----------------------------------------------------------------------
;
; This bootblock only works with FAT-16, will not use FAT later anyway.. =)
;
; To make FAT16 floppy in linux -> "mkfs.msdos -v -F 16 /dev/fd0"
; To mount it in Linux          -> "mount -t msdos -o fat=16 /dev/fd0 /mnt/floppy"
;
; IT HAS TO BE MOUNTED WITH THE fat=16 OPTION OR LINUX WILL MESS UP THE FLOPPY
; (Shit... pulled my hair for some time before I found out this)
;
;-----------------------------------------------------------------------

; Floppy parameters (1.44)
SECTORS_1_44	  equ	18
HEADS_1_44	  equ	2
CYLINDERS_1_44	  equ	160

; Addresses
LOAD_SEG	  equ	0xa00
LOAD_ADDR	  equ	0xa000

; When we follow FAT chain, clusters are temporary loaded to this place
TMP_CLUSTER_SEG   equ   0xa0
TMP_CLUSTER_ADDR  equ   0xa00

; Store the BIOS drivenumber of the boot drive here
BOOT_DRIVE        equ   0x99999

; GDT indices
FLAT_CODE         equ   0x8




struc	DISK_PARM	;BIOS hard disk parameters
		DP_cyl		resw	1
		DP_heads	resb	1
				resb	5	;Obsolete
		DP_ctrl		resb	1
				resb	5	;Obsolete
		DP_sect		resb	1
				resb	1
endstruc

struc	PT_ENT		;Partition table entry
		BootFlag	resb	1
		BeginHead       resb	1
		BeginSector     resb	1
		BeginCyl        resb	1
		SystemID        resb	1
		EndHead         resb	1
		EndSector       resb	1
		EndCyl          resb	1
		RelSectorLow    resw	1
		RelSectorHigh   resw	1
		NumSectorsLow   resw	1
		NumSectorsHigh  resw	1
endstruc

struc	BB		;Partition Boot block (FAT Specific)
				resb	0xD	;Things we ignore
		BB_clu		resb	1	;Sectors per cluster
		BB_res		resw	1	;Reserved sectors
		BB_fats		resb	1	;Number of FATs
		BB_root		resw	1	;Root directory entries
				resb	3	;Things we ignore
		BB_fat		resw	1	;Sectors per fat
endstruc


;-----------------------------------------------------------------------
; The code
;

section .text use 16
org     0x7c00

	jmp short start			;Standard start of boot sector
	nop
	resb	0x3B			;Skip over parameters (set by format)

start:	cli				
	mov     byte [BOOT_DRIVE],dl    ; BIOS puts boot drive in DL 


;; Reset FDC (really important, bugs out on some BIOS'es otherwise)
	
        xor     ah,ah                   
        int     0x13
             
;; Load ourselves, could probably be removed

	mov	al, 0x80		;Segment for boot blocks
	mov	es, ax			

	xor	eax, eax		;Segment and LBN
	mov	ds, ax			
	mov	ss, ax
	mov	sp, start
					;[0x7c79]
;; Print loading message

	mov	si, loading_message
	call	print

;; Read bootblock of this partition (we are floppy or part. bootblobk)

	xor	edi, edi	
	inc	di			;Count of sectors
	call	read_sectors		;Read the boot block (ourselves)
					; [0x7c80]
;---------------------------
; Read DIRECTORY into 0x8000
;---------------------------
		

	mov	bp, 0x800		;Point at partition boot block
	mov	bx, [bp+BB_res]		;Sectors before first FAT
	add	eax, ebx		;LBN of FAT
	push	eax			;Need to read the FAT later
	movzx	bx, byte [bp+BB_fats]	;Number of FATs
	imul	bx, word [bp+BB_fat]	;Times size of FAT
	add	eax, ebx		;LBN of Root directory
	mov	di, word [bp+BB_root]	;Root directory entries

	push	di			; used again later
	dec	di			;Convert to number of sectors
	shr	di, 4			;  16 directory entries per sector
	inc	di

	mov	es, bp			;Read directory at 800:0
					;[0x7ca1]
	call	read_sectors
	add	eax, edi		;LBN of cluster 2
					;[0x7ca7]

;----------------------------------
; Search the directory for filename
;----------------------------------

	pop	bx			;Root directory entries
	xor	di, di			;Point at directory

.20:	
	mov	si, file_name		;Name of file we want
	xor	ecx, ecx
	mov	cl, 11
	a32 rep cmpsb			;Found the file?
	je	found			;Yes
	add	cl, 21			;Offset to next directory entry
	add	edi, ecx		;Advance to next entry
	dec	bx			;Loop through all entries
	ja	.20			;Jmp if above zero

	;Couldn't find file in directory

	jmp     disk_error

;------------------------------------------
; Ok, found. Read FAT into memory at 0x8000
;------------------------------------------

found:					;[0x7cc3]
	mov	bx, [es:edi+0xF]	;Starting cluster of file
	mov	di, [bp+BB_fat]		;Size of FAT (in sectors)
	xchg	eax, ebp		;ebp = LBN of cluster 2
	pop	eax			;LBN of FAT
	push	bx			;Starting cluster of file
	call	read_sectors
					;[0x7cd3]
	mov	bx, TMP_CLUSTER_SEG
	mov	es, bx			;es = 0xa0
	mov	edi, LOAD_ADDR-TMP_CLUSTER_ADDR	;LOAD_ADDR minus ES base
					;[0x7cdd]


;--------------------------------------------------------
; Follow linked list in FAT and load all clusters of file
;--------------------------------------------------------

.10:
	xor	eax, eax
	pop	si			;Next cluster of file
	dec	si
	dec	si

	cmp	si, byte 0xFFFFFFEE 	;Valid cluster?
	jae	eof			;No: assume end of file

	push word [esi*2+0x8004]	;Cluster after it
	push	di			;Save destination address
	mov	al, [0x800+BB_clu]	;Size of each cluster
	mov	di, ax			;  (in sectors)
	mul	esi			;Times current cluster number
	add	eax, ebp		;Plus LBN of cluster 2
					;[0x7cfc]
	call	read_sectors		;Read that cluster
					;[0x7cff]

;; Move read cluster to it's real load address

	mov	cx, di			;Cluster size in sectors
	xchg	ch, cl			;Cluster size in words
	pop	di			;Restore destination address
	es a32 rep movsw
	jmp short .10			;Loop until end of file
					;[0x7d0e]

;---------------------------
; File read, jump to stage 2
;---------------------------

eof:				; [0x7d0a]
	cli

	mov   ax, 0x12		; Video mode now set by loader32
	int   0x10		
	cli
	
	lgdt  [cs:gdt]		;Load GDT
    
	;; Loader 32 requires PM            
        mov   eax, cr0
        inc   eax
        mov   cr0, eax

	jmp   dword FLAT_CODE:LOAD_ADDR   ; Go to stage 2 32-bit 

      
read_sectors:
; Input:
;	EAX = LBN
;	DI  = sector count
;	ES = segment
; Output:
;	BL = low byte of ES
;	EBX high half cleared
;	DL = 0x80
;	EDX high half cleared
;	ESI = 0
; Clobbered:
;	BH, CX, DH

	push	eax
	push	di
	push	es

.10:	push	eax		;LBN
	push	di


        cmp     byte [BOOT_DRIVE], 0x80
	jb	.fdd			; jump if less than.

; LBN -> CHS translation, harddrive specific...

.hdd:
	; Have not quite tested this hdd code, hopefully it works (grin)
	
	lds	si, [0x104]	       ;Hard drive 0 parameters
	cdq			       ;edx = 0
	movzx	ebx, byte [si+DP_sect]
	div	ebx		       ;EAX=track ;EDX=sector-1
	mov	bl, [si+DP_heads]
	jmp	.indep

; LBN -> CHS, the floppy way....

.fdd:
	cdq			     ;edx = 0
	mov	ebx, SECTORS_1_44
	div	ebx		     ;EAX=track ;EDX=sector-1
	mov	bl, byte HEADS_1_44


; Independent code of translation...

.indep:
	mov	cx, dx		;CL=sector-1 ;CH=0
	inc	cx		;CL=Sector number

	xor	dx, dx
	div	ebx

	mov	dh, dl		;Head
	mov	dl, [BOOT_DRIVE];Select drive 
	xchg	ch, al		;CH=Low 8 bits of cylinder number; AL=0
	shr	ax, 2		;AL[6:7]=High two bits of cylinder; AH=0
	or	cl, al		;CX = Cylinder and sector

	;;	
	;; Transfer one sector at a time, for now
	;; - Since the biggest thing we load is the loader32
	;;   this should do since loader32 is about 9k size

	mov     al, 1
.20:	
	mov	ah, 2		;Read
	xor	bx, bx

	mov	byte [retry_count], 5   ; Retry IO 5 times

.reread:

	pusha		; Push incase of retries

	int	0x13

	jc	.retry
	
.no_read_error:	

	add     esp, 16	; Throw away pushed stuff

;; Print a dot

	push	si
	mov	si, loading_dot
	call	print
	pop	si

	mov	bx, es	; One sector = 512 byte = 32 * 16 byte
	add	bx, 32	; Advance segment
	mov	es, bx

	pop	eax
	inc	eax

	pop	di
	sub	di, 1
	ja	near .10

	pop	es
	pop	di
	pop	eax
	xor	esi, esi
	ret

.retry:
	sub	byte [retry_count], 1
	jz	disk_error

	xor	ax,ax	; reset disk controller
	int     0x13

	popa		; Restore args for int 0x13,2
	jmp     short .reread

disk_error:
	mov	si, error_message
	call    print
	jmp short $

;
; Print message at dx:si
;
print:
	pusha
.loop:
	lodsb
	or	al,al
	jz	.done
	mov	ah, 0x0e	
	mov	bx, 0x7
	int	0x10
	jmp     .loop
.done:
	popa
        ret

     	
; FAT name      12345678123
file_name   db 'LOADER     '	

; Retry count (for bios disk routines)
retry_count db 0

; Boot message
loading_message db 'Fusion',0
loading_dot     db '.',0

; Error message 
error_message   db 'E',0

; Global Descriptor Table
gdt:
	dw	0x10		; gdt limit=2048, 2 GDT entries
	dw	null_seg,0x0    ; gdt base

null_seg:							
	dw	0,0,0,0		; dummy

flat_code_seg:
	dw	0xFFFF		; 4Gb - limit
	dw	0x0000		; base address=0
	dw	0x9A00		; code read/exec
	dw	0x00CF		; granularity=4096, 386

	resb 0x1FD+$$-$		

	db	0x55, 0xAA	;Standard end of boot sector

