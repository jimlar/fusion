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
#define NUM_FORKS  1

static int test_var;

/*
 * Do some child stuff
 *
 */

void do_child (int num) {
  //test_var = 1;

  while (1) {
    printf ("%u", num);
  }
}

/*
 * Main of init
 */

void init_main (void) {

  int i = 0;
  //static char   *video_mem;
  //char           tmp;

  //test_var = 0;

  for (i = 0; i < NUM_FORKS; i++) {
    
    printf ("fork(%u)", (i + 1));
    switch (fork()) {
    case 0:
      do_child(i);
      break;
      
    case -1:
      printf ("Fork failed!\n");
      while(1);
    }
  }

  /* We're parent */
  while (1) {
    printf ("p");
  }

  /* Do a page fault */
  //video_mem = (char *) 0xb8000;
  //video_mem[0] = '0';
  //tmp = video_mem[0];
  
  
  /* Never exit init */
  while (1);
}
