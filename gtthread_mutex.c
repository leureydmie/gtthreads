/**********************************************************************
gtthread_mutex.c.

This file contains the implementation of the mutex subset of the
gtthreads library.  The locks can be implemented with a simple queue.
 **********************************************************************/

/*
  Include as needed
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "gtthread.h"
/*
  The gtthread_mutex_init() function is analogous to
  pthread_mutex_init with the default parameters enforced.
  There is no need to create a static initializer analogous to
  PTHREAD_MUTEX_INITIALIZER.
 */
int gtthread_mutex_init(gtthread_mutex_t* mutex){
    //Need to block the signals here
    sigset_t mask;
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    gtthread_mutex_t* new_mutex = mutex;

    //Set mutex attributes
    new_mutex->owner = gtthread_self().id;
    new_mutex->value = 0;

    sigprocmask(SIG_UNBLOCK,&mask,NULL);
    return 0;
}

/*
  The gtthread_mutex_lock() is analogous to pthread_mutex_lock.
  Returns zero on success.
 */
int gtthread_mutex_lock(gtthread_mutex_t* mutex){
    //Need to block the signals here
    sigset_t mask;
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    //Loops while neither the mutex is locked nor the thread owns the lock.
    while(mutex->value == 1 && mutex->owner != gtthread_self().id)
    {
        //Unblock the signals before yield
        sigprocmask(SIG_BLOCK, &mask, NULL);
        gtthread_yield();
        //Re-block the signals before resume looping
        sigprocmask(SIG_BLOCK, &mask, NULL);
    }
    mutex->value = 1;
    mutex->owner = gtthread_self().id;
    //Unblock the signals
    sigprocmask(SIG_UNBLOCK,&mask,NULL);
    return 0;
}

/*
  The gtthread_mutex_unlock() is analogous to pthread_mutex_unlock.
  Returns zero on success.
 */
int gtthread_mutex_unlock(gtthread_mutex_t *mutex)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    //Block signals before reading & writing mutex
    sigprocmask(SIG_BLOCK, &mask, NULL);
    if(mutex->owner != gtthread_self().id)
    {
        sigprocmask(SIG_UNBLOCK,&mask,NULL);
        return -1;
    }
    else if (mutex->value == 0)
    {
        sigprocmask(SIG_UNBLOCK,&mask,NULL);
        return -1;
    }
    else
    {
        mutex->value = 0;
        sigprocmask(SIG_UNBLOCK,&mask,NULL);
        return 0;
    }
}

/*
  The gtthread_mutex_destroy() function is analogous to
  pthread_mutex_destroy and frees any resourcs associated with the mutex.
*/
int gtthread_mutex_destroy(gtthread_mutex_t *mutex){
    return 0;
}

