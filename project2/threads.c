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

	processThreads[0].id = 0;
	processThreads[0].state = READY;
	processThreads[0].rsp = NULL;
	setjmp(processThreads[0].reg);
	activeThreads++;
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
			processThreads[activeThreads].rsp = malloc(32767);
			setjmp(processThreads[activeThreads].reg);

			// manually set start routine
			processThreads[activeThreads].reg[0].__jmpbuf[JB_PC] = ptr_mangle((unsigned long) start_thunk);
			processThreads[activeThreads].reg[0].__jmpbuf[JB_R13] = ptr_mangle((unsigned long) arg);
			processThreads[activeThreads].reg[0].__jmpbuf[JB_R12] = ptr_mangle((unsigned long) start_routine);

			activeThreads++;
		}

    return 0;
}

void pthread_exit(void *value_ptr) {
  exit(0);
}

pthread_t pthread_self(void) {
  pthread_t foo = NULL;
  return foo;
}
