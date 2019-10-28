#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

#define THREAD_CNT 4
#define THREAD_TOTAL 8

// waste some time
void *count(void *arg) {
	unsigned long int c = (unsigned long int)arg;
	int i;
	for (i = 0; i < c; i++) {
		if ((i % 100000) == 0) {
			printf("tid: 0x%x Just counted to %d of %ld\n", (unsigned int)pthread_self(), i, c);
		}
	}
    return arg;
}

void *stringCount(void *arg) {
	unsigned long int c = 10000000*3;
	int i;
	for (i = 0; i < c; i++) {
		if ((i % 100000) == 0) {
			printf("tid: 0x%x Just counted to %d of %ld\n", (unsigned int)pthread_self(), i, c);
		}
	}
    return arg;
}

int main(int argc, char **argv) {
	pthread_t threads[THREAD_TOTAL];
	int i;
	unsigned long int cnt = 10000000;
	char *str = "It's working!!";

    //create THREAD_CNT threads
	for(i = 0; i<THREAD_CNT; i++) {
		pthread_create(&threads[i], NULL, count, (void *)((i+1)*cnt));
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
    count((void *)(cnt*(THREAD_CNT + 1)));
    return 0;
}
