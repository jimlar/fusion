/*
 *
 * Fork - for fusion
 * (c) 1999 Jimmy Larsson
 *
 */

#include "process.h"
#include "syscall.h"
#include "printk.h"

#include "fork.h"

/*
 * Fork 'source' and put the new process in
 * run queue
 *
 * returns pid of new child
 */

int do_fork (process_t *source)
{
  process_t      *new_child;

  new_child = get_proc_entry();

  //No free proc-table entry?
  if (new_child == NULL)
    return -1;

  printf ("FORK: copying tss and proc data\n");
  
  //Copy misc data
  new_child->uid = source->uid;
  new_child->gid = source->gid;
  new_child->priority = source->priority;

  //Copy TSS, CR3 should not be copied later, when mem is done
  new_child->tss.backlink = source->tss.backlink;
  new_child->tss.eip = source->tss.eip;
  new_child->tss.eflags = source->tss.eflags;
  
  new_child->tss.eax = source->tss.eax;
  new_child->tss.ebx = source->tss.ebx;
  new_child->tss.ecx = source->tss.ecx;
  new_child->tss.edx = source->tss.edx;

  new_child->tss.esp = source->tss.esp;
  new_child->tss.ebp = source->tss.ebp;
  new_child->tss.esi = source->tss.esi;
  new_child->tss.edi = source->tss.edi;

  new_child->tss.es = source->tss.es;
  new_child->tss.cs = source->tss.cs;
  new_child->tss.ss = source->tss.ss;
  new_child->tss.ds = source->tss.ds;
  new_child->tss.fs = source->tss.fs;
  new_child->tss.gs = source->tss.gs;

  new_child->tss.ss0 = source->tss.ss0;
  new_child->tss.esp0 = source->tss.esp0;


  new_child->tss.ldt = source->tss.ldt;
  new_child->tss.trace = source->tss.trace;
  new_child->tss.iomapbase = source->tss.iomapbase;

  printf ("FORK: setting up mem-tables\n");

  //Do mem part of fork
  do_fork_vm (source, new_child);

  //Childs return is 0, does this work??
  new_child->tss.eax = 0;


  printf ("FORK: making child ready\n");

  //Put it into run queue
  make_process_ready (new_child);
 

  printf ("FORK: done\n");

  return new_child->pid;
}


