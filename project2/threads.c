#include <stdio.h>
#include <unistd.h>
#include<pthread.h>
#include <stdlib.h>

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

int main()
{
    return 0;
}
