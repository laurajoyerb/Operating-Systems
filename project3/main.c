#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include "semaphore.h"

#define THREAD_CNT 4
#define THREAD_TOTAL 8

int number = 0;

int sem_init(sem_t *sem, int pshared, unsigned value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_destroy(sem_t *sem);

// waste some time
void *count(void *arg) {
	unsigned long int c = 10000000; //(unsigned long int)arg;
	int i;
	sem_wait(arg);
	for (i = 0; i < c; i++) {
		number++;
		if ((i % 100000) == 0) {
			printf("tid: 0x%x incremented number to %d\n", (unsigned int)pthread_self(), number);
		}
	}
	sem_post(arg);
  return arg;
}

void *stringCount(void *arg) {
	unsigned long int c = 10000000*3;
	int i;
	for (i = 0; i < c; i++) {
		if ((i % 100000) == 0) {
			printf("tid: 0x%x Doing string things\n", (unsigned int)pthread_self());
		}
	}
	printf("Thread %d: I'm done now\n", (unsigned int)pthread_self());
  return arg;
}

int main(int argc, char **argv) {
	pthread_t threads[THREAD_TOTAL];
	int i;
	char *str = "It's working!!";

	sem_t newSem;
  sem_init(&newSem, 0, 1);

    //create THREAD_CNT threads
	for(i = 0; i<THREAD_CNT; i++) {
		pthread_create(&threads[i], NULL, count, &newSem);
	}

	for (i = THREAD_CNT; i < THREAD_TOTAL; i++) {
		pthread_create(&threads[i], NULL, stringCount, (void *)(str));
	}

		void* thread_value;

		for(i = 0; i<THREAD_TOTAL; i++) {
			pthread_join(threads[i], &thread_value);
		}
    // But we have to make sure that main does not return before
    // the threads are done... so count some more...
    count(&newSem);
    return 0;
}
