#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
int write (int fd, const void *buffer, unsigned length);
void close (int fd);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);

#endif /* userprog/syscall.h */
