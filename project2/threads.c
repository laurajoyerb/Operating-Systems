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
	printf("check: scheduling\n");
	if (setjmp(processThreads[currentThread].reg) == 0) {
		printf("check: stopping thread %d, finding next one\n", currentThread);
		int i;
		 for (i = 0; i < MAX_THREADS; i++) {
		 		if (processThreads[i].state == EXITED) {
					printf("looks like thread %d has exited\n", i);
					free(processThreads[i].rsp);
				}
		 }
		if (activeThreads - 1 <= currentThread) {
			currentThread = 0;
		} else {
			currentThread++;
		}

		while (processThreads[currentThread].state != READY && processThreads[currentThread].state != RUNNING) {
			printf("check: it wasn't thread %d. Incrementing\n", currentThread);
			currentThread++;
			if (activeThreads >= currentThread) {
				printf("check: wrapping around back to thread 0. Active Threads: %d, Current Thread: %d\n", activeThreads, currentThread);
				currentThread = 0;
			}
		}
		printf("check: found next thread, it's: %d\n", currentThread);

		sigset_t ss;
		sigemptyset(&ss);
		sigaddset(&ss, SIGALRM);
		sigprocmask(SIG_UNBLOCK, &ss, NULL);

		longjmp(processThreads[currentThread].reg, 1); // jumps to next thread
	} else {
		printf("check: trying to start up thread %d\n", currentThread);
		sigset_t ss;
		sigemptyset(&ss);
		sigaddset(&ss, SIGALRM);
		sigprocmask(SIG_UNBLOCK, &ss, NULL);
	}
}

void initialize() {
	printf("check: initializing\n");
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
	sa.sa_flags = SA_NODEFER;
	sigaction(SIGALRM, &sa, NULL);

	struct itimerval timer;
  timer.it_value.tv_usec = 50*1000;
  timer.it_value.tv_sec = 0;
  timer.it_interval.tv_usec = 50*1000;
  timer.it_interval.tv_sec = 0;
  if(setitimer(ITIMER_REAL, &timer, NULL) == -1){
          printf("ERROR: Timer could not be initialized\n");
          exit(-1);
  }
}

int pthread_create(
  pthread_t *thread,
  const pthread_attr_t *attr,
  void *(*start_routine) (void *),
  void *arg) {
		printf("check: creating a thread in pthread_create\n");
		sigset_t ss;
		sigemptyset(&ss);
		sigaddset(&ss, SIGALRM);
		sigprocmask(SIG_BLOCK, &ss, NULL);
		if (!initialized) {
			initialize();
		}

		if (activeThreads < MAX_THREADS) {
			printf("assigning values in pthread_create\n");
			processThreads[activeThreads].id = *thread;
			processThreads[activeThreads].state = READY;
			int* bottom = malloc(32767/4);
			processThreads[activeThreads].rsp = bottom + 32767/4 - 8;
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
	printf("check: exiting thread %d\n", currentThread);
	printf("check: there are currently %d threads active (including the one exiting right now)\n", activeThreads);
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGALRM);
	sigprocmask(SIG_BLOCK, &ss, NULL);

	processThreads[currentThread].state = EXITED;
	processThreads[currentThread].rsp = NULL;
	activeThreads--;
	schedule();
  __builtin_unreachable();
}

pthread_t pthread_self(void) {
	// printf("check: return id %d\n", currentThread);
  return processThreads[currentThread].id;
}
