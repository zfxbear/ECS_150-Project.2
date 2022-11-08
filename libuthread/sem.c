// Reference: 
// The following doc is used to understand how to implement semaphore
// https://www.geeksforgeeks.org/semaphores-in-process-synchronization/

#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore {
    int count;
    queue_t thread_queue;
};
typedef struct semaphore semaphore;

typedef struct uthread_tcb* uthread_tcb_t;

sem_t sem_create(size_t count)
{
    preempt_disable();
    sem_t sem = malloc(sizeof(semaphore));
    sem->count = count;
    sem->thread_queue = queue_create();
    preempt_enable();
    return sem;
}

int sem_destroy(sem_t sem)
{
    preempt_disable();
    queue_destroy(sem->thread_queue);
    free(sem);
    preempt_enable();
    return 0;
}

int sem_down(sem_t sem)
{
    preempt_disable();
    sem->count--;
    if (sem->count < 0) {
        queue_enqueue(sem->thread_queue, uthread_current());
        uthread_block();
    }
    return 0;
}

int sem_up(sem_t sem)
{
    preempt_disable();
    sem->count++;
    if (sem->count <= 0) {
        uthread_tcb_t wake_up_thread;
        queue_dequeue(sem->thread_queue, (void**)&wake_up_thread);
        uthread_unblock(wake_up_thread);
    }
    preempt_enable();
    return 0;
}
