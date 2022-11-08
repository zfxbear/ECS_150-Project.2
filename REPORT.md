# PROJECT2 REPORT

# 1 Project Goal
- This project aims to help us understand how to implement a user-level thread
 library, enabled preempt using signal handling, and implement a semaphore. 
 Besides, we also get the practice of using c-language to implement basic queue 
 data structure and writing unit tests for code robustness testing.

# 2 Project team members and roles
- Evan Zhou, group leader, in charge of
   * design, task breakdown, and allocation to members, submission of git branch
   * implementation and report on uthread.c, preempt.c and test_preempt.c

- QiJun Liang, group memeber, in charge of
   * implementation and report of queue.c and queue_tester_example.c
   * implementation and report of sem.c

# 3 Project Design And Implementation Ideas
## An overview of the implementation
- Our high-level idea is, we implemented the queue using a double linked-list. 
  We utilized the context.c file to understand ucontext lib and implement the
  context-switching logic between main and schedulable threads. For preempt, it
  is just following stand signal handling and timer set up GNU lib usage. And 
  for semaphore, we use a count plus a queue to implement.

## Specific implementation and Design
### libuthread/Makefile
----------------------------
**Idea**
- We just followed the slides provided by Professor on "Makefile Tutorial" as
  the basics of the Makefile
- We added `targets` to be the final static archive file `libuthread.a`
- We added `objs` to be all objects .o files needed for the archive file.

**Reference:**
- Makefile tutorial - Discussion Slides Professor JP provided
- How to archive a static library:
    - https://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html

### libuthread/queue.c
----------------------------
**Idea**
- Queue is implemented by using a double linkedlist. linkedlist node is defined
  as `struct node` with one `data` field, one `prevNode` ptr, one `nextNode` ptr
- Queue itself has a empty dummyHead pointing to the actual head, so that empty
  queue could be deal with gracefully. The head is used for *dequeue*, and the
  tail node is used for *appending/enqueueu* new item at the end for FIFO order.
  It also has a count field to track the current queue lenght easily.

**Chanllenge**
- One chanllege we faced was dealing with NULL pointers in special cases, if we
  access a pointer without checking equal to NULL, we sometimes get a segment
  fault. Then we added all NULL checking to make sure pointers could be accessed
  safely.

**Unit Tests: apps/queue_tester_example.c**
- We extended the original queue_tester_example.c to include more comprehesive 
  cases, and edge cases. Such as elements of queue could be a struct, FIFO order
  needs to be guaranteed, invalid input should return -1, `queue_length()` 
  should report correct element count in each queue operation step
- We also reused code provided in project2.html for some inspiration on makeing 
  `queue_iterate()` deletion resilient. We fixed a bug to support delete by 
  saving carefully the `prevNode` ptr before calling on iterate func after which
  a delete might happen and we would lose on updating the iterator.

**Reference**
- Single linked list in C: https://www.learn-c.org/en/Linked_lists

### libuthread/uthread.c
----------------------------
**Idea**
- Data structure: We have a `struct uthread_tcb` representing thread control 
  block, it's having context stack ptr for free memory, thread id for tracking 
  whether it's the main thread or child thread (main thread having thread if 0). 
  Also we have a data structure called `scheduler` for keeping info about current
  running thread, the single main tcb pointer, and three threads queues 
  1) one ready_queue for all runnable threads 
  2) and one blocked_queue for all blocked threads
  3) and one exited_queue for reclaim thread memeory before process exits
- Our main thread is using the default process's stack as it's stack space. It
  has while loop of checking whether ready_queue non empty and switch context 
  over to the first dequeued thread. If ready queue is empty, while loop would
  exit and then delete all exit queue's tcb to reclaim memeory.
- For child thread, they would be given a special stack space allocated in the
  default process's heap as there "execution stack", they have ucontext 
  including cpu register states and stack base pointer and top pointer etc, as 
  well as func ptr so tha context switch could happen easily.
- `uthread_yield`: Would put current child thread at tail of ready_queue. Then 
   would yield control back to main tcb executing for scheduling the oldest 
   thread in front of the ready_queue
- `uthread_exit`: Put current child thread tcb in exited_queue, and switch cxt
  to main thread.
- `uthread_create`: allocated new stack from process's heap space, link it's 
  stack ponter and func pointer, and add new thread to scheduler's ready_queue 
