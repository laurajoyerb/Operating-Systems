#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdbool.h>
#include "ec440threads.h"

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

void initialize() {
	initialized = true;
	currentThread = 0;

	processThreads[0].id = 0;
	processThreads[0].state = READY;
	processThreads[0].rsp = NULL;
	setjmp(processThreads[0].reg);
	activeThreads++;
}

void schedule() {
	printf("scheduling\n");
	setjmp(processThreads[currentThread].reg); // saves previous threads stuff

	if (activeThreads > currentThread) {
		currentThread++;
		while (activeThreads > currentThread && processThreads[currentThread].state != READY) {
			currentThread++;
		}
		printf("found next thread, it's: %d\n", currentThread);

		longjmp(processThreads[currentThread].reg, 1); // jumps to next thread
	}
}

int pthread_create(
  pthread_t *thread,
  const pthread_attr_t *attr,
  void *(*start_routine) (void *),
  void *arg) {
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

			printf("RBX: 0x%08lx\nRBP: 0x%08lx\nR12: 0x%08lx\nR13: 0x%08lx\nR14: \
	0x%08lx\nR15: 0x%08lx\nSP:  0x%08lx\nPC:  0x%08lx\n",
	        processThreads[activeThreads].reg[0].__jmpbuf[JB_RBX], processThreads[activeThreads].reg[0].__jmpbuf[JB_RBP],
	        processThreads[activeThreads].reg[0].__jmpbuf[JB_R12], processThreads[activeThreads].reg[0].__jmpbuf[JB_R13],
	        processThreads[activeThreads].reg[0].__jmpbuf[JB_R14], processThreads[activeThreads].reg[0].__jmpbuf[JB_R15],
	        processThreads[activeThreads].reg[0].__jmpbuf[JB_RSP], processThreads[activeThreads].reg[0].__jmpbuf[JB_PC]);

	    printf("RSPd:0x%08lx\nPCd: 0x%08lx\n",
	        ptr_demangle(processThreads[activeThreads].reg[0].__jmpbuf[JB_RSP]),
	        ptr_demangle(processThreads[activeThreads].reg[0].__jmpbuf[JB_PC])
	        );

			activeThreads++;

			schedule();
		}

    return 0;
}

void pthread_exit(void *value_ptr) {
	printf("exiting\n");
	// processThreads[currentThread].state = EXITED;
	// processThreads[currentThread].rsp = NULL;
	// activeThreads--;
	// schedule();
  exit(0);
}

pthread_t pthread_self(void) {
  pthread_t foo = NULL;
  return foo;
}
