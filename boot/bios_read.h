/*
 * Protected mode BIOS disk read routines
 * for Loader32 - the Fusion loader
 *
 * (c) 1999 Jimmy Larsson
 *
 */

#ifndef BIOS_READ_H
#define BIOS_READ_H


#define SECTORS_1_44          18
#define HEADS_1_44            2
#define SAFE_MAX_TRANSFER     BIOS_MAX_LEN - 8
#define SECTOR_SIZE           512

#define BIOS_MAX_HEAD      15
#define BIOS_MAX_CYL       1023
#define BIOS_MAX_SECT      63
#define BIOS_MAX_LEN       128

#define BIOS_HDD0          0x80
#define BIOS_DISK_RETRIES  5


/* The structure of the BIOS harddisk parameters */

typedef struct
{
  unsigned short   cylinders;
  unsigned char    heads;
  char             obsolete[5];
  unsigned char    control;
  char             obsolete2[5];
  unsigned char    sectors;
  char             pad;
} bios_disk_param;



/*
 * Read Sectors
 * read sectors from disk using a bios call, with retries on bios errors
 *
 * (returns sectors read or negative if error)
 */

int read_sectors (unsigned long   lbn, 
		  unsigned int    sectors, 
                  unsigned long   address);




/*
 * initialize
 */

void init_bios_read (int boot_disk);

#endif
