/*
 * Kernel assert debug tool
 *
 * (c) Jimmy Larsson 1999
 *
 */

#include "assert.h"
#include "util.h"
#include "printk.h"

/**
 * To have asserts compile with -DASSERT
 *
 */

#ifdef ASSERT

void assert_failed (char *exp, char *file, char *baseFile, int line)
{
    if (!string_cmp(file, baseFile)) 
    {
      printf ("assert(%s) failed in file %s, line %d\n", exp, file, line);
    } else 
    {
      printf ("assert(%s) failed in file %s (included from %s), line %d\n", 
	     exp, 
	     file, 
	     baseFile, 
	     line);
    }
}

#endif


