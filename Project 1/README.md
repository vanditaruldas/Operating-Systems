			+--------------------+
			|       CSE 521      |
			| PROJECT 1: THREADS |
			|   DESIGN DOCUMENT  |
			+--------------------+
				   

			     ALARM CLOCK
			     ===========

---- DATA STRUCTURES ----

>> A1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

In thread.h :
Modified : struct thread
Added -    int64_t sleep_ticks;  		        /* Number of ticks to sleep. */

In timer.c :
Added : static struct list sleep_list;

For thread.h:
sleep_ticks is added to store the number of ticks the thread is supposed to sleep. It is set as 0 initially and also after sleep in complete.

For timer.c
sleep_list keeps the list of threads that are sleeping.
  
---- ALGORITHMS ----

>> A2: Briefly describe what happens in a call to timer_sleep(),
>> including the effects of the timer interrupt handler.

First the size of ticks to sleep is checked. If it is less than or equal to zero then nothing is done. If its greater than zero, interrupts are turned off and sleep_ticks is set for the thread and thread is added to sleeping list. Then thread_block is called. Interrupts are then turned on. As interrupt are turned off the interrupt handler does not execute.

>> A3: What steps are taken to minimize the amount of time spent in
>> the timer interrupt handler?

To ensure that minimum time is spent in the handler, a list of sleeping threads is maintained. So that only those threads are checked for waking up.

---- SYNCHRONIZATION ----

>> A4: How are race conditions avoided when multiple threads call
>> timer_sleep() simultaneously?

As interrupts are turned off race conditions are avioded during multiple calls. 

>> A5: How are race conditions avoided when a timer interrupt occurs
>> during a call to timer_sleep()?

As interrupts are turned off race conditions are avioded when timer interrupt occurs during timer_sleep. This is done cause interrupt handlers cant take locks.

---- RATIONALE ----

>> A6: Why did you choose this design?  In what ways is it superior to
>> another design you considered?

This is implementation does not have too much overhead. The sleep ticks is stored in the thread which avaiable in the sleep list, so no extra fetching of data is required. Initially we had had called thread_foreach which looks at all the threads. This way is superior as it checks only the threads that are currently sleeping.


			 PRIORITY SCHEDULING
			 ===================

---- DATA STRUCTURES ----

>> B1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

In thread.h
Modified : struct thread
Added -    int base_priority;                  /* Base priority used in priority Donation. */
           struct list locks_held;             /* List of locks held by thread. */
           struct lock *waiting_lock;          /* Lock the thread is waiting for. */
		   
In sync.h
Modified : struct lock
Added -    struct list_elem elem;      /* List Element for lock used to createa list of locks in priority donation. */
           int priority;               /* The current priority of the thread that has the lock. */
           int base_priority;          /* The priority at which the thread got this lock. */		   
		   
For thread :		   
1) base_priority is used to store the priority of the thread if it is changed using set_priority after a donation has taken place. Once all the donations are complete priority of thread is returned to this priority if it is not 0.
3) locks_held is the list of locks held by a thread. Each lock has it own priority, so when one lock is released if the thread still holds locks the priority set to highest priority lock.
3) waiting_lock is set if the thread is waiting for a lock that is already taken by someother lock. This helps create a chain for priority donation.

For lock :
1) elem is added so that the lock can be added to the list of locks taken by a thread.
2) priority is the highest priority given to the holder of this lock.
3) base_priority is the priority at which the thread acquired this lock.


>> B2: Explain the data structure used to track priority donation.
>> Use ASCII art to diagram a nested donation.  (Alternately, submit a
>> .png file.)

+------------+
|            |
|  Thread 1  |
+------------+
      |Locks Held List Thread 1
      |
+-----+------+                       +------------+
|   Lock 1   | <---------------------+            |
+-----+------+     Waiting Lock      |  Thread 2  |
      |                              +------------+
      |                                    |Locks Held List Thread 2
+-----+------+                             |
|   Lock 2   | <-------+             +-----+------+                       +------------+
+------------+         |             |   Lock 3   | <---------------------+            |
                       |             +------------+     Waiting Lock      |  Thread 3  |
          Waiting Lock |                                                  +------------+
                       |                                                        |Locks Held List Thread 3
                       +------------+                                           |
                       |            |                                     +-----+------+
                       |  Thread 4  |                                     |   Lock 4   |
                       +------------+                                     +------------+
                             |Locks Held List Thread 4
                             |
                       +-----+------+                       +------------+
                       |   Lock 5   | <---------------------+            |
                       +------------+     Waiting Lock      |  Thread 5  |
                                                            +------------+



---- ALGORITHMS ----

>> B3: How do you ensure that the highest priority thread waiting for
>> a lock, semaphore, or condition variable wakes up first?

All the threads waiting for a lock, semaphore or condition are ordered by priority. So the thread at the head has the priority and that wakes up first.

>> B4: Describe the sequence of events when a call to lock_acquire()
>> causes a priority donation.  How is nested donation handled?

