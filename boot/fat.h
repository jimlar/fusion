/*
 * FAT Utilities for Loader32 the Fusion loader
 *
 * (c) Jimmy Larsson 1999
 *
 */

#ifndef FAT_H
#define FAT_H

/* FAT boot block indices */
#define BB_SECT_PER_CLUSTER   0xd
#define BB_RESERVED_SECT_LO   0xe
#define BB_RESERVED_SECT_HI   0xf
#define BB_NO_OF_FATS         0x10
#define BB_ROOTENTRIES_LO     0x11
#define BB_ROOTENTRIES_HI     0x12
#define BB_SECT_PER_FAT_LO    0x16
#define BB_SECT_PER_FAT_HI    0x17

/* fat filename len */
#define FAT_FILENAME_LEN      8
#define FAT_EXTENSION_LEN     3

#define DIR_ENTRIES_PER_SECT  16

/* Structure of the FAT bootblock */

typedef struct 
{
  char             ignore[0xd]; /* Ignored shit */
  unsigned char    sectors_per_cluster;
  unsigned short   reserved_sectors;
  unsigned char    no_of_fats;
  unsigned short   root_entries;
  char             ignore2[3];
  unsigned short   sectors_per_fat;

  char             code[512 - 24];
} fat_boot_block;
   
/* Structure of the FAT directory entries */

typedef struct
{
  char             filename[11];
  unsigned char    attributes;
  char             ignore[10];
  unsigned short   time;
  unsigned short   date;
  unsigned short   start_cluster;
  unsigned long    size;
} fat_dir_t;


/* Structure of a Partitiontable Entry */

typedef struct
{
  unsigned char    bootflag;
  unsigned char    beginhead;
  unsigned char    beginsector;
  unsigned char    begincyl;
  unsigned char    system_id;
  unsigned char    endhead;
  unsigned char    endsector;
  unsigned char    endcyl;
  unsigned short   rel_sector_low;
  unsigned short   rel_sector_high;
  unsigned short   num_sectors_low;
  unsigned short   num_sectors_high;
} partition_table_entry;


/* 
 * initialize fat stuff
 *
 */

void init_fat (int boot_drive);



/*
 * Read file data into buf
 *
 *  len   - read length
 *  start - startpos
 *  buf   - where to copy data
 *
 */

int fat_fread (void   *buf,
	       int     start,
	       int     len, 
	       char   *filename,
	       char    print_progress);

#endif
