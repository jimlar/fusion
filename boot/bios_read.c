/*
 * Protected mode BIOS disk read routines
 * for Loader32 - the Fusion loader
 *
 * (c) 1999 Jimmy Larsson
 *
 */

#include "bios_read.h"
#include "loader32.h"
#include "printk.h"

/* The boot disk */
static int bios_boot_disk = 0;

/*
 * BIOS Read sectors
 * Simple glue to 16-bit bios call
 * - returns sectors read or, if negative, an bios error code
 *
 * Assumes:
 *       - We are in protected mode with correct GDT
 *       - We have a good stack (well you probably already have that if you get here... =)
 *       - The BIOS Int. vectors have not been exterminated yet
 *
 */

int bios_read_sectors (unsigned int    cylinder,
		       unsigned int    sector,
		       unsigned int    head,
		       unsigned int    len,
		       unsigned long   address)
{
  unsigned long    eax_reg;
  unsigned long    ebx_reg;
  unsigned long    ecx_reg;
  unsigned long    edx_reg;
  unsigned long    bios_return_code;
  unsigned long    es_reg;
  char            *from_buf;
  char            *to_buf;
  long             i;


  /* Check all parameters */
  if (len > BIOS_MAX_LEN)
    return -1;
  if (cylinder > BIOS_MAX_CYL)
    return -1;
  if (sector > BIOS_MAX_SECT)
    return -1;
  if (head > BIOS_MAX_HEAD)
    return -1;

  /* Dest. addresses (for temporary dest.) */
  es_reg = LOAD_BUFFER_ADDR >> 4;
  ebx_reg = 0;

  eax_reg = 512 + len;

  /*   
   *   CX = | c7 c6 c5 c4 c3 c2 c1 c0 | c9 c8 s5 s4 s3 s2 s1 s0 |
   *
   *   c = cylinder bit, s = sector bit
   * 
   */  

  ecx_reg = (cylinder & 0xff) << 8;
  ecx_reg = ecx_reg | sector | (cylinder & 0x300) >> 2;

  edx_reg = head << 8;
  edx_reg = edx_reg + bios_boot_disk;

  /* EBX = 0 assumed in this code */

  __asm__("
           mov   $0x20, %%di    /* Load 16-bit descriptors */
           mov   %%di,  %%ds
           mov   %%di,  %%ss

           ljmp  $0x18, $flushpm16 /* Set 16-bit CS */
flushpm16:

           mov   %%cr0, %%edi   /* Go Real-Mode */
           dec   %%edi
           mov   %%edi, %%cr0

.code16
           nop                  /* Fix 386 race condition */
 
           ljmp  $0,$flushrm    /* Set CS for real mode operation */
flushrm:
        
           mov   $0, %%di       /* Load real-mode segments */
           mov   %%di, %%ds
           mov   %%di, %%ss

           mov   %%bx,  %%es    /* Set ES (transfer-to segment) */
           xor   %%ebx, %%ebx   

           int   $0x13 

           cli                  /* Some BIOSes enable interrupts */

           jnc   no_disk_error  /* successful? */

           /* no success, reset disk system */

           push  %%eax          /* Save error code */
           xor   %%ah, %%ah       
           int   $0x13          /* reset it, depend on DL, from last int */
           cli
           pop   %%eax

no_disk_error:
           mov   %%cr0, %%ecx   /* Back to Protected-Mode */
           inc   %%ecx
           mov   %%ecx, %%cr0

.code32
           .byte 0x66, 0xea     /* Set CS for protected mode operation */
           .long flushpm        /* Hardcoded instruction, taken from   */
           .word 0x8            /* Intel's programming ref.            */
flushpm:

           mov	 $0x10, %%bx
	   mov	 %%bx,  %%ds
	   mov	 %%bx,  %%es
           mov   %%bx,  %%fs
	   mov	 %%bx,  %%gs
	   mov	 %%bx,  %%ss
                    

        " : "=eax" (bios_return_code)                                           /* Output    */
	  : "eax" (eax_reg), "ebx" (es_reg), "ecx" (ecx_reg), "edx" (edx_reg)   /* Input     */
          : "eax", "ebx", "ecx", "edx");                                        /* Clobbered */


  if (bios_return_code & 0xff00)           /* Error ? */
  {
    printf ("L32: BIOS sector read failed!! BIOS returned status 0x%x\n     C/H/S/N = %u/%u/%u/%u\n",
	    (int) ((bios_return_code & 0xffff) >> 8),
	    cylinder,
	    head, 
	    sector,
	    len);

    return -((bios_return_code & 0xff00) >> 8);     /* Yes! */
  }

  /* Loading ok... move data to correct destination */
  from_buf = (char *) LOAD_BUFFER_ADDR;
  to_buf = (char *) address;

  for (i = 0; i <= (len * SECTOR_SIZE); i++)
  {
    to_buf[i] = from_buf[i];
  }

  return bios_return_code & 0xff;

}


/*
 * Read Sectors
 * read sectors from disk using a bios call, with retries on bios errors
 *
 * (returns sectors read or negative if error)
 */

int read_sectors (unsigned long   lbn, 
		  unsigned int    sectors, 
                  unsigned long   address)
{
  int             sector;
  int             head;
  int             cylinder;
  int             retry_count;
  int             total_read = 0;
  unsigned long   cur_addr = address;
  unsigned long   cur_lbn = lbn;
  int             read_now = sectors;
  int             really_read;


  /* This is only valid for 1.44MB floppy right now */

  while (sectors > total_read)
  {
   
    /* Translate LBN to disk geometry */
    cylinder = (cur_lbn / SECTORS_1_44) / HEADS_1_44;
    sector = (cur_lbn % SECTORS_1_44) + 1;
    head = (cur_lbn / SECTORS_1_44) % HEADS_1_44;

    /* limit len to not cross DMA boundary */
    if (read_now > SAFE_MAX_TRANSFER)
      read_now = SAFE_MAX_TRANSFER;

    if (bios_boot_disk < 0x80)
    {
      /* If floppy, we can't cross sectors per side boundaries */
      
      if ((read_now + sector - 1) > SECTORS_1_44)
	read_now = SECTORS_1_44 - MIN(sector-1,SECTORS_1_44);

    }

    retry_count = 0;
    really_read = -1;

    /* BIOS reads shuold always be retried at least 3 times (sigh!) */
    while (really_read < 0 && retry_count < BIOS_DISK_RETRIES)
    {
      really_read = bios_read_sectors (cylinder, sector, head, read_now, cur_addr);
      retry_count++;
    }

    if (really_read < 0)
      return -1;
    
    /* Update counters and addresses */
    total_read = total_read + really_read;
    cur_lbn = cur_lbn - really_read;
    cur_addr = cur_addr - SECTOR_SIZE * really_read;
    read_now = sectors - total_read;

  }

  return total_read;

}


/*
 * initialize
 */

void init_bios_read (int boot_disk)
{
  bios_boot_disk = boot_disk;
}
