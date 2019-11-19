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

struct TLS {
  pthread_t id;
  unsigned int size;
  unsigned int num_pages;
  struct page ** pages;
};

struct TLS tls_map[128];

bool initialized = false;
int page_size;

void tls_handle_page_fault() {
  printf("page fault handled\n");
}

void tls_init() {
  int i;
  for (i = 0; i < 128; i++) {
    tls_map[i].id = -1;
  }
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
  if(!initialized) {
    tls_init();
  }

  if (size < 0) {
    return -1;
  }




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
