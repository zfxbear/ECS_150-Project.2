/*
 * Preempt Enable
 *
 * Expected output is the following
 *
 * thread2
 * thread1
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

static int FLAG = 1;

void thread2(void *arg)
{
    (void)arg;
    FLAG = 0;
    printf("thread2\n");
}

// As line 31 is stuck threa1 is not self yielding 
// its preempted and then thread2 output "thread2" and set while lool
// flag to 0, thus thread 1 exit and print thread1
void thread1(void *arg)
{
    (void)arg;

    uthread_create(thread2, NULL);
    while(FLAG); // infinit loop
    printf("thread1\n");
}

int main(void)
{
    uthread_run(true, thread1, NULL); // note if preempt is passed in as fasle. process would stuck
    return 0;
}
