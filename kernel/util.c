/*
 * Kernel utilities
 * (c) Jimmy LArsson 1999
 *
 */

#include "util.h"



/*
 * small string compare
 *
 */ 

int string_cmp (char   *str1, 
	    char   *str2)
{
  int i = 0;

  while (str1[i] != '\0')
  {
    if (str1[i] != str2[i])
      return 1;            /* not same string */

    i++;
  }

  /* Check that both strings are ended */
  if (str1[i] == '\0' && str2[i] == '\0')
    return 0;

  return 1;
}
 
/* 
 * get CR3 utility 
 *
 */
long get_cr3 (void)
{
  unsigned long cr3;
  
  asm ("mov %%cr3, %0" : "=r" (cr3));
  return cr3;
}

/* 
 * get CR2 utility 
 *
 */
long get_cr2 (void)
{
  unsigned long cr2;
  
  asm ("mov %%cr2, %0" : "=r" (cr2));
  return cr2;
}
           
