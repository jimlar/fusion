/*
 * FAT Utilities for Loader32 the Fusion loader
 *
 * (c) Jimmy Larsson 1999
 *
 */

#include "fat.h"
#include "loader32.h"
#include "bios_read.h"
#include "printk.h"

/* values set by inti_fat() */
static fat_boot_block    bb;
static unsigned short   *fat_ptr;
static unsigned char     bb_raw[SECTOR_SIZE];
static unsigned char     sector_buffer[SECTOR_SIZE];


/*
 * fatname_cmp
 * Compare string as if they were FAT filenames
 *
 */

int fatname_cmp (char   *name1,
		 char   *name2)
{
  int    i;


  for (i = 0; i < 11; i++)
  {
    if (name1[i] != name2[i])
      return 0;
  }

  return 1;
}

/*
 * fat_load
 * Follow FAT-chain and load file into given address
 *
 */

unsigned long fat_load (unsigned short   *fat_ptr,
			unsigned short    start_cluster,
			unsigned long     load_addr,
			int               file_start,
			int               load_bytes,
			char              print_progress)
{
  unsigned long    sector = 0;
  unsigned short   cluster = start_cluster;
  int              read_status;
  long             total_read = 0;
  unsigned char   *tmp_ptr;
  int              i, offset;
  int              num_loaded = 0;
  int              first_cluster = 1;
  int              progress_dummy = 0;
 
  while (cluster > 1 && cluster < 0xfff0)
  {
    /* Start of data area */
    sector = bb.reserved_sectors + bb.sectors_per_fat * bb.no_of_fats + bb.root_entries / DIR_ENTRIES_PER_SECT;
    
    /* Get the actual sector of this cluster */
    sector = sector + (cluster - 2) * bb.sectors_per_cluster;
    
    /* Read this cluster */
    read_status = read_sectors (sector, bb.sectors_per_cluster, (unsigned long) &sector_buffer[0]);
    if (read_status < 0)
      return -1;
    
    /* If this cluster is inside requested buffer area, copy it */
    if (total_read + SECTOR_SIZE >= file_start && load_bytes != -1)
    {
      tmp_ptr = (unsigned char *) load_addr;

      if (first_cluster)
      {
	offset = file_start % SECTOR_SIZE;
	first_cluster = 0;
      } else
      {
	offset = 0;
      }

      i = 0;

      while (load_bytes > 0 && (i + offset) < SECTOR_SIZE)
      {
	tmp_ptr[i] = sector_buffer[i + offset];
	i++;
	load_bytes--;
	num_loaded++;
      }

      if (load_bytes == 0)
	return num_loaded;

      load_addr += i;

    } else if (load_bytes == -1)
    {
      /* Copy all */
      tmp_ptr = (unsigned char *) load_addr;
      for (i = 0; i < SECTOR_SIZE; i++)
      {
	tmp_ptr[i] = sector_buffer[i];
	num_loaded++;
      }      
      load_addr += i;
    }

    /* prepare for next cluster */
    total_read += read_status * SECTOR_SIZE;
    cluster = fat_ptr[cluster];

    /* Print progress */
    if (print_progress) {
      
      if (progress_dummy % 2)
	printf ("%c", 0xdb);

      progress_dummy++;
    }
  }

  return num_loaded;
}

/*
 * fat_bootblock_decode
 * extract data from raw boot block 
 *
 */

void fat_bootblock_decode (unsigned char    *raw, 
			   fat_boot_block   *bb)
{
  bb->sectors_per_cluster = raw[BB_SECT_PER_CLUSTER];
  bb->reserved_sectors = raw[BB_RESERVED_SECT_LO];  
  bb->reserved_sectors += raw[BB_RESERVED_SECT_HI] << 8;
  bb->no_of_fats = raw[BB_NO_OF_FATS];
  bb->root_entries = raw[BB_ROOTENTRIES_LO];
  bb->root_entries += raw[BB_ROOTENTRIES_HI] << 8;
  bb->sectors_per_fat = raw[BB_SECT_PER_FAT_LO];
  bb->sectors_per_fat += raw[BB_SECT_PER_FAT_HI] << 8;
}


