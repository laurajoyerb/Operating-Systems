#include <stdio.h>
#include <unistd.h>
#include<pthread.h>
#include <stdlib.h>

// struct thread {
// 	pthread_t id;
// 	// Information about the state of the thread (set of registers)
//
// 	// Information about its stack (pointer to stack area)
// 	Information about status of thread (ready, running, exited)
// }


int pthread_create(
  pthread_t *thread,
  const pthread_attr_t *attr,
  void *(*start_routine) (void *),
  void *arg) {
    return 0;
}

void pthread_exit(void *value_ptr) {
  exit(0);
}

pthread_t pthread_self(void) {
  pthread_t foo = NULL;
  return foo;
}
// 
// int main()
// {
//     return 0;
// }
