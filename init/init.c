/*
 *
 * INIT for fusion
 * This is the first process of the system,
 * the rest of the processes are created with fork calls
 * (much like posix systems)
 *
 *
 * (c) Jimmy Larsson 1999
 *
 */

#include <stdio.h>
#include <unistd.h>

/* Set this to 1 for fork tests */
#define TEST_FORKING  1

static int test_var;

/*
 * Do some child stuff
 *
 */

void do_child (char *mess)
{
  test_var = 1;

  while (1)
  {
    printf (mess);
  }
}

/*
 * Main of init
 */

void init_main (void)
{
  //static char   *video_mem;
  //char           tmp;

  test_var = 0;

  if (TEST_FORKING) {
    printf ("forking... ");
    switch (fork()) {
    case 0:
      do_child("1");
      break;
      
    case -1:
      printf ("Fork failed!\n");
      while(1);
    }
    
    printf ("forking... ");
    switch (fork()) {
    case 0:
      do_child("2");
      break;
      
    case -1:
      printf ("Fork failed!\n");
      while(1);
    }
    
    printf ("forking... ");
    switch (fork()) {
    case 0:
      do_child("3");
      break;
      
    case -1:
      printf ("Fork failed!\n");
      while(1);
    }

    printf ("forking... ");
    switch (fork()) {
    case 0:
      do_child("4");
      break;
      
    case -1:
      printf ("Fork failed!\n");
      while(1);
    }
    
    printf ("forking... ");
    switch (fork()) {
    case 0:
      do_child("5");
      break;
      
    case -1:
      printf ("Fork failed!\n");
      while(1);
    }

    printf ("forking... ");
    switch (fork()) {
    case 0:
      do_child("6");
      break;
      
    case -1:
      printf ("Fork failed!\n");
      while(1);
    }
  }

  /* We're parent */
  while (1)
  {
    printf ("p");
  }

  /* Do a page fault */
  //video_mem = (char *) 0xb8000;
  //video_mem[0] = '0';
  //tmp = video_mem[0];
  
  
  /* Never exit init */
  while (1);
}
