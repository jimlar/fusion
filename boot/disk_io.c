/*
 * Disk and FS abstraction layer for loader32
 *
 * (c) Jimmy Larsson 1999
 *
 */

#include "disk_io.h"
#include "fat.h"
#include "bios_read.h"


static FILE filehandles[MAX_OPEN_FILES];

/*
 * Initialize
 *
 */

void init_fs (int boot_drive)
{
  int i;

  init_bios_read (boot_drive);
  init_fat (boot_drive);

  /* Free filehandles */
  for (i = 0; i < MAX_OPEN_FILES; i++)
  {
    filehandles[i].filepos = -1;
  }  
}


/*
 * Read data from a file into buffer
 *  returns bytes read
 *
 * (a size of -1 means read whole file)
 * 
 */

int fread (void   *buf, int   bytes, FILE   *file,  char show_progress)
{
  int read_status;

  read_status = fat_fread (buf, file->filepos, bytes, file->filename, show_progress);
 
  if (read_status != -1)
    file->filepos = file->filepos + read_status;

  return read_status;
}

/*
 * 
 * Seek into file (pos is from the beginning)
 *
 *
 */

void fseek (FILE   *file, int   pos)
{
  file->filepos = pos;
}

/*
 * 
 * Open a file
 *
 *
 */

FILE *fopen (char   *filename)
{
  int i;

  /* Find free filehandle */
  for (i = 0; i < MAX_OPEN_FILES; i++)
  {
    if (filehandles[i].filepos == -1)
    {
      filehandles[i].filename = filename;
      filehandles[i].filepos = 0;
      return &filehandles[i];
    }
  } 

  return NULL;
}


/*
 * Close file
 *
 */

void fclose (FILE   *file)
{
  if (file != NULL)
    file->filepos = -1;
}
