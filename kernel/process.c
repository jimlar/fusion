/*
 * The processes of Fusion
 * 
 * (c) Jimmy Larsson 1998
 * 
 */

#include "switching.h"
#include "process.h"
#include "interrupt.h"
#include "port_io.h"
#include "printk.h"
#include "ipc.h"

#define TIMER_HZ         100
#define TIMER_LATCH_WORD (1193180/TIMER_HZ)

/* Timer 0, mode 3, binary, write */
#define TIMER_MODE_WORD  0x36 

#define TIMER_COUNTER0_PORT     0x40
#define TIMER_MODE_CONTROL_PORT 0x43

  
process_t   processes[MAX_PROCESSES];
static int         next_pid;

/** Lists for scheduler */
static process_t  *run_queue;
static process_t  *run_queue_tail;

static process_t  *free_list;

/** Locks for the lists */
static spin_lock_t  free_list_lock;
static spin_lock_t  run_queue_lock;


/** The currently running process */
process_t   *current_proc;


/*
 * IRQ handler to call schedule
 *  (for now)
 */

int scheduler_irq (int irq)
{
  schedule();
  return 0;
}

/*
 * Initialize process subsystem
 *
 *
 *
 */

void init_processses ()
{
  int i;

  /** init hardware switching stuff */
  init_switching();


  /* mark all entries free */
  for (i = 0; i < MAX_PROCESSES; i++)
  {
    processes[i].state = STATE_FREE;
    
    if (i < (MAX_PROCESSES - 1))
      processes[i].next = &processes[i + 1];

    if (i > 0)
      processes[i].prev = &processes[i - 1];
  }

  processes[MAX_PROCESSES - 1].next = NULL;
  processes[0].prev = NULL;

  free_list = &processes[0];

  next_pid = 1;
  run_queue = NULL;
  run_queue_tail = NULL;
  current_proc = NULL;

  release_spin_lock (&free_list_lock);
  release_spin_lock (&run_queue_lock);

  /* hook scheduler with irq 0 */
  register_irq_handler (scheduler_irq, 0);

  /* fire up timer */
  outb (TIMER_MODE_WORD, TIMER_MODE_CONTROL_PORT);
  outb (TIMER_LATCH_WORD & 0xff, TIMER_COUNTER0_PORT);
  outb (TIMER_LATCH_WORD >> 8, TIMER_COUNTER0_PORT);
}


/*
 * Get free process entry
 *
 * (initializes pid) 
 *
 * returns NULL if none availiable
 *
 * 
 */

process_t *get_proc_entry ()
{
  process_t *free_proc; 

  //Lock free list
  get_spin_lock (&free_list_lock);

  free_proc = free_list;
  
  if (free_list == NULL)
    return NULL;

  free_list = free_list->next;

  //Release lock
  release_spin_lock (&free_list_lock);

  free_proc->pid = next_pid++;
  free_proc->next = NULL;
  free_proc->prev = NULL;
  free_proc->state = STATE_READY;

  return free_proc;
}

/*
 * Release a process entry
 *
 */

void put_proc_entry (process_t *proc)
{
  //Lock free list
  get_spin_lock (&free_list_lock);

  proc->next = free_list;
  proc->prev = NULL;
  free_list = proc;

  //Release lock
  release_spin_lock (&free_list_lock);

  proc->state = STATE_FREE;
}


/*
 * Add process onto run queue end
 * 
 *
 */

void make_process_ready (process_t  *proc)
{
  //Get run queue lock
  get_spin_lock (&run_queue_lock);

  if (run_queue == NULL)
  {
    run_queue = proc;
    run_queue_tail = proc;
    release_spin_lock (&run_queue_lock);
    return;
  }

  run_queue_tail->next = proc;
  proc->prev = run_queue_tail;
  proc->next = NULL;
  proc->state = STATE_READY;

  run_queue_tail = proc;

  //Release lock
  release_spin_lock (&run_queue_lock);
}


/*
 * Schedule, pretty simple right now...
 * Plain old round robin, without priorities
 *
 */

void schedule ()
{
  process_t *new_proc;

  /** Nothing new to run? */
  if (run_queue == NULL)
  {
    if (current_proc != NULL)
    {
      load_task (&current_proc->tss, BUSY_SET);
      return;
    } else
    {
      load_idle_task (BUSY_SET);
      return;
    }
  }

  /** Put old process on run queue, if any */
  if (current_proc != NULL)
  {
    make_process_ready (current_proc);
  }

  /** Dequeue new proc from head of run queue */

  //Lock run queue
  get_spin_lock (&run_queue_lock);

  new_proc = run_queue;
  run_queue = new_proc->next;
  run_queue->prev = NULL;

  //Done with run queue
  release_spin_lock (&run_queue_lock);

  new_proc->state = STATE_RUNNING;

  current_proc = new_proc;

  load_task (&current_proc->tss, BUSY_SET);
}
