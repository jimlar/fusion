/*
 * types.h
 * Kernel Type definitions for Fusion used structures
 * (c) 1998 Jimmy Larsson
 *
 */

#ifndef TYPES_H
#define TYPES_H


/* 
 * Page Frame information (physical memory page, that is) 
 *
 * (This may seem like wasting memory but one of these for
 *  every page in and 8MB computer only takes 32kB, so no worries)
 */

typedef struct page_frame_struct
{
  struct page_frame_struct   *next;
  struct page_frame_struct   *prev;
  short                       age;
  short                       count;
  int                         page_frame_nr;
} page_frame_t;


/*
 * This structure is used to represent blocks of virtual
 * memory
 *
 */

/* flag values */
#define  VM_READONLY  0x0001
#define  VM_READWRITE 0x0002
#define  VM_CODE      0x0004
#define  VM_SYSTEM    0x0008

typedef struct vm_block_struct
{
  unsigned long             vm_start;    /* start address      */
  unsigned long             vm_end;      /* end address        */
  unsigned short            vm_flags;    /* memory block flags */

  int                       count;       /* shared, between how many */

  /* Will probably have inode and function pointers here later */

  struct vm_block_struct   *vm_next;     /* next in list */

} vm_block_t;


/* interrupt handler type */
typedef int  (*irq_handler_t)(int irq);

/* task struct segment type */
typedef struct tss_386_s
{
  unsigned short  backlink, __blh;
  unsigned long   esp0;
  unsigned short  ss0, __ss0h;
  unsigned long   esp1;
  unsigned short  ss1, __ss1h;
  unsigned long   esp2;
  unsigned short  ss2, __ss2h;
  unsigned long   cr3, eip, eflags;
  unsigned long   eax, ecx, edx, ebx, esp, ebp, esi, edi;

  unsigned short  es, __esh;
  unsigned short  cs, __csh;
  unsigned short  ss, __ssh;
  unsigned short  ds, __dsh;
  unsigned short  fs, __fsh;
  unsigned short  gs, __gsh;
  unsigned short  ldt, __ldth;
  unsigned short  trace, iomapbase;
} tss_386_t __attribute__ ((aligned (16)));
  


/* Trap and Int gate type */
typedef struct gate_s
{
  short int       offset_low;
  short int       selector;
  unsigned char   pad;           /* 000xxxxx, int/trap */
  unsigned char   p_dpl_type;
  short int       offset_high;
} gate_t;


/* Segment descriptor type */
typedef struct seg_descriptor_s
{
  short int       limit_low;
  short int       base_low;
  unsigned char   base_mid;
  unsigned char   p_dpl_type;
  unsigned char   g_limit_high;
  unsigned char   base_high;
} seg_descriptor_t;

/* tss descriptor type */
typedef seg_descriptor_t   tss_descriptor_t;



/*
 * Loader arguments pased to kernel on start
 *  THE SAME FORMAT HAS TO BE USED IN LOADER32, OFCOURSE! 
 *
 */

/* Loader arguments are put here from loader32 */
#define LOADER_ARGUMENTS_ADDR     0x80000
#define LOADER_ARGUMENTS_FNLEN    100


/** Loaded modules list elements (Kernel module IS included in this list) */
typedef struct loaded_module_s
{
  /* Next in list */
  struct loaded_module_s   *next_module;
  
  /* Load adress and length */
  unsigned int              load_address;
  unsigned int              loaded_pages;

  /* init function pointer, void init (void) */
  void                    (*init)(void); 

  /* filename, truncated to 100 chars */
  char                      filename[LOADER_ARGUMENTS_FNLEN + 1];

  /* Linked list of the page frames allocated on load of this module */
  page_frame_t             *page_frames;

} loaded_module_t;


/** Arguments head */
typedef struct 
{
  /* detected memory size */
  unsigned int       mem_size_bytes;

  /* size of arguments list */
  unsigned int       arguments_size;

  /* ROM font info */
  char              *font_data;
  int                font_width;
  int                font_height;
  
  /* Cursor pos from loader */
  int                cursor_x;
  int                cursor_y;

  /* head of list with loaded modules */ 
  loaded_module_t   *loaded_modules;
} loader_args_t;



/*
 * The process type
 */

typedef struct process_s
{
  /** Processor state */
  tss_386_t     tss; 

  /** Process ID */
  int           pid;

  /** Process state */
  short         state;
  
  /** Process memory areas */
  vm_block_t    vm_code;
  vm_block_t    vm_data;
  vm_block_t    vm_stack;
  vm_block_t    vm_kernel_stack;


  /** Process userid and groupid */
  int           uid;
  int           gid;

  /** Umask, used with open() */
  int           umask;

  /** Pointers for use in scheduler */
  struct process_s  *next;
  struct process_s  *prev;

  /** Process priority (0 - low) */
  int           priority;

  /** File descriptors and other shit goes here too */

} process_t;


#endif 








