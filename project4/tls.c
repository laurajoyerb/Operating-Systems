#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include "semaphore.h"
#include "tls.h"

struct page {
  unsigned int address;
  int ref_count;
};

typedef struct thread_local_storage {
  pthread_t id;
  unsigned int size;
  unsigned int num_ages;
  struct page ** pages;
} TLS;

int tls_create(unsigned int size) {
  return 0;
}

int tls_write(unsigned int offset, unsigned int length, char *buffer) {
  return 0;
}

int tls_read(unsigned int offset, unsigned int length, char *buffer) {
  return 0;
}

int tls_destroy() {
  return 0;
}

int tls_clone(pthread_t tid) {
  return 0;
}
