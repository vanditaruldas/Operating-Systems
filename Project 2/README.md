Implementing Process Termination Messages, Argument Passing, System Calls and Denying Writes to Executables.

		+--------------------------+
       	     |	     CSE 521		|
		     | PROJECT 2: USER PROGRAMS	|
		     | 	   DESIGN DOCUMENT     	|
		     +--------------------------+

			   ARGUMENT PASSING
			   ================

---- DATA STRUCTURES ----

>> A1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

struct thread : char *prog_name						/* Program name*/

This is used to print the program name in process termination message

---- ALGORITHMS ----

>> A2: Briefly describe how you implemented argument parsing.  How do
>> you arrange for the elements of argv[] to be in the right order?
>> How do you avoid overflowing the stack page?

Each argument is extract using strtok_r. Then esp is decreased to string length of the arugment + 1. Then *esp is stored in argv array to be used for storing the addresses. Then memcpy is called for the argument. After all the arguments have parsed we have pushed all the arguments into the stack and have our argv array and argc which is count of arguments. Then the sentinel is added. There the address of all the elements in the argv array are pushed into the stack starting from the back and this way the right order is maintained. Then the address of argv is pushed which is the currect *esp. Then argc is pushed and then another sentinel.

---- RATIONALE ----

>> A3: Why does Pintos implement strtok_r() but not strtok()?

In pintos the kernel separates commands into command line(program name) and arguments. So we need this address so that we can use it latter. The difference between strtok and strtok_r is that strtok_r has the save_ptr pointer that helps us achieve this.

>> A4: In Pintos, the kernel separates commands into a executable name
>> and arguments.  In Unix-like systems, the shell does this
>> separation.  Identify at least two advantages of the Unix approach.

1) The Unix system can check if the argumentname and if the number of arguments are within limit before passing to kernel. This avoids kernel failures.
2) Less time spent in kernel.

			     SYSTEM CALLS
			     ============

---- DATA STRUCTURES ----

>> B1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

struct file_fd
{
	struct file* file;
	int fd;
	struct list_elem elem;
};

struct child
{
  int tid;
  int status;
  struct list_elem elem;
};

struct thread : 
struct list file_list;
int fd;
bool parentwaiting;		
struct list child_list;
struct semaphore wait;
struct child* child_obj;
tid_t parent;

struct file_fd is used to maintin the file descriptors assigned to a filename for a particular thread.
struct child is used to maintain the status of any thread so that these details can be accessed by the parent thread even after the child thread is killed.
struct list file_list is the listof files currnetly open by a thread.
int fd is the fd to be assigned to the next file opened by that thread.
bool parentwaiting is used to check if wait has already been called on this process.
struct list child_list list of child processes created by that thread.
struct semaphore wait used to put parent thread to sleep and wake up once child is complete.
struct child* child child_obj for the current thread.
tid_t parent the parent of the current thread.

>> B2: Describe how file descriptors are associated with open files.
>> Are file descriptors unique within the entire OS or just within a
>> single process?

When file open is called for a particular file. A new file_fd object is created which has the file_name and fd number, that is to be returned. The fd are unique within a single process are pre the requirement document. Every thread has a variable fd that stores the next fd number to be used. This fd and filename is stored in the file_fd object created. The list_elem of this object is added to the file_list of the thread for future use. The fd of the thread is then increased for the next file open call and the fd stored in file_fd is returned. If the file does not exist in the file system -1 is returned.

---- ALGORITHMS ----

>> B3: Describe your code for reading and writing user data from the
>> kernel.

For both read and write there are 3 arugments. First these 3 are validated to be valid pointers. Apart from this we check if the fd is not greater than the current fd of the thread to ensure it is valid. We also check if fd = 0 was called for write and fd = 1 was called for read. In both these cases the thread exists with -1 status. After all these checks are passed, read or write functions are called. If fd=1 write function writes to console otherwise it searches for the file in the file_list if file is present the file_write is called otherwise -1 is returned. If fd=0 read function reads from keyboard otherwise it searches for the file in the file_list if file is present the file_read is called otherwise -1 is returned.

>> B4: Suppose a system call causes a full page (4,096 bytes) of data
>> to be copied from user space into the kernel.  What is the least
>> and the greatest possible number of inspections of the page table
>> (e.g. calls to pagedir_get_page()) that might result?  What about
>> for a system call that only copies 2 bytes of data?  Is there room
>> for improvement in these numbers, and how much?

