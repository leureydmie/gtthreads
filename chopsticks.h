#ifndef _CHOPSTICKS_H_
#define _CHOPSTICKS_H_

typedef struct
{
    gtthread_mutex_t mutex;
    int owner;
    int ask;
    enum {Clean, Dirty} state;
} chopstick;

void chopsticks_init(void);

void chopsticks_destroy(void);

void pickup_chopsticks(int phil_id);

void putdown_chopsticks(int phil_id);

#endif