When a lock_acquire() is called, if the lock already has a holder then thread_donate. thread_donate checks if this lock has already donated a priority to this thread. If it has it removes the lock from locks_held list of the thread. Then it sets the new priority to the lock and thread, and then adds the lock back to list ensuring that the list of locks are in order of donate priority. Then it checks if the holder thread has waiting_lock set. If it has it repeats this proces for the next lock and this way donates the priority until it reaches the main thread that actually needs the priority.

>> B5: Describe the sequence of events when lock_release() is called
>> on a lock that a higher-priority thread is waiting for.

When lock_release() is called it calls return_priority. return_priority checks if any priority has been donated from this lock. If it has then it removes the lock from the locks_held list of the thread. Then if the list has any more locks it takes the highest priority from those locks(that is the first lock as they are already order by priority) and set this priority to the thread. If there are not more elements in the list it checks if the priority of the thread has been changed using set_priority which is done by checking the base_priority variable of the thread. If the value is greater than zero, it is set to this priority otherwise it is set to the base_priority stored at the lock which is the priority of the thread when the lock was acquired.

---- SYNCHRONIZATION ----

>> B6: Describe a potential race in thread_set_priority() and explain
>> how your implementation avoids it.  Can you use a lock to avoid
>> this race?

A race could occur while the priority is being changed if the timer_interrupt also tries to change the priority which is does after every 4 ticks. This wont occur as the interrupt has been turned off to synchornize with the timer_interrupt. Locks cannot be used as interrupt handlers cant take locks.

---- RATIONALE ----

>> B7: Why did you choose this design?  In what ways is it superior to
>> another design you considered?

This is the simplest and most efficient way to store all the priorities and run through a chain if required. Initially we considered maintaining a list of threads that we were waiting on a thread to release a lock but that would have taken up too much space of the thread and also would have been very complicated and in efficient way to check for chains.

			  ADVANCED SCHEDULER
			  ==================

---- DATA STRUCTURES ----

>> C1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration.  Identify the purpose of each in 25 words or less.

In thread.h
Modified : struct thread
Added -    int recent_cpu;                     /* recent_cpu of the thread. */
           int nice;                           /* nice value of the thread. */

In thread.c
Added : int load_avg

For thread.h:
1) recent_cpu stores the recent_cpu value for the thread.
2) nice stores the nice value for the threads.

For thread.c
1) load_avg store the load average for the system.		   

---- ALGORITHMS ----

>> C2: Suppose threads A, B, and C have nice values 0, 1, and 2.  Each
>> has a recent_cpu value of 0.  Fill in the table below showing the
>> scheduling decision and the priority and recent_cpu values for each
>> thread after each given number of timer ticks:

timer  recent_cpu    priority   thread
ticks   A   B   C   A   B   C   to run
-----  --  --  --  --  --  --   ------
 0      0   0   0  63  61  59     A
 4      4   0   0  62  61  59     A
 8      8   0   0  61  61  59     B
12      8   4   0  61  60  59     A
16      12  4   0  60  60  59     B
20      12  8   0  60  59  59     A
24      16  8   0  59  59  59     C
28      16  8   4  59  59  58     B
32      16  12  4  59  58  58     A
36      20  12  4  58  58  58     C

>> C3: Did any ambiguities in the scheduler specification make values
>> in the table uncertain?  If so, what rule did you use to resolve
>> them?  Does this match the behavior of your scheduler?

It does not say if two threads are the same priority which thread will be scheduled. I scheduled the thread that has not run recently. That is how I have order the threads in my ready list.

>> C4: How is the way you divided the cost of scheduling between code
>> inside and outside interrupt context likely to affect performance?

Most of the calculations are done inside the timer_interrupt which has interrupt disabled. Only the nice value is set outside interrupt but the interrupt has to be turned of as the timer_interrupt cant taake locks. In a system with many threads, this is a very inefficient way to calcuate values as it will affect performance.

---- RATIONALE ----

>> C5: Briefly critique your design, pointing out advantages and
>> disadvantages in your design choices.  If you were to have extra
>> time to work on this part of the project, how might you choose to
>> refine or improve your design?

Used a neat and clean approach. Minimal variables and easy to understand. 

Disadvantages are the interrupt have to be turned off alot to synchornize with priority being updated by the  timer_interrupt and using a sorted list takes alot of time. Everytime the priority is recalculated the list needs to be ordered again. Also the fixed-point calculations could cause overflow for large values. 

A better data structure should be used for maintainf the threads.

>> C6: The assignment explains arithmetic for fixed-point math in
>> detail, but it leaves it open to you to implement it.  Why did you
>> decide to implement it the way you did?  If you created an
>> abstraction layer for fixed-point math, that is, an abstract data
>> type and/or a set of functions or macros to manipulate fixed-point
>> numbers, why did you do so?  If not, why not?

A new header file was created with all the macros to manipulate fixed-point numbers. This is a clean way to do it. Always for the macros to be used in other files if required.
