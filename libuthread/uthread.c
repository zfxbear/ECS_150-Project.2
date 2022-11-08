// Reference
// - Understand user-level threads: https://stackoverflow.com/questions/9145321/implementing-a-user-level-threads-package
// - To understand how ucontext work: http://albertnetymk.github.io/2014/12/05/context/
// https://www.gnu.org/software/libc/manual/html_mono/libc.html#System-V-contexts
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

/* Thread state */
enum uthread_state {
    READY,   // runnable, waiting for time slot
    RUNNING, // currently running thread
    BLOCKED, // blocked after thread_block. if unblocked would be moved to READY
    EXITED   // thread already completed. scheduler will reclaim metadata
};
typedef enum uthread_state uthread_state;

/* TCB for holding thread information */
struct uthread_tcb {
    int thread_id;       // UUID of current thread
    uthread_state state; // current state, used by scheudler
    uthread_ctx_t *ctx;  // threads context
    void *stk_ptr;       // top of stack for thread execution
};
typedef struct uthread_tcb uthread_tcb;
typedef struct uthread_tcb* uthread_tcb_t;

/* Scheduler information */
struct scheduler {
    int thread_counter;     // next available thread ID for new thread
    uthread_tcb_t cur_tcb;  // current running thread
    uthread_tcb_t main_tcb; // where scheduler logic is located
                            // main_tcb would be run on process default stack
    queue_t ready_queue;    // threads ready queue
    queue_t blocked_queue;  // threads blocked queue
    queue_t exited_queue;   // thread exited queeu
};
typedef struct scheduler* scheduler_t;
// declare a global scheduler
struct scheduler sched;
scheduler_t scheduler = &sched;

/* TCB initializer */
// Note: here first round i discover that need to pass by pointer instead of value
// causing my tcb not properly 
int tcb_init(uthread_tcb_t *tcb_t_ptr, int thread_id, uthread_state thread_state,
             uthread_func_t func, void *arg)
{
    *tcb_t_ptr = malloc(sizeof(uthread_tcb));
    if (*tcb_t_ptr == NULL) {
        return -1;
    }
    (*tcb_t_ptr)->thread_id = thread_id;
    (*tcb_t_ptr)->state = thread_state;
    (*tcb_t_ptr)->ctx = malloc(sizeof(uthread_ctx_t));
    if (thread_id == 0) {
        // for main-tcb, just grab the current context
        getcontext((*tcb_t_ptr)->ctx);
        // stack is NULL for main, as it's using process's default stack
        (*tcb_t_ptr)->stk_ptr = NULL;
    } else {
        // for non main-tcb threads, set tcb context to have it's own stack/func
        void *new_stack = uthread_ctx_alloc_stack();
        uthread_ctx_init((*tcb_t_ptr)->ctx, new_stack, func, arg);
        // recored stack pointer so as to reclaim mem in destroy
        (*tcb_t_ptr)->stk_ptr = new_stack;
    }
    return 0;
}

/* Scheduler initializer */
// With the assumption that there would only be one scheduler in the app
int scheduler_init(void)
{
    scheduler->thread_counter = 1; // non main threads start from thread_id 1
    tcb_init(&scheduler->main_tcb, 0, RUNNING, NULL, NULL);
    scheduler->cur_tcb = scheduler->main_tcb; // current running tcb
    scheduler->ready_queue = queue_create(); // holding all ready to run threads
    scheduler->blocked_queue = queue_create(); // all blocked threads
    scheduler->exited_queue = queue_create(); // all exited threads for reclaim
    return 0;
}

/* Return current running thread */
struct uthread_tcb *uthread_current(void)
{
    return scheduler->cur_tcb;
}

/* Capture current context and save in tcb, then resume to main_context */
void uthread_yield(void)
{
    // only yield to main if current thread is child thread
    if (scheduler->cur_tcb->thread_id != 0) {
        // put current tcb in ready queue
        preempt_disable();
        uthread_tcb_t yielding_thread = scheduler->cur_tcb;
        yielding_thread->state = READY;
        queue_enqueue(scheduler->ready_queue, yielding_thread);
        // update scheduler info
        scheduler->main_tcb->state = RUNNING;
        scheduler->cur_tcb = scheduler->main_tcb;
        // swtich context, save current context to yielding_thread, switch to main
        uthread_ctx_switch(yielding_thread->ctx, scheduler->main_tcb->ctx);
    }
}