- `uthread_run`: logic of the main thread. while loop to fetch next ready thread
  and reclaim mem for exited tcb before process exits.
- `uthread_block`: put current tcb in blocked_queue of the scheduler
- `utherad_unblocked`: put tcb from blocked_queue to end of ready_queue for main
  thread to pick it up next round.

**Chanllenge**
- The biggest chanllenge is understanding how the ucontext lib works, and where
  the heap and stack are located for main and childs thread. It was hard at the
  beginning, but the code in `context.c` provided by professor helped a lot.
- One bug that frustrated us a lot was that original in our `tcb_init()` and
  `scheduler_init()` we passed in a pointer by value! That resulted in our tcb
  and scheduler struct data structure not properly initialized. We had to refer
  to "GDB" discussion slides to locate where the "segment fault" error was
  triggered and did a lot of printf along the way to find out the issue.

**Test**
- Tests were done by uploading all source files on to `pc42.cs.ucdavis.edu`. As
  tests could not be done locally, we think Mac might not support the ucontext
  syscall. `apps/Makefile` was used to generate two executable `uthread_hello.x`
  and `uthread_yield.x`. We ran those .x files and expected output were observed

**Reference**
- To understand ucontext library and user level thread implementation:
    - https://stackoverflow.com/questions/9145321/implementing-a-user-level-threads-package
    - http://albertnetymk.github.io/2014/12/05/context/
    - https://www.gnu.org/software/libc/manual/html_mono/libc.html#System-V-contexts

### libuthread/sem.c
----------------------------
**Idea**
- Semaphore: contains two fields 1) counter for holding the current semaphore 
  value 2) a queue of threads that are waiting on the counter value to satisfy 
  certain conditions to be blocked or unblocked. Preempt were disabled during
  atomic operations.

**Challenge**
- It was not easy for us to come up with the idea until we search on this doc
  https://www.geeksforgeeks.org/semaphores-in-process-synchronization/, which
  inspired us on using the data strucutre mentioned above.
- We need to carefully think about where to put the preempt_enable/disable to
  make sure certain operations are non-interruptable.
- `uthread_current()` `uthread_block` `uthread_unblock` are our friends in 
  implementing the sem_down() and sem_up() function. about putting a blocked
  thread off ready_queue() when sem->count < 0 and wake them up accordingly 
  when sem->count<=0.

**Test**
- Tests were done by uploading all source files on to `pc42.cs.ucdavis.edu`. We
  modified `apps/Makefile` to include all `sem_*.x` lis the `programs` list
  and run all `sem_*.x` to check semephore logics.

**Reference**
- The following doc is used to understand how to implement semaphore
    - https://www.geeksforgeeks.org/semaphores-in-process-synchronization/

### libuthread/preempt.c and test_preempt.c
----------------------------
**Idea**
- For 100HZ timer, we used two itimerval 1) enable_timer to set a 100HZ to fire
  signal 2) disable_timer to disable the timer by setting corresponding values
- For signal hanlder, we refer back to system call slides provided by professor
  and it helped a lot.
- For blocking and unblocking a singal, we get inspired also from "system call"
  slide and looked up usage on `sigprocmask` in manual pages.

**Challenge**
- Challenge was to understand how to set up and disable timer, the list in the
  reference section really helped a lot to understand how to use the related
  lib functions.

**Test**
- Our test consists of two child threads and a global variable FLAG initialized
  to be 1. thread1 would have an infinite loop checking on this FLAG condition,
  then print out "thread1" loop ended. Thread2 would set the FLAG to 0, then
  print "thread2". If preempt works, thread1's infinite loops would be paused 
  and give thread2 the chance to reset FLAG to zero, causing both threads to 
  complete and we should see "thread2" and "thread1" printed out in order. 
  If preempt set to false, then the code in thread1 would hang with no output
  We run the code and behaviors are expected.

**Reference**
- The following links are used for our understanding of lib and use them.
    - https://www.gnu.org/software/libc/manual/html_mono/libc.html#Signal-Actions
    - https://www.gnu.org/software/libc/manual/html_mono/libc.html#Setting-an-Alarm
    - https://man7.org/linux/man-pages/man2/timer_settime.2.html
    - https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-Signals
    - https://www.informit.com/articles/article.aspx?p=23618&seqNum=14
    - Lecture system call page 41 signal 's code'
