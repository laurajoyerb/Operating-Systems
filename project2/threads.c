#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdbool.h>
#include "ec440threads.h"
#include <sys/time.h>
#include <signal.h>
#include <string.h>

#define MAX_THREADS 128

// states of threads
#define READY 0
#define RUNNING 1
#define EXITED 2

// registers of jump buf
#define JB_RBX 0
#define JB_RBP 1
#define JB_R12 2
#define JB_R13 3
#define JB_R14 4
#define JB_R15 5
#define JB_RSP 6
#define JB_PC 7

bool initialized = false;
int activeThreads = 0;
int currentThread;

struct thread {
	pthread_t id;
  jmp_buf reg;
  int state;
  int* rsp;
};

// array to hold all threads
struct thread processThreads[MAX_THREADS];

void schedule() {
	printf("scheduling\n");
	if (setjmp(processThreads[currentThread].reg) == 0) {
		printf("stopping thread %d, finding next one\n", currentThread);
		processThreads[currentThread].state = READY;
		if (activeThreads <= currentThread) {
			currentThread = 0;
		} else {
			currentThread++;
		}

		while (processThreads[currentThread].state != READY) {
			currentThread++;
			if (activeThreads > currentThread) {
				currentThread = 0;
			}
		}
		printf("found next thread, it's: %d\n", currentThread);

		longjmp(processThreads[currentThread].reg, 1); // jumps to next thread
	} else {
		printf("trying to start up thread %d\n", currentThread);
	}
}

void initialize() {
	initialized = true;
	currentThread = 0;

	processThreads[0].id = 0;
	processThreads[0].state = READY;
	processThreads[0].rsp = NULL;
	setjmp(processThreads[0].reg);
	activeThreads++;

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = schedule;
	sigaction(SIGALRM, &sa, NULL);
}

int pthread_create(
  pthread_t *thread,
  const pthread_attr_t *attr,
  void *(*start_routine) (void *),
  void *arg) {
		printf("creating a thread\n");
		if (!initialized) {
			initialize();
		}

		if (activeThreads < MAX_THREADS) {
			processThreads[activeThreads].id = *thread;
			processThreads[activeThreads].state = READY;
			int* bottom = malloc(32767);
			processThreads[activeThreads].rsp = bottom + 32767 - 8;
			*(processThreads[activeThreads].rsp) = (unsigned long) &pthread_exit;
			setjmp(processThreads[activeThreads].reg);

			// manually set start routine
			processThreads[activeThreads].reg[0].__jmpbuf[JB_PC] = ptr_mangle((unsigned long) start_thunk);
			processThreads[activeThreads].reg[0].__jmpbuf[JB_R13] = (unsigned long) arg;
			processThreads[activeThreads].reg[0].__jmpbuf[JB_R12] = (unsigned long) start_routine;
			processThreads[activeThreads].reg[0].__jmpbuf[JB_RSP] = ptr_mangle((unsigned long) processThreads[activeThreads].rsp);

			activeThreads++;

			schedule();
		}

    return 0;
}

void pthread_exit(void *value_ptr) {
	printf("exiting\n");
	processThreads[currentThread].state = EXITED;
	processThreads[currentThread].rsp = NULL;
	activeThreads--;
	schedule();
  __builtin_unreachable();
}

pthread_t pthread_self(void) {
	printf("returning thread id\n");
  return processThreads[currentThread].id;
}
