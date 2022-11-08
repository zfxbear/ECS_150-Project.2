// References:
// https://www.gnu.org/software/libc/manual/html_mono/libc.html#Signal-Actions
// https://www.gnu.org/software/libc/manual/html_mono/libc.html#Setting-an-Alarm
// https://man7.org/linux/man-pages/man2/timer_settime.2.html
// https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-Signals
// https://www.informit.com/articles/article.aspx?p=23618&seqNum=14
// Lecture system call page 41 signal 's code'

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100      // 100Hz is 100 times per seconds
#define SEC 1000000 // secon in micro-seconds
static int ALARM_INTERVAL = SEC/HZ; // in micro-seconds
static struct itimerval enable_timer, disable_timer;

/* Signal set for enable/disable the SIGVTALRM singal */
static sigset_t timer_signal_set;

/* Signal handler */
static struct sigaction sa;

/* Initalize all the static signal set timer */
void init_timers(void)
{
    // Set enable_timer to 100HZ
    // first round timer alarm count down time
    enable_timer.it_value.tv_sec = 0; // 0 seconds
    enable_timer.it_value.tv_usec = ALARM_INTERVAL; // in microseconds
    // when first round timer alarm count down to 0, how to top them back up
    enable_timer.it_interval.tv_sec = 0; // top up it_value.tv_sec to 0
    enable_timer.it_interval.tv_usec = ALARM_INTERVAL; // top up it_value.tv_usec

    // Set disable timer to 0
    disable_timer.it_value.tv_sec = 0; // 0 seconds
    disable_timer.it_value.tv_usec = 0; // 0 microseconds
    // do not top up when it_value count down to zero
    disable_timer.it_interval.tv_sec = 0;
    disable_timer.it_interval.tv_usec = 0;

    // Set the timer_signal_set
    sigemptyset(&timer_signal_set); // empty set first
    sigaddset(&timer_signal_set, SIGVTALRM); // add SIGVTALRM to the empty set
}

/* Handler is to let the current running thread yield */
void timer_handler()
{
    uthread_yield();
}

/* Disable by blocking SIGVTALRM signal set */
void preempt_disable(void)
{
    sigprocmask(SIG_BLOCK, &timer_signal_set, NULL);
}

/* Enable by unblocking SIGVTALRM signal set */
void preempt_enable(void)
{
    sigprocmask(SIG_UNBLOCK, &timer_signal_set, NULL);
}

/* Start the preempt by*/
void preempt_start(bool preempt)
{
    if (preempt) {
        // init data structure
        init_timers();
        // set handler
        sa.sa_handler = timer_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGVTALRM, &sa, NULL);
        // set timer and start it
        setitimer(ITIMER_VIRTUAL, &enable_timer, NULL);
    }
}

void preempt_stop(void)
{
    // clear timer
    setitimer(ITIMER_VIRTUAL, &disable_timer, NULL);
}