If its a full page the least we need is 1 if we the address returned is the page head.
If it is more than a full page, there are 2 cases. Case 1, if it is continuous then we need 2 to check the start and end pointers. Case 2, if it is non-continuous we need to check every address. 

>> B5: Briefly describe your implementation of the "wait" system call
>> and how it interacts with process termination.

The wait system call firsts check if the child_thread exist or if it is already waiting or if the current thread is no the parent. If any of these cases are true thread is exited with -1. Otherwise parentwaiting is set as true and then wait semaphore down is called to put the the semaphore to sleep. In the process termination funcion, the thread sets it exit status in the child_obj frees all its child objects and file objects using child_list and file_list. Then wait semaphore up is called to wake up the parent thread.

>> B6: Any access to user program memory at a user-specified address
>> can fail due to a bad pointer value.  Such accesses must cause the
>> process to be terminated.  System calls are fraught with such
>> accesses, e.g. a "write" system call requires reading the system
>> call number from the user stack, then each of the call's three
>> arguments, then an arbitrary amount of user memory, and any of
>> these can fail at any point.  This poses a design and
>> error-handling problem: how do you best avoid obscuring the primary
>> function of code in a morass of error-handling?  Furthermore, when
>> an error is detected, how do you ensure that all temporarily
>> allocated resources (locks, buffers, etc.) are freed?  In a few
>> paragraphs, describe the strategy or strategies you adopted for
>> managing these issues.  Give an example.

We have used the is_user_vaddr to check if all the points in the stack are valid before dereferencing them. Then additional pagedir_get_page is called for all strings to validate them. In any case all failures call thread_exit which calles process_exit. In process_exit all allocated memory and locks are released.

For example if there is any exception the system calls kill function. Here thread_exit is called. We have modified this to pass the status as -1 so that any parent process and user will know that this process has been killed by kernel. thread_exit in turn calls process_exit where we free all the allocated memory and locks.

---- SYNCHRONIZATION ----

>> B7: The "exec" system call returns -1 if loading the new executable
>> fails, so it cannot return before the new executable has completed
>> loading.  How does your code ensure this?  How is the load
>> success/failure status passed back to the thread that calls "exec"?

In process_execute after the parent thread returns from thread_create we make the child thread call sema down of semaphore down. This puts the parent thread to sleep. Once load/fails or succeeds  the child thread calls sema up and the parent thread continues from where we put it to see. To handle succes or failure in load, once the parent thread wakes up we check if the child thread is still part of the all_list thread list. If it isnt we return -1. Which informs that there has been a failure.

>> B8: Consider parent process P with child process C.  How do you
>> ensure proper synchronization and avoid race conditions when P
>> calls wait(C) before C exits?  After C exits?  How do you ensure
>> that all resources are freed in each case?  How about when P
>> terminates without waiting, before C exits?  After C exits?  Are
>> there any special cases?

Synchronization is ensured using the wait semaphore. Whenever a thread exits it sets its status so that the parent thread can access it and then releases all the child objects it is holding.

Case 1 : P calls wait from C exits. We get the child thread using thread_get (The function we wrote to return the structure of any thread in the system). This child thread calls wait semaphore to put the parent to sleep. Once C exits it wakes up P and then P can check the status.

Case 2 : P calls wait from C exits. When C exits it has already stored in child_obj that is linked to the P and has not been freed. P can check it status from that object.

Case 3 : P terminates without waiting for C to exit. When P terminates it frees all its resources so there is no issue.

Case 4 : P terminates after C exit. C will release its resources and P will release its respective resources.

---- RATIONALE ----

>> B9: Why did you choose to implement access to user memory from the
>> kernel in the way that you did?
We checked the memory pointers before dereferencing it as described in method 1. We didnt implement the second option as we could not fully understand it and were not comfortable in solving the issues that would occur while implementing it.

>> B10: What advantages or disadvantages can you see to your design
>> for file descriptors?
Advantages : Thread space is not used to implement. Kernel is aware of all open files.
Disadvantages : Too much memory will be allocated in huge systems. A global fd would be a much better way to implement it. Takes a lot of effort to keep track.

>> B11: The default tid_t to pid_t mapping is the identity mapping.
>> If you changed it, what advantages are there to your approach?

We havent changed the identity mapping.
