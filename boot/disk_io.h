/*
 * Disk and FS abstraction layer for loader32
 *
 * (c) Jimmy Larsson 1999
 *
 */

#ifndef DISK_IO_H
#define DISK_IO_H

#include "loader32.h"

/* max open files */
#define MAX_OPEN_FILES   10


typedef struct
{
  int     filepos;
  char   *filename;
} FILE;


/*
 * Initialize
 *
 */

void init_fs (int boot_drive);


/*
 * Read data from a file into buffer
 *  returns bytes read
 *
 * (a size of -1 means read whole file)
 * 
 */

int fread (void   *buf, int   size, FILE   *file, char show_progress);

/*
 * 
 * Seek into file (pos is from the beginning)
 *
 *
 */

void fseek (FILE   *file, int   pos);


/*
 * 
 * Open a file
 *
 *
 */

FILE *fopen (char   *filename);


/*
 * Close file
 *
 */

void fclose (FILE   *file);


#endif
