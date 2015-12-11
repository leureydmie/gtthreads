#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "gtthread.h"


void* thr1(void *in) {
    int return_value = gtthread_self().id;
    gtthread_exit(&return_value);
}

int main() {
  gtthread_t t1;
  int ret=0;
  int * get_value = malloc(sizeof(int));
  gtthread_init(250000);
  gtthread_create(&t1, thr1, NULL);
  printf("Le thread %d est cree, %d, %d\n\n", t1.id, t1.suspended, t1.joinable);
  gtthread_join(t1, NULL);
  printf("Main a recupere la valeur %d\n\n", *get_value);
  printf("Fin du main\n\n");
  //while(1);
  gtthread_exit(&ret);
}
