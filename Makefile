#Compile the dining philosophers binary

dining: gtthread_sched.o gtthread_mutex.o dining_main.o chopsticks.o steque.o philosopher.o 
	gcc -Wall  gtthread_sched.o gtthread_mutex.o dining_main.o chopsticks.o philosopher.o steque.o -o dining
steque.o: steque.c steque.h
	gcc -Wall -c steque.c
chopsticks.o: chopsticks.c philosopher.h chopsticks.h gtthread.h
	gcc -Wall -c chopsticks.c
dining_main.o: dining_main.c chopsticks.h philosopher.h gtthread.h
	gcc -Wall -c dining_main.c 
philosopher.o: philosopher.c philosopher.h
	gcc -Wall -c philosopher.c

#Compile the static library

libgtthread.a: gtthread_sched.o gtthread_mutex.o
	ar -rcs libgtthread.a gtthread_sched.o gtthread_mutex.o
gtthread_sched.o:gtthread_sched.c gtthread.h
	gcc -Wall -c gtthread_sched.c
gtthread_mutex.o:gtthread_mutex.c gtthread.h
	gcc -Wall -c gtthread_mutex.c

#Clean the directory

clean:
	rm *.o
