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
#include "semaphore.h"

#define MAX_THREADS 128

// states of threads
#define READY 0
#define EXITED 1
#define BLOCKED 2

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
int numSems = 0;

int nextThread = -1;

struct thread {
	pthread_t id;
  jmp_buf reg;
  int state;
  int* rsp;
	int blocked_thread;
	int num_blocks;
	void* exit_status;
	void** store_exit_status;
};

struct threadNode {
	struct threadNode* next;
	int id;
};

struct waitingThreads {
	struct threadNode *first;
	struct threadNode *last;
	int number;
};

struct semaphore {
	int counter;
	struct waitingThreads *queue;
	sem_t* id;
};

// arrays to hold all threads and semaphores
struct thread processThreads[MAX_THREADS];
struct semaphore processSems[MAX_THREADS];

void schedule();
void initialize();
void lock();
void unlock();
int pthread_join(pthread_t thread, void **value_ptr);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
void pthread_exit_wrapper();
void pthread_exit(void *value_ptr);
pthread_t pthread_self(void);

int sem_init(sem_t *sem, int pshared, unsigned value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_destroy(sem_t *sem);

int sem_init(sem_t *sem, int pshared, unsigned value) {
	lock();
	struct semaphore temp;
	temp.counter = value;
	struct waitingThreads *q = (struct waitingThreads*)malloc(sizeof(struct waitingThreads));
	q->first = NULL;
	q->last = NULL;
	q->number = 0;
	temp.queue = q;
	temp.id = sem;
	sem->__align = numSems;
	processSems[numSems] = temp;
	numSems++;
	unlock();
	return 0;
}

int sem_wait(sem_t *sem) {
	lock();
	if (processSems[sem->__align].counter == 0) {
		processThreads[currentThread].state = BLOCKED;

		// adds to queue
		struct threadNode *node = (struct threadNode*)malloc(sizeof(struct threadNode));
		node->next = NULL;
		node->id = currentThread;

		if (processSems[sem->__align].queue->number == 0) {
			processSems[sem->__align].queue->first = node;
			processSems[sem->__align].queue->last = node;
			processSems[sem->__align].queue->first->next = node;
		} else {
			processSems[sem->__align].queue->last->next = node;
			processSems[sem->__align].queue->last = node;
		}
		processSems[sem->__align].queue->number++;

	} else {
		processSems[sem->__align].counter--;
	}
	unlock();
	schedule();
	return 0;
}

int sem_post(sem_t *sem) {
	lock();
	if (processSems[sem->__align].counter == 0 && processSems[sem->__align].queue->number > 0) {
		int next = processSems[sem->__align].queue->first->id;
		processThreads[next].state = READY; // unblocks next thread
		nextThread = next;
		struct threadNode* temp = processSems[sem->__align].queue->first;
		processSems[sem->__align].queue->first = processSems[sem->__align].queue->first->next;
		free(temp);
		processSems[sem->__align].queue->number--;
	} else {
		processSems[sem->__align].counter++;
	}
	unlock();
	schedule();
	return 0;
}

int sem_destroy(sem_t *sem) {
	lock();
	processSems[sem->__align].counter = 0;
	if (processSems[sem->__align].queue->number > 0) {
		printf("Not good\n");
	}
	free(processSems[sem->__align].queue);
	processSems[sem->__align].id = NULL;
	unlock();
	return 0;
}

void schedule() {
	if (setjmp(processThreads[currentThread].reg) == 0) {
		// frees memory from exited threads
		int i;
		 for (i = 0; i < MAX_THREADS; i++) {
		 		if (processThreads[i].state == EXITED) {
					free(processThreads[i].rsp);
				}
		 }

		 if (nextThread != -1) {
 			currentThread = nextThread;
			nextThread = -1;
 		} else {

				// wraps around back to 0 if necessary
			 if (activeThreads - 1 <= currentThread) {
				 currentThread = 0;
			 } else {
				 currentThread++;
			 }

			 // finds next ready thread
			 while (processThreads[currentThread].state != READY || currentThread >= MAX_THREADS) {
				 currentThread++;
				 // wraps if necessary
				 if (currentThread >= activeThreads) {
					 currentThread = 0;
				 }
			 }

 		}
		unlock();
		longjmp(processThreads[currentThread].reg, 1); // jumps to next thread
	} else {
		unlock();
	}
}

void initialize() {
	lock();
	// runs the first time a thread is created in order to add the main process as the first thread in the TCB
	initialized = true;
	currentThread = 0;

	processThreads[0].id = 0;
	processThreads[0].state = READY;
	processThreads[0].blocked_thread = -1;
	processThreads[0].num_blocks = 0;
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
	unlock();
}

void lock() {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGALRM);
	sigprocmask(SIG_BLOCK, &ss, NULL);
}

void unlock() {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &ss, NULL);
};

int pthread_join(pthread_t thread, void **value_ptr) {
	lock();
	int target = 1;

	while (processThreads[target].id != thread) {
		target++;
		if (target >= 128) {
			printf("Error: Target thread could not be identified, no match for %x\n", (unsigned int) thread);
			return -1;
		}
	}
	if (processThreads[target].state == EXITED) {
		if (value_ptr != NULL) {
			*value_ptr = processThreads[target].exit_status;
		}
		unlock();
		return 0;
	}

	processThreads[target].blocked_thread = currentThread;
	if (value_ptr != NULL) {
		// *value_ptr = processThreads[target].exit_status;
		processThreads[target].store_exit_status = value_ptr;
	} else {
	}
	// processThreads[target].store_exit_status = *value_ptr;
	processThreads[currentThread].state = BLOCKED;
	processThreads[currentThread].num_blocks++;
	schedule();
	return 0;
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
			lock();
			processThreads[activeThreads].id = *thread;
			processThreads[activeThreads].blocked_thread = -1;
			processThreads[0].num_blocks = 0;
			processThreads[activeThreads].state = READY;
			void* bottom = malloc(32767);
			processThreads[activeThreads].rsp = bottom + 32767 - sizeof(unsigned long);
			*(processThreads[activeThreads].rsp) = (unsigned long) &pthread_exit_wrapper;
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

void pthread_exit_wrapper()
{
	unsigned long int res;
	asm("movq %%rax, %0\n":"=r"(res));
	pthread_exit((void *) res);
}

void pthread_exit(void *value_ptr) {
	lock();
	processThreads[currentThread].state = EXITED;
	processThreads[currentThread].rsp = NULL;
	processThreads[currentThread].exit_status = value_ptr;
	if (processThreads[currentThread].store_exit_status != NULL) {
		*(processThreads[currentThread].store_exit_status) = value_ptr;
	}

	int blocked_thread = processThreads[currentThread].blocked_thread;
	if (blocked_thread != -1) {
		processThreads[blocked_thread].num_blocks--;
		if (
				processThreads[blocked_thread].num_blocks == 0 &&
				processThreads[blocked_thread].state == BLOCKED
			) {
			processThreads[blocked_thread].state = READY;
		}
	}
	schedule();
  __builtin_unreachable();
}

pthread_t pthread_self(void) {
  return processThreads[currentThread].id;
}
