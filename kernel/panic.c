/*
 * panic.c
 * (c) Jimmy Larsson 1998
 * Die with an error message
 *
 */

#include "panic.h"
#include "printk.h"


void panic (char   *msg)
{
  printf ("\nPANIC: %s\n", msg);

  while (1);
}


void warning (char   *msg)
{
  printf ("\nWARNING: %s\n", msg);

}


