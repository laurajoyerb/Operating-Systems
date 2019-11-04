#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include "semaphore.h"

#define THREAD_CNT 4
#define THREAD_TOTAL 8

int number = 0;
int number2 = 0;

int sem_init(sem_t *sem, int pshared, unsigned value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_destroy(sem_t *sem);

sem_t mutex;
sem_t full;
sem_t empty;

// waste some time
// producer
void *count(void *arg) {
	unsigned long int c = 300; //(unsigned long int)arg;
	int i;
	for (i = 0; i < c; i++) {
		sem_wait(&empty);
		sem_wait(&mutex);
		number++;
			printf("tid: 0x%x incremented number to %d\n", (unsigned int)pthread_self(), number);
		sem_post(&mutex);
		sem_post(&full);
	}
  return arg;
}

// consumer
void *stringCount(void *arg) {
	unsigned long int c = 300;
	int i;
	for (i = 0; i < c; i++) {
		sem_wait(&full);
		sem_wait(&mutex);
		number--;
			printf("tid: 0x%x decremented number to %d\n", (unsigned int)pthread_self(), number);
		sem_post(&mutex);
		sem_post(&empty);
	}
	printf("Thread %d: I'm done now\n", (unsigned int)pthread_self());
  return arg;
}

int main(int argc, char **argv) {
	pthread_t threads[THREAD_TOTAL];
	int i;
	// char *str = "It's working!!";
	int n = 6;

  sem_init(&mutex, 0, 1);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, n);

    //create THREAD_CNT threads
	for(i = 0; i<THREAD_CNT; i++) {
		pthread_create(&threads[i], NULL, count, &mutex);
	}

	for (i = THREAD_CNT; i < THREAD_TOTAL; i++) {
		pthread_create(&threads[i], NULL, stringCount, &mutex);
	}

		void* thread_value;

		for(i = 0; i<THREAD_TOTAL; i++) {
			pthread_join(threads[i], &thread_value);
		}
    // But we have to make sure that main does not return before
    // the threads are done... so count some more...
    // count(&mutex);
		sem_destroy(&mutex);
		sem_destroy(&full);
		sem_destroy(&empty);
    return 0;
}
