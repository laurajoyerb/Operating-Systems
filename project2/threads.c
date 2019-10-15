#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdbool.h>

#define MAX_THREADS 128

// states of threads
#define READY 0
#define RUNNING 1
#define EXITED 2

bool initialized = false;

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
	printf("intitializing\n");
}

int pthread_create(
  pthread_t *thread,
  const pthread_attr_t *attr,
  void *(*start_routine) (void *),
  void *arg) {
		if (!initialized) {
			initialize();
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
