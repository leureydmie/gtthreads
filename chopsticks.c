#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include "gtthread.h"
#include "chopsticks.h"
#include "philosopher.h"


static chopstick chopsticks[5];
/*
 * Performs necessary initialization of mutexes.
 */
void chopsticks_init()
{
	int i;
	for(i=0;i<5;i++)
	{
		gtthread_mutex_init(&(chopsticks[i].mutex));
        chopsticks[i].state = Dirty;
		chopsticks[i].owner = -1;
		chopsticks[i].ask = -1;
	}
}

/*
 * Cleans up mutex resources.
 */
void chopsticks_destroy(){
    int i;
	for(i=0;i<5;i++)
	{
		gtthread_mutex_destroy(&chopsticks[i].mutex);
	}
}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */
void pickup_chopsticks(int phil_id){
    int right_neighbour = (phil_id + 1) % 5;
    int left_neighbour = (phil_id + 4) % 5;
    /*
     * By default, we give the chopstick to the philosopher who has the smallest index
     * among those who can take it
     */
    gtthread_mutex_lock(&chopsticks[phil_id].mutex);
    gtthread_mutex_lock(&chopsticks[right_neighbour].mutex);

    while((chopsticks[right_neighbour].owner != phil_id) || (chopsticks[phil_id].owner != phil_id))
    {
        gtthread_mutex_unlock(&(chopsticks[phil_id].mutex));
        gtthread_mutex_unlock(&(chopsticks[right_neighbour].mutex));

        gtthread_mutex_lock(&chopsticks[right_neighbour].mutex);
            if ((phil_id < right_neighbour) && (chopsticks[right_neighbour].owner == -1) && (chopsticks[right_neighbour].ask == -1))
            {
                pickup_right_chopstick(phil_id);
                chopsticks[right_neighbour].owner = phil_id;
                chopsticks[right_neighbour].ask = -1;
            }
            else if (chopsticks[right_neighbour].owner == -1 && chopsticks[right_neighbour].ask == phil_id)
            {
                pickup_right_chopstick(phil_id);
                chopsticks[right_neighbour].owner = phil_id;
                chopsticks[right_neighbour].ask = -1;
            }
            else if (chopsticks[right_neighbour].owner == phil_id && chopsticks[right_neighbour].ask != -1 && chopsticks[right_neighbour].state == Dirty)
            {
                putdown_right_chopstick(phil_id);
                chopsticks[right_neighbour].owner = -1;
                chopsticks[right_neighbour].state = Clean;
            }
            else if (chopsticks[right_neighbour].owner != phil_id && chopsticks[right_neighbour].ask == -1)
            {
                chopsticks[right_neighbour].ask = phil_id;
            }
        gtthread_mutex_unlock(&chopsticks[right_neighbour].mutex);

        gtthread_mutex_lock(&chopsticks[phil_id].mutex);
            if ((phil_id < left_neighbour) && (chopsticks[phil_id].owner == -1) && (chopsticks[phil_id].ask == -1))
            {
                pickup_left_chopstick(phil_id);
                chopsticks[phil_id].owner = phil_id;
                chopsticks[phil_id].ask = -1;
            }
            else if (chopsticks[phil_id].owner == -1 && chopsticks[phil_id].ask == phil_id)
            {
                pickup_left_chopstick(phil_id);
                chopsticks[phil_id].owner = phil_id;
                chopsticks[phil_id].ask = -1;
            }
            else if (chopsticks[phil_id].owner == phil_id && chopsticks[phil_id].ask != -1 && chopsticks[phil_id].state == Dirty)
            {
                putdown_left_chopstick(phil_id);
                chopsticks[phil_id].owner = -1;
                chopsticks[phil_id].state = Clean;
            }
            else if (chopsticks[phil_id].owner != phil_id && chopsticks[phil_id].ask == -1)
            {
                chopsticks[phil_id].ask = phil_id;
            }
        gtthread_mutex_unlock(&chopsticks[phil_id].mutex);

        gtthread_yield();

        gtthread_mutex_lock(&chopsticks[phil_id].mutex);
        gtthread_mutex_lock(&chopsticks[right_neighbour].mutex);
    }

    gtthread_mutex_unlock(&chopsticks[phil_id].mutex);
    gtthread_mutex_unlock(&chopsticks[right_neighbour].mutex);
}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */
void putdown_chopsticks(int phil_id)
{
    int right_neighbour = (phil_id + 1) % 5;

    gtthread_mutex_lock(&chopsticks[right_neighbour].mutex);
        putdown_right_chopstick(phil_id);
        chopsticks[right_neighbour].owner = -1;
        chopsticks[right_neighbour].state = Dirty;
    gtthread_mutex_unlock(&chopsticks[right_neighbour].mutex);

    gtthread_mutex_lock(&chopsticks[phil_id].mutex);
        putdown_left_chopstick(phil_id);
        chopsticks[phil_id].owner = -1;
        chopsticks[phil_id].state = Dirty;
    gtthread_mutex_unlock(&chopsticks[phil_id].mutex);
}
