/*
 * Loader for FUSION
 * This is the rewritten 32bit version in C
 * I had to do this since the assembler version was
 * starting to get out of hand, with this version I can lift
 * the loading to a more useable level with dynamic configuration etc.
 *
 * Uses a jump to 16-bit and bios calls to actually load stuff
 *
 * (Most of this code is totally dependant on FAT16 being used)
 *
 * (c) 1998-1999 Jimmy Larsson
 *
 * NEED TO FIX:
 *        - Harddisk support 
 *        - Fat12 support?  (maybe native FS instead)
 *
 * 
 *
 */

#include "loader32.h"
#include "printk.h"
#include "../kernel/types.h"
#include "../kernel/elf.h"
#include "fat.h"
#include "bios_read.h"
#include "disk_io.h"
#include "vga.h"
#include "pcx.h"

static int          boot_drive;
static Elf32_Phdr   program_header_table[MAX_PHT_ENTRIES];

#define FDC_DIGITAL_OUT_PORT  0x3f2


/* Entry point of loader, has to be first in binary */

void start_loader (void)
{ 
  __asm__ __volatile__ ("
            cli

  __5:	    in	 $0x64, %al    /* Enable A20 */
	    test $2, %al
	    jnz	 __5
	    mov	 $0xd1, %al
	    out	 %al, $0x64
  __6:	    in	 $0x64, %al
	    test $2, %al
	    jnz	 __6
	    mov	 $0xdf, %al
	    out	 %al, $0x60

            cs                /* Load segments */ 
            lgdt gdt

            mov	 $0x10, %ax
	    mov	 %ax,   %ds
	    mov	 %ax,   %es
            mov  %ax,   %fs
	    mov	 %ax,   %gs
	    mov	 %ax,   %es
	    mov	 %ax,   %ss
            jmp  start_loader_done


/* Global Descriptor Table */

gdt:
        .word	0x28		/* gdt limit */
        .word	null_seg,0x0    /* gdt base */

null_seg:							
        .word	0,0,0,0		/* NULL segment */

flat_code32_seg:
        .word	0xFFFF		/* 4Gb - limit */
        .word	0x0000		/* base address=0 */
        .word	0x9A00		/* code read/exec */
        .word	0x00CF		/* granularity=4096, 386 */

flat_data32_seg:
        .word	0xFFFF		/* 4Gb - limit */
        .word	0x0000		/* base address=0 */
        .word	0x9200		/* data read/write */
        .word	0x00CF		/* granularity=4096, 386 */

flat_code16_seg:
        .word	0xFFFF		/* 4Gb - limit */
        .word	0x0000		/* base address=0 */
        .word	0x9A00		/* code read/exec */
        .word	0x008F		/* granularity=4096, 16-bit */

flat_data16_seg:
        .word	0xFFFF		/* 4Gb - limit */
        .word	0x0000		/* base address=0 */
        .word	0x9200		/* data read/write */
        .word	0x008F		/* granularity=4096, 16-bit */


start_loader_done:

");
 	
  main ();
  while (1);
}


/*
 * detect_mem
 *
 * Direct probe memory above 1MB
 *
 * Returns found mem in bytes
 */

int detect_mem (void)
{
  unsigned int    *ptr;
  unsigned int     a, mem_bytes;
  unsigned short   mem_mb;
  unsigned char    irq1, irq2;
  unsigned int     cr0;

  /* save IRQ's */
  irq1 = inb (0x21);
  irq2 = inb (0xA1);
  
  /* kill all irq's */
  outb (0xff, 0x21);
  outb (0xff, 0xA1);
  
  mem_mb = 0;

  
  /* store a copy of CR0 */
  __asm__ __volatile ("movl %%cr0, %%eax"
		      :"=a"(cr0)
		      : :"eax");

  /*
   * invalidate the cache
   * write-back and invalidate the cache
   */  
  __asm__ __volatile__ ("wbinvd");

  /*
   * plug cr0 with just PE/CD/NW
   * cache disable(486+), no-writeback(486+), 32bit mode(386+)
   */
  __asm__ __volatile__ ("movl %%eax, %%cr0" :
			: "a" (cr0 | 0x00000001 | 0x40000000 | 0x20000000) 
			: "eax");
  
  do
  {
    mem_mb++;

    /* 
     * Test last 4 bytes of this megabyte 
     * testing the first bytes seemed ureliable on some machines
     **/
    ptr = (unsigned int *) ((mem_mb + 1) * 1024 * 1024 - 4);

    a = *ptr;
    
    *ptr = 0x55AA55AA;
    
    /*
     * the empty asm calls tell gcc not to rely on whats in its registers
     * as saved variables (this gets us around GCC optimisations)
     */   
    asm("":::"memory");

    if(*ptr != 0x55AA55AA)
      break;
    else
    {
      *ptr = 0xAA55AA55;

      asm("":::"memory");

      if(*ptr != 0xAA55AA55)
	break;
    }    
   
    asm("":::"memory");
    *ptr = a;
  
  } while (mem_mb < 4096);
  
  __asm__ __volatile__("movl %%eax, %%cr0" 
		       :: "a" (cr0) 
		       : "eax");


  mem_bytes = mem_mb * 1024 * 1024;

  outb(irq1, 0x21);
  outb(irq2, 0xA1);

  //printf ("L32: detected %u kB memory\n", mem_bytes / 1024);
  return mem_bytes;
}


/*
 * panic
 * Print message and halt
 *
 */

void panic (char *mess)
{
  printf ("%s, L32 Halted!!", mess);
  while (1);
}

/*
 * find_next_page
 * returns address of next page start
 *
 */

unsigned long find_next_page (unsigned long   address)
{
  return address + (PAGE_SIZE - (address % PAGE_SIZE));
}

/*
 * simple string copy 
 *
 */

void simple_strcpy (char *dest, char *src)
{
  int i = 0;

  while (src[i] != '\0')
  {
    dest[i] = src[i];
    i++;
  }

  dest[i] = '\0';
}


/*
 * elf_read
 * Load the main kernel file as an elf program
 *
 * return bytes of memory needed by this program (not same as file bytes)
 *
 */

int elf_read (void   *load_addr, FILE  *elf_file)
{
  Elf32_Ehdr       elf_header;
  int              pht_entries;
  int              pht_size;
  int              i,j;
  void            *load_ptr;
  int              load_min_addr;
  int              load_max_addr;
  unsigned char   *tmp_ptr;


  if (fread ((void *) &elf_header, sizeof(elf_header), elf_file, 1) != sizeof(elf_header))
    panic ("elf load error!");

  if (elf_header.e_ident[EI_MAG0] != ELFMAG0 ||
      elf_header.e_ident[EI_MAG1] != ELFMAG1 ||  
      elf_header.e_ident[EI_MAG2] != ELFMAG2 ||  
      elf_header.e_ident[EI_MAG3] != ELFMAG3)
  {
    panic ("BAD ELF Magic numbers!");
  }

  if (elf_header.e_type != ET_EXEC)
    panic ("BAD ELF File! (non executable)");

  if (elf_header.e_machine != EM_386)
    panic ("BAD ELF File! (non i386)");

  if (elf_header.e_version != EV_CURRENT)
    panic ("BAD ELF File (invalid version)!");

  /*
   * Maybe we'll allow this to be dynamic later, don't know...
   */

  if (elf_header.e_entry != SYSTEM_LOAD_ADDR) 
    panic ("BAD ELF File (entry point not correct)!");
  
  pht_entries = elf_header.e_phnum;
  pht_size = pht_entries * elf_header.e_phentsize; 

  if (pht_entries > MAX_PHT_ENTRIES)
    panic ("ELF: too many program header table entries!");

  /* Read program header table */
  fseek (elf_file, elf_header.e_phoff);

  if (fread ((void *) &program_header_table[0], pht_size, elf_file, 1) != pht_size)
    panic ("ELF: PHT load error!");

  load_min_addr = 0x7fffffff;
  load_max_addr = 0;

  for (i = 0; i < pht_entries; i++)
  {
    fseek (elf_file, program_header_table[i].p_offset);
    load_ptr = (void *) program_header_table[i].p_vaddr;
    
    load_min_addr = MIN(load_min_addr, (int) load_ptr);
    load_max_addr = MAX(load_max_addr, (int) load_ptr + program_header_table[i].p_memsz);

    /** If memory size is larger than file size, the rest should be zero filled */
    if (program_header_table[i].p_filesz < program_header_table[i].p_memsz)
    {
      tmp_ptr = (unsigned char *) (program_header_table[i].p_vaddr + program_header_table[i].p_filesz);
      for (j = 0; j < (program_header_table[i].p_memsz - program_header_table[i].p_filesz); j++)
	tmp_ptr[j] = 0;
    } 

    if (fread (load_ptr, program_header_table[i].p_filesz, elf_file, 1) != program_header_table[i].p_filesz)
      panic ("ELF: PHT entry load error!");

  }

  return load_max_addr - load_min_addr;
}


/*
 * main - Entry point for stage two 
 *
 * Assumed at this point:
 *       - A stack has been set up 
 *       - The segments and GDT have been set up with a flat model 
 *       - The BIOS interrups have not been moved
 *       - We are in protected mode
 */

void main (void)
{
  int               j;
 
  int               num_loaded = 0;
  unsigned long     load_status;
  FILE             *cur_file;


  /* First file in list is assumed kernel */
  unsigned long     file_load_addr = SYSTEM_LOAD_ADDR;   

  char             *files_to_load[] = {"SYSTEM", "INIT.SYS", "\0"};

  /* Argument list, passed to kernel */
  loader_args_t     *loader_args = (loader_args_t *) LOADER_ARGUMENTS_ADDR;
  loaded_module_t   *loaded_module = (loaded_module_t *)(LOADER_ARGUMENTS_ADDR 
							 + sizeof (loader_args_t));
  /* module pointers */
  loaded_module_t   *cur_module = loaded_module;
  loaded_module_t   *prev_module = NULL;

  vga_set_mode (0x12);
  vga_clear_screen();

  /** initalize arguments */
  loader_args->mem_size_bytes = detect_mem();

  /** at least the kernel has to be loaded, so this is safe */
  loader_args->loaded_modules = loaded_module;


  /* Get boot_drive from the bootblock */
  boot_drive = (int) *((char *) BIOS_LOAD_DRIVE);

  /*
   * If were gonna support harddisks we
   * have to read partition table here and determine
   * which partition to boot from and then
   * read the bootblock from that partition
   *
   */


  init_fs (boot_drive);

  /*
   * Read and show boot logo
   *
   */
  //cur_file = fopen (BOOT_LOGO_FILE);
  //load_status = fread ((void *) BOOT_LOGO_LOAD_ADDR, -1, cur_file, 0);
  //pcx_draw_image ((void *) BOOT_LOGO_LOAD_ADDR, (void *) BOOT_LOGO_DECODE_ADDR, 0, 0);

  printf ("Loader32\n");
  printf ("---------------------------------------\n");

  /*
   * now for the real work
   * Load the OS
   *
   */ 

  cur_file = NULL;
 
  for (j = 0; files_to_load[j][0] != '\0'; j++)
  {
    printf ("\n%s: ", files_to_load[j]);

    if (cur_file != NULL)
      fclose(cur_file);

    cur_file = fopen (files_to_load[j]);
    if (cur_file == NULL)
      panic ("can't open file!!");

    if (j == 0)
    {
      //printf ("L32: Loading Kernel binary (%s) as ELF program...\n", files_to_load[j]); 
      load_status = elf_read ((void *) file_load_addr, cur_file);
    } else
    {
      load_status = fread ((void *) file_load_addr, -1, cur_file, 1);
    }

    if (load_status < 0)
      panic ("can't load file!!");

    //printf ("L32: loaded %s into 0x%x, %d bytes allocated\n",
    //	    files_to_load[j],
    //	    (int) file_load_addr,
    //	    (int) load_status);

    /* Module loaded Ok, fill in arguments */
    cur_module->next_module = NULL;
    cur_module->load_address = file_load_addr;
    cur_module->init = NULL;
    cur_module->page_frames = NULL;

    /* link us into list */
    if (prev_module != NULL)
      prev_module->next_module = cur_module;
 
    /* copy filename into args */
    simple_strcpy (cur_module->filename, files_to_load[j]);
  
    file_load_addr = find_next_page (file_load_addr + load_status);
    
    /* fill in rest of args (loaded pages) */
    cur_module->loaded_pages = (file_load_addr - cur_module->load_address) / PAGE_SIZE;

    prev_module = cur_module;
    cur_module = &prev_module[1];
    
    num_loaded++;

  }

  fclose (cur_file);
  printf ("\n");

  /* 5. Set len of args */
  loader_args->arguments_size = sizeof (loaded_module_t) * num_loaded + sizeof (loader_args_t);
  //printf ("L32: loaded %u modules\n", num_loaded);

  /* 6. Turn off Disk motor */
  outb (0, FDC_DIGITAL_OUT_PORT);

  /* 7. Jump to OS */
  //printf ("L32: done, Kernel jumping to 0x%x\n", SYSTEM_LOAD_ADDR);

  /* Set vga state in args */
  loader_args->cursor_x = get_cursor_x();
  loader_args->cursor_y = get_cursor_y();
  loader_args->font_width = get_font_width();
  loader_args->font_height = get_font_height();
  loader_args->font_data = get_font_data();

  __asm__("pushl %%eax  
           ljmp $8,$0x100000"
	  :                     /* no output regs      */
	  : "a" (loader_args)); /* put args ptr in eax */

}







