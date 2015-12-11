
/**********************************************************************
gtthread_sched.c.

This file contains the implementation of the scheduling subset of the
gtthreads library.  A simple round-robin queue should be used.
 **********************************************************************/
/*
  Include as needed
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "gtthread.h"
#include <unistd.h>
/*
   Students should define global variables and helper functions as
   they see fit.
 */
void timer_handler(int);
void gtthread_launch(void *(*start_routine)(void *), void *arg);
static steque_t thread_round_robin;
static gtthread_t* thread_main;
static struct itimerval timer;
static struct sigaction sa;
static int nb_threads;

// Definition des signaux

void timer_handler(int signo)
{
    if (signo == 0) exit(-1);
    gtthread_yield();
}

void gtthread_launch(void *(*start_routine)(void *), void *arg)
{
    void* return_value;
    return_value = start_routine(arg);
    gtthread_exit(return_value);
}


/*
  The gtthread_init() function does not have a corresponding pthread equivalent.
  It must be called from the main thread before any other GTThreads
  functions are called. It allows the caller to specify the scheduling
  period (quantum in micro second), and may also perform any other
  necessary initialization.

  Recall that the initial thread of the program (i.e. the one running
  main() ) is a thread like any other. It should have a
  gtthread_t that clients can retrieve by calling gtthread_self()
  from the initial thread, and they should be able to specify it as an
  argument to other GTThreads functions. The only difference in the
  initial thread is how it behaves when it executes a return
  instruction. You can find details on this difference in the man page
  for pthread_create.
 */
void gtthread_init(long period){
    /*
    * Main thread initialisation
    */
    steque_init(&thread_round_robin);
    nb_threads = 1;
    thread_main = malloc(sizeof(gtthread_t));
    thread_main->id = 0;
    thread_main->joinable = 1;
    thread_main->suspended = false;
    if(getcontext(&(thread_main->uctx))==-1)
    {
        perror("getcontext error");
        exit(-1);
    }
    thread_main->uctx.uc_link = NULL;
    steque_enqueue(&thread_round_robin, (steque_item) thread_main);

     if(period >= 0)
    {
        /*
        * Setting timer
        */
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = period;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = period;
        setitimer (ITIMER_VIRTUAL, &timer, NULL);

        /*
        * Setting signal handler
        */
        memset (&sa, 0, sizeof (sa));
        sa.sa_handler = &timer_handler;
        sigaction (SIGVTALRM, &sa, NULL);
    }
}


/*
  The gtthread_create() function mirrors the pthread_create() function,
  only default attributes are always assumed.
 */
int gtthread_create(gtthread_t *thread,
		    void *(*start_routine)(void *),
		    void *arg){
    sigset_t mask;

    //memset(thread, 0, sizeof(*thread));

    /*
    * Critical section
    */

    gtthread_t* t = thread;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);

	if(getcontext(&(t->uctx))==-1)
	{
        perror("getcontext error");
        exit(-1);
    }

    sigprocmask(SIG_BLOCK, &mask, NULL);
    t->id = nb_threads;
    nb_threads++;
    t->joinable =  true;
    t->suspended = false;
    t->uctx.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
    t->uctx.uc_stack.ss_size = SIGSTKSZ;
    t->uctx.uc_link = &(thread_main->uctx);
    makecontext(&(t->uctx), (void*) gtthread_launch, 2, (void*) start_routine, arg);
    steque_enqueue(&thread_round_robin, (steque_item) t);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    return 0;
}

/*
  The gtthread_join() function is analogous to pthread_join.
  All gtthreads are joinable.
 */
int gtthread_join(gtthread_t thread, void **status)
{
    sigset_t mask;
    gtthread_t this_thread = gtthread_self();
    gtthread_t* t_to_join;
    int count = 0;
    /*
    * Critical section
    */
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);

        while (((gtthread_t*) steque_front(&thread_round_robin))->id != thread.id
                 && ++count <= steque_size(&thread_round_robin)) steque_cycle(&thread_round_robin);
        if(count >= steque_size(&thread_round_robin))
        {
            perror("Attempting to join a thread that does not exist");
            return -1;
        }
        t_to_join = (gtthread_t*) steque_front(&thread_round_robin);
        while (((gtthread_t*) steque_front(&thread_round_robin))->id != this_thread.id) steque_cycle(&thread_round_robin);

    if (thread.id == this_thread.id)
    {
        perror("Attempting to join the current thread");
        return -1;
    }
    if (t_to_join->joinable == false)
    {
        perror("Thread is not joinable");
        return -1;
    }
    else t_to_join->joinable = false;

    while(t_to_join->suspended == false)
    {
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        gtthread_yield();
        sigprocmask(SIG_BLOCK, &mask, NULL);
    }
    if (t_to_join->return_value != NULL && status != NULL)
    {
        *status = t_to_join->return_value;
    }
    gtthread_cancel(thread);
    sigprocmask(SIG_UNBLOCK,&mask,NULL);

    return 0;
}

/*
  The gtthread_exit() function is analogous to pthread_exit.
 */
void gtthread_exit(void* retval){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    if((steque_isempty(&thread_round_robin)))
    {
        perror("Queue empty");
        exit(-1);
    }

    gtthread_t* this_thread_ptr = (gtthread_t*) steque_front(&thread_round_robin);
    this_thread_ptr->suspended = true;
    this_thread_ptr->return_value = retval;
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    while(1) gtthread_yield();
}


/*
  The gtthread_yield() function is analogous to pthread_yield, causing
  the calling thread to relinquish the cpu and place itself at the
  back of the schedule queue.
 */
void gtthread_yield(void){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    if(steque_size(&thread_round_robin)>1)
    {
        steque_cycle(&thread_round_robin);
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        swapcontext(&(((gtthread_t*) (thread_round_robin.back->item))->uctx),
                    &(((gtthread_t*) steque_front(&thread_round_robin))->uctx)
                    );
    }
}

/*
  The gtthread_yield() function is analogous to pthread_equal,
  returning zero if the threads are the same and non-zero otherwise.
 */
int  gtthread_equal(gtthread_t t1, gtthread_t t2){
    return (t1.id == t2.id);
}

/*
  The gtthread_cancel() function is analogous to pthread_cancel,
  allowing one thread to terminate another asynchronously.
 */
int  gtthread_cancel(gtthread_t thread){
    if((steque_isempty(&thread_round_robin)))
    {
        perror("Queue empty");
        return -1;
    }
    gtthread_t this_thread = gtthread_self();
    int count = 0;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
        while (((gtthread_t*) steque_front(&thread_round_robin))->id != thread.id
                 && ++count < steque_size(&thread_round_robin)) steque_cycle(&thread_round_robin);
        if(count >= steque_size(&thread_round_robin))
        {
            perror("Attempting to destroy a thread that does not exist");
            return -1;
        }
        steque_pop(&thread_round_robin);
        while (((gtthread_t*) steque_front(&thread_round_robin))->id != this_thread.id) steque_cycle(&thread_round_robin);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    return 0;
}

/*
  Returns calling thread.
 */
gtthread_t gtthread_self(void){
    gtthread_t* return_value = steque_front(&thread_round_robin);
    return *return_value;
}