/* Mark self state as exit, then yield control back to main */
void uthread_exit(void)
{
    preempt_disable();
    // update current running thread state to EXITED
    uthread_tcb_t exiting_thread = scheduler->cur_tcb;
    exiting_thread->state = EXITED;
    // update scheduler info
    int ret = queue_enqueue(scheduler->exited_queue, scheduler->cur_tcb);
    if (ret == -1) {
        perror("unable to put exited thread in zombie queue");
        exit(1);
    }
    scheduler->main_tcb->state = RUNNING;
    scheduler->cur_tcb = scheduler->main_tcb;
    // swtich context, save current context (won't be used), and switch to main
    uthread_ctx_switch(exiting_thread->ctx, scheduler->main_tcb->ctx);
}

/* Create a new tcb, and add it to scheduler's ready queue */
int uthread_create(uthread_func_t func, void *arg)
{
    preempt_disable();
    // create a new tcb
    uthread_tcb_t new_thread = NULL;
    tcb_init(&new_thread, scheduler->thread_counter, READY, func, arg);
    // put it in scheduler
    int ret = queue_enqueue(scheduler->ready_queue, new_thread);
    if (ret == -1) {
        perror("unable to put newly created thread in ready queue");
        exit(1);
    }
    scheduler->thread_counter += 1; // increase next available thread_id
    preempt_enable();
    return 0;
}

/* Initialize scheduler, run a while loop for all threads */
int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    if (preempt) {
        preempt_start(preempt);
    }
    // initialize scheduler
    scheduler_init();
    // create first thread
    uthread_create(func, arg);
    // loop until no more ready threads
    while (queue_length(scheduler->ready_queue) > 0) {
        uthread_tcb_t next_runnable_thread;
        queue_dequeue(scheduler->ready_queue, (void**)&next_runnable_thread);
        next_runnable_thread->state = RUNNING;
        scheduler->cur_tcb = next_runnable_thread;
        scheduler->main_tcb->state = READY;
        // enable timer disruppt and switch to child thread
        preempt_enable();
        uthread_ctx_switch(scheduler->main_tcb->ctx, scheduler->cur_tcb->ctx);
    }
    // at end of loop, reclaim all exited tbreads
    uthread_tcb_t tcb;
    while(queue_length(scheduler->exited_queue) > 0) {
        queue_dequeue(scheduler->exited_queue, (void**)&tcb);
        uthread_ctx_destroy_stack(tcb->stk_ptr); // free stack
        free(tcb->ctx);
        free(tcb);
    }
    // reclaim tcb allocated for main scheduler thread (reverse schedulerinit)
    free(scheduler->main_tcb->ctx);
    free(scheduler->main_tcb);
    // disable preempt
    if (preempt) {
        preempt_stop();
    }
    return 0;
}

/* Block current thread and return to main for scheduler logic */
void uthread_block(void)
{
    preempt_disable();
    uthread_tcb_t blocked_thread = scheduler->cur_tcb;
    blocked_thread->state = BLOCKED;
    queue_enqueue(scheduler->blocked_queue, blocked_thread);
    scheduler->main_tcb->state = RUNNING;
    scheduler->cur_tcb = scheduler->main_tcb;
    uthread_ctx_switch(blocked_thread->ctx, scheduler->main_tcb->ctx);
}

/* Unblock thread by deleting from blocked_queue to ready_queue */
void uthread_unblock(struct uthread_tcb *uthread)
{
    preempt_disable();
    uthread_tcb_t unblocked_thread = uthread;
    unblocked_thread->state = READY;
    queue_delete(scheduler->blocked_queue, (void*)unblocked_thread);
    queue_enqueue(scheduler->ready_queue, (void*)unblocked_thread);
    preempt_enable();
}
