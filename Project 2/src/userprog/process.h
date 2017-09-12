#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (int status); /* Pass status for parent thread and to print to user. */
void process_activate (void);

/* Structure of child object. Used by Parent process to keep track of the status of its childs.*/
struct child
{
  int tid;
  int status;
  struct list_elem elem;
};

#endif /* userprog/process.h */
