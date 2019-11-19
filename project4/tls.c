#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "semaphore.h"
#include "tls.h"
#include <stdbool.h>
#include <signal.h>

#define HASH_SIZE 100

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

struct hash_element {
  pthread_t id;
  TLS *tls;
  struct hash_element *next;
};

bool initialized = false;
int page_size;

struct hash_element* hash_table[HASH_SIZE];

void tls_handle_page_fault() {
  printf("page fault handled\n");
}

void tls_init() {
  struct sigaction sigact;

  page_size = getpagesize();

  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = SA_SIGINFO;
  sigact.sa_sigaction = tls_handle_page_fault;

  sigaction(SIGBUS, &sigact, NULL);
  sigaction(SIGSEGV, &sigact, NULL);

  initialized = true;
}

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