/*
 * copy fat filename into loader args structure
 *
 */

void strcpy_fat (char   *args_fn,
		 char   *fat_fn)
{
  int len_fn = FAT_FILENAME_LEN;
  int len_ext = FAT_EXTENSION_LEN;
  int i, j, ext_start;

  /* plain copy */
  for (i = 0; i < len_fn; i++)
  {
    args_fn[i] = fat_fn[i];
  }

  j = i;

  /* remove trailing spaces on filename */
  while (args_fn[--i] == ' ');
  
  i++;
  ext_start = i;
  args_fn[i++] = '.';


  /* plain copy of extension */
  for (; j < len_fn + len_ext; j++)
  {
    args_fn[i++] = fat_fn[j];
  }
  
  /* remove trailing spaces on extension */
  while (args_fn[--i] == ' ');
  
  if (i != ext_start) /* remove dot if only space in ext */
    i++;

  /* end string */
  args_fn[i] = '\0';

}

/* 
 * initialize fat stuff
 *
 */

void init_fat (int boot_drive)
{
  long              fat_lbn, dir_lbn;
  int               read_status;
  int               root_dir_len;

  /* 1. Read bootblock */
  read_status = read_sectors (0, 1, (long) &bb_raw[0]);
  if (read_status < 0)
    panic ("L32 ERROR: can't read boot block!!");

  /* Decode fat boot block, since a simple struct will not do */
  fat_bootblock_decode (&bb_raw[0], &bb);
 
  /* 2. Read directory */
  dir_lbn = bb.reserved_sectors + bb.no_of_fats * bb.sectors_per_fat;  
  root_dir_len = bb.root_entries / DIR_ENTRIES_PER_SECT;
  read_status = read_sectors (dir_lbn, root_dir_len, DIR_LOAD_ADDR);
  if (read_status < 0)
    panic ("L32 ERROR: can't read root directory!!");

  /* 3. Read FAT into memory (directly after directory) */
  fat_lbn = bb.reserved_sectors;
  fat_ptr = (unsigned short *) (long) (DIR_LOAD_ADDR + SECTOR_SIZE * read_status);
  read_status = read_sectors (fat_lbn, bb.sectors_per_fat, DIR_LOAD_ADDR + SECTOR_SIZE * read_status);
  if (read_status < 0)
    panic ("L32 ERROR: can't read FAT!!");

}

/*
 * Make fat disk name from std 8+3 filename
 *
 */

void fatify_filename (char *dest, char *src)
{
  int i,j;

  for (i = 0; i < (FAT_FILENAME_LEN + FAT_EXTENSION_LEN); i++)
    dest[i] = ' ';

  for (i = 0; i < FAT_FILENAME_LEN; i++)
  {
    if (src[i] == '.')
    {
      i++;
      
      for (j = 0; j < FAT_EXTENSION_LEN; j++)
      {
	if (src[i] == '\0')
	  break;
	
	if (src[i] >= 'a' && src[i] <= 'z')
	  dest[FAT_FILENAME_LEN + j] = src[i + j] - ('a' - 'A');
	else
	  dest[FAT_FILENAME_LEN + j] = src[i + j];
      }

      break;
    }

    if (src[i] == '\0')
      break;

    if (src[i] >= 'a' && src[i] <= 'z')
      dest[i] = src[i] - ('a' - 'A');
    else
      dest[i] = src[i];
  }
}



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
	       char    print_progress)
{
  fat_dir_t        *dir_ptr;
  char fat_filename[FAT_FILENAME_LEN + FAT_EXTENSION_LEN + 1];
  int i;
  int load_status;

  fatify_filename (fat_filename, filename);
  fat_filename[FAT_FILENAME_LEN + FAT_EXTENSION_LEN] = '\0';


  dir_ptr = (fat_dir_t *) DIR_LOAD_ADDR;

  for (i = 0; i < bb.root_entries; i++)
  {
    if (fatname_cmp (dir_ptr->filename, fat_filename))
    {
     
      load_status = fat_load (fat_ptr, dir_ptr->start_cluster, (unsigned long) buf, start, len, print_progress);
      return load_status;
    }

    dir_ptr++;
  }

  return -1;
}
