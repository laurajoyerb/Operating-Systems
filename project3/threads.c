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
#define EXITED 1

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
	if (setjmp(processThreads[currentThread].reg) == 0) {

		// frees memory from exited threads
		int i;
		 for (i = 0; i < MAX_THREADS; i++) {
		 		if (processThreads[i].state == EXITED) {
					free(processThreads[i].rsp);
				}
		 }
		 // wraps around back to 0 if necessary
		if (activeThreads - 1 <= currentThread) {
			currentThread = 0;
		} else {
			currentThread++;
		}

		// finds next ready thread
		while (processThreads[currentThread].state != READY) {
			currentThread++;
			// wraps if necessary
			if (activeThreads >= currentThread) {
				currentThread = 0;
			}
		}

		// unblocks thread
		sigset_t ss;
		sigemptyset(&ss);
		sigaddset(&ss, SIGALRM);
		sigprocmask(SIG_UNBLOCK, &ss, NULL);

		longjmp(processThreads[currentThread].reg, 1); // jumps to next thread
	} else {
		// unblocks threads that had previously been stopped by the scheduler
		sigset_t ss;
		sigemptyset(&ss);
		sigaddset(&ss, SIGALRM);
		sigprocmask(SIG_UNBLOCK, &ss, NULL);
	}
}

void initialize() {
	// runs the first time a thread is created in order to add the main process as the first thread in the TCB
	initialized = true;
	currentThread = 0;

	processThreads[0].id = 0;
	processThreads[0].state = READY;
	processThreads[0].rsp = NULL;
	setjmp(processThreads[0].reg);
	activeThreads++;

	// sets up signal handler for sig alarm, points it to the schedule() function
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = schedule; // the schedule() function will handle the action for the signal SIGALRM
	// this makes sure that the SIGALRM signal will still be processed even inside of the schedule() function
	sa.sa_flags = SA_NODEFER;
	sigaction(SIGALRM, &sa, NULL);

	// sets up a timer to send a sig alarm every 50ms
	struct itimerval timer;

	// time until the first timer interrupt
  timer.it_value.tv_usec = 50*1000;
  timer.it_value.tv_sec = 0;

	// how often the timer should go after the first time (ie, interval between alarms)
  timer.it_interval.tv_usec = 50*1000;
  timer.it_interval.tv_sec = 0;
  if(setitimer(ITIMER_REAL, &timer, NULL) == -1){
          printf("ERROR: Timer malfunctioned\n");
          exit(-1);
  }
}

void lock() {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGALRM);
	sigprocmask(SIG_BLOCK, &ss, NULL);
}

void unlock() {
	// unblocks thread
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &ss, NULL);
};

int pthread_join(pthread_t thread, void **value_ptr) {
	return 0;
}

int pthread_create(
  pthread_t *thread,
  const pthread_attr_t *attr,
  void *(*start_routine) (void *),
  void *arg) {
		sigset_t ss;
		sigemptyset(&ss);
		sigaddset(&ss, SIGALRM);
		sigprocmask(SIG_BLOCK, &ss, NULL);
		if (!initialized) {
			initialize();
		}

		if (activeThreads < MAX_THREADS) {
			processThreads[activeThreads].id = *thread;
			processThreads[activeThreads].state = READY;
			void* bottom = malloc(32767);
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
  return processThreads[currentThread].id;
}
