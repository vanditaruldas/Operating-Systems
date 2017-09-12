#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "devices/input.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

struct lock filesystem_lockdown; /* Used to synchornize file system calls. */

/* Structure used by thread to link any fd of a thread to its file name.*/
struct file_fd
{
	struct file* file;
	int fd;
	struct list_elem elem;
};

void
syscall_init (void) 
{
  lock_init(&filesystem_lockdown);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!( is_user_vaddr ((const void *)f->esp))) /* check if system call number is a valid pointer.*/
  { 
       printf("\n Inside");
       thread_exit(-1);
  }

  int syscall = *(int *) f->esp;
  if((syscall <0)||(syscall>20))
      thread_exit(-1);
  switch (syscall)
  {
	case SYS_HALT:
	{
          shutdown_power_off();
	}
	case SYS_EXIT:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
		
		int exit_stat = *((int *) f->esp+1);
		thread_exit(exit_stat);
	}
	case SYS_WRITE:
	{ 
		if (!( is_user_vaddr ((const void *)(f->esp + 4)) && is_user_vaddr ((const void *)(f->esp + 8)) && is_user_vaddr ((const void *)(f->esp + 12)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
		
		int fd = *(int *)(f->esp+4);
        void *buffer = *(char**)(f->esp + 8);
        unsigned size = *(unsigned *)(f->esp + 12);

		if(fd == 0)
		  thread_exit(-1);

		if(fd > thread_current()->fd)
		  thread_exit(-1);

                
        buffer = pagedir_get_page(thread_current()->pagedir, (const void *) buffer);
		if(!buffer)
			thread_exit(-1);
		
		f->eax = write (fd, buffer, size);
		break;
    }
	case SYS_WAIT:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
	        int wait = *(int *) (f->esp + 4);
		f->eax = process_wait(wait);
		break;
	}
	case SYS_EXEC:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);

		void* filename = *(char**)(f->esp + 4);
		filename = pagedir_get_page(thread_current()->pagedir, (const void *)filename);
                if(!filename)
			thread_exit(-1);
		
		char *str = *(char **)(f->esp + 4);
                f->eax = process_execute((const char*)str);
		break;
	}
	case SYS_CREATE:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)) && is_user_vaddr ((const void *)(f->esp + 8)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);

	        void *buffer = *(char**)(f->esp + 4);
                buffer = pagedir_get_page(thread_current()->pagedir, (const void *) buffer);
                if(!buffer)
                        thread_exit(-1);
		
         	lock_acquire(&filesystem_lockdown);
                f->eax = filesys_create((const char *)buffer, *(int *) (f->esp + 8));
		lock_release(&filesystem_lockdown);
		
                break;
	}
	case SYS_REMOVE:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);

    	void *filename = *(char**)(f->esp + 4);
        filename = pagedir_get_page(thread_current()->pagedir, (const void *) filename);
		
		if(!filename)
			thread_exit(-1);
		
		lock_acquire(&filesystem_lockdown);
		f->eax = filesys_remove((const char*)filename);
		lock_release(&filesystem_lockdown);
		break;
	}
	case SYS_OPEN:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);

    	void *filename = *(char**)(f->esp + 4);
        filename = pagedir_get_page(thread_current()->pagedir, (const void *) filename);
		
		if(!filename)
			thread_exit(-1);
		
		f->eax = open((const char*)filename);
		break; 		
	}
	case SYS_FILESIZE:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
		
		int fd = *(int *) (f->esp + 4);
		f->eax = filesize(fd);
		break;
	}
	case SYS_READ:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)) && is_user_vaddr ((const void *)(f->esp + 8)) && is_user_vaddr ((const void *)(f->esp + 12)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
		
        int fd = *(int *)(f->esp + 4);
        void *buffer = *(char**)(f->esp + 8);
	    void *fd_check = (int *)(f->esp + 4); 

        if(fd > thread_current()->fd)
             thread_exit(-1);
  	
        if(!(is_user_vaddr ((const void *) fd_check)))
           thread_exit(-1);

				  
        if(!(is_user_vaddr ((const void *) buffer)))
              thread_exit(-1);

                if(fd == 1)
                  thread_exit(-1);


        buffer = pagedir_get_page(thread_current()->pagedir, (const void *) buffer);
        unsigned size = *(unsigned *)(f->esp + 12);

		if(!buffer)
			thread_exit(-1);
		
		f->eax = read (fd, buffer, size);
		break;
	}

	case SYS_SEEK:
	{
		if (!( is_user_vaddr ((const void *)(f->esp + 4)) && is_user_vaddr ((const void *)(f->esp + 8)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
		
		int fd = *(int *) (f->esp + 4);
		unsigned position = *(unsigned *) (f->esp + 8);
		lock_acquire(&filesystem_lockdown);
		struct file* f = NULL;
		struct list_elem *e;
		for (e = list_begin (&thread_current()->file_list); e != list_end (&thread_current()->file_list); e = list_next (e))
		{
			struct file_fd *ffd = list_entry (e, struct file_fd, elem);
			if (fd == ffd->fd)
			{
				f = ffd->file;
			}
		}

		if(f)
		{
			file_seek(f, position);
		}
		lock_release(&filesystem_lockdown);
		break;
	} 
	case SYS_TELL:
	{ 
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
		
		int fd = *(int *) (f->esp + 4);
		f->eax = tell(fd);
		break;
	}
	case SYS_CLOSE:
	{ 
		if (!( is_user_vaddr ((const void *)(f->esp + 4)))) /* Checking if arguments for call are valid pointers.*/
			thread_exit(-1);
		
		int fd = *(int *) (f->esp + 4);
		close(fd);
		break;
	}
  }
}

/* Write function. Writes to console if fd is 1 or writes to file if it is opened by thread.*/
int write (int fd, const void *buffer, unsigned size)
{
	int bytes_written = 0;
	if(fd == 1)
	{
        putbuf(buffer, size);
		bytes_written = size;
	}
	else
	{
		lock_acquire(&filesystem_lockdown);
		struct file* f = NULL;
		struct list_elem *e;
		for (e = list_begin (&thread_current()->file_list); e != list_end (&thread_current()->file_list); e = list_next (e))
		{
			struct file_fd *ffd = list_entry (e, struct file_fd, elem);
			if (fd == ffd->fd)
			{
				f = ffd->file;
			}
		}

		if(!f)
		{
			bytes_written =  -1;
		}
		else
		{
			bytes_written = file_write(f, buffer, size);
		}
		lock_release(&filesystem_lockdown);
	}
	return bytes_written;
}

/* Close a file and removes its fd and filename linking with the thread. */
void close (int fd)
{
	lock_acquire(&filesystem_lockdown);
	struct list_elem *e = list_begin(&thread_current()->file_list);
	struct list_elem *next;
	  
	while (e != list_end (&thread_current()->file_list))
	{
	  next = list_next(e);
	  struct file_fd* ffd = list_entry (e, struct file_fd, elem);
	  if ((fd == ffd->fd)||(fd == 0))
	  {
		file_close(ffd->file);
		list_remove(&ffd->elem);
		free(ffd);
		if(fd != 0)
			break;
	  }
	  e = next;
	}
	lock_release(&filesystem_lockdown);
}

/* Opens a file. Creates fd and filename linking for thread and returns fd.*/
int open (const char *filename)
{
	lock_acquire(&filesystem_lockdown);
	int fd;
	struct file* f = filesys_open(filename);
	if(!f)
	{
		fd = -1;
	}
	else
	{
		  struct file_fd *ffd = malloc(sizeof(struct file_fd));
		  ffd->file = f;
		  ffd->fd = thread_current()->fd;
		  fd = thread_current()->fd;
		  thread_current()->fd++;
		  list_push_back(&thread_current()->file_list, &ffd->elem);
	}
	lock_release(&filesystem_lockdown);
	return fd;
}

/* Returns file size based of file derived from fd.*/
int filesize (int fd)
{
	lock_acquire(&filesystem_lockdown);
	struct file* f = NULL;
	struct list_elem *e;
	for (e = list_begin (&thread_current()->file_list); e != list_end (&thread_current()->file_list); e = list_next (e))
	{
		struct file_fd *ffd = list_entry (e, struct file_fd, elem);
		if (fd == ffd->fd)
		{
			f = ffd->file;
		}
	}
	int file_size;
	if(!f)
	{
		file_size = -1;
	}
	else
	{
		file_size = file_length(f);
	}

	lock_release(&filesystem_lockdown);
    return file_size;
}

/* Reads dataa from keyboard if fd = 0. Otherwise reads from file if file has been opened by thread.*/
int read (int fd, void *buffer, unsigned size)
{
	int bytes_read;
	if(fd == 0)
	{
		uint8_t* readinput = (uint8_t *) buffer;
		for (unsigned i = 0; i < size; i++)
				readinput[i] = input_getc();
		bytes_read = size;
	}
	else
	{
		lock_acquire(&filesystem_lockdown);
		struct file* f = NULL;
		struct list_elem *e;
		for (e = list_begin (&thread_current()->file_list); e != list_end (&thread_current()->file_list); e = list_next (e))
		{
			struct file_fd *ffd = list_entry (e, struct file_fd, elem);
			if (fd == ffd->fd)
			{
				f = ffd->file;
			}
		}

		if(!f)
		{
			bytes_read =  -1;
		}
		else
		{
			bytes_read = file_read(f, buffer, size);
		}
		lock_release(&filesystem_lockdown);
	}
	return bytes_read;
}

/* Returns the next read or write postion of a file dervied from the fd passed.*/
unsigned tell (int fd)
{
	unsigned return_position;
	lock_acquire(&filesystem_lockdown);
	struct file* f = NULL;
	struct list_elem *e;
	for (e = list_begin (&thread_current()->file_list); e != list_end (&thread_current()->file_list); e = list_next (e))
	{
		struct file_fd *ffd = list_entry (e, struct file_fd, elem);
		if (fd == ffd->fd)
		{
			f = ffd->file;
		}
	}

	if(!f)
	{
		return_position = -1;
	}
	else
	{
		return_position = file_tell(f);
	}
	lock_release(&filesystem_lockdown);
	return return_position;
}

