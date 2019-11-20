#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "semaphore.h"
#include "tls.h"
#include <stdbool.h>
#include <signal.h>
#include <sys/mman.h>

#define HASH_SIZE 100

struct page {
  unsigned long int address;
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

void tls_protect(struct page* p) {
  if(mprotect((void *) p->address, page_size, 0)) {
    fprintf(stderr, "tls_protect: could not protect page\n");
    exit(1);
  }
}

void tls_unprotect(struct page* p) {
  if(mprotect((void *) p->address, page_size, PROT_READ | PROT_WRITE)) {
    fprintf(stderr, "tls_unprotect: could not unprotect page\n");
    exit(1);
  }
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

  // checks if thread already has local storage
  int i;
  for (i = 0; i < 128; i++) {
    if (tls_map[i].id == pthread_self()) {
      return -1;
    }
  }

  // current thread has no LSA yet

  // finds next space new tls
  int next = -1;
  for (next = 0; next < 128; next++) {
    if (tls_map[next].id == -1) {
      break;
    }
  }

  struct TLS* temp = calloc(1, sizeof(struct TLS));

  temp->id = pthread_self();
  temp->size = size;
  temp->num_pages = (size - 1) / (page_size) + 1;

  for (i = 0; i < temp->num_pages; i++) {
    // struct page* p = NULL; // = (struct page *) calloc(1, sizeof(struct page*));
    // p->address = (unsigned long int) mmap(0, page_size, 0, MAP_ANON | MAP_PRIVATE, 0, 0);
    // p->ref_count = 1;
    // temp->pages[i] = p;
  }

  // tls_map[next] = *temp;

  return 0;
}

int tls_write(unsigned int offset, unsigned int length, char *buffer) {
  int currTLS;
  bool hasTLS = false;
  for (currTLS = 0; currTLS < 128; currTLS++) {
    if (tls_map[currTLS].id == pthread_self()) {
      hasTLS = true;
      break;
    }
  }

  if (hasTLS == false) {
    return -1;
  }

  if (tls_map[currTLS].size < offset + length) {
    return -1;
  }

  int i;
  for (i = 0; i < tls_map[currTLS].num_pages; i++) {
    tls_unprotect(tls_map[currTLS].pages[i]);
  }

  int cnt, idx;
  for (cnt = 0, idx = offset; idx < (offset + length); ++cnt, ++idx) {
    struct page *p, *copy;
    unsigned int pn, poff;
    pn = idx / page_size;
    poff = idx % page_size;
    p = tls_map[currTLS].pages[pn];
    if (p->ref_count > 1) {
      /* this page is shared, create a private copy (COW) */
      copy = (struct page *) calloc(1, sizeof(struct page));
      copy->address = (unsigned long int) mmap(0, page_size, PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
      copy->ref_count = 1;
      tls_map[currTLS].pages[pn] = copy;
      /* update original page */
      p->ref_count--;
      tls_protect(p);
      p = copy;
    }
    char* dst = ((char *) p->address) + poff;
    *dst = buffer[cnt];
  }

  for (i = 0; i < tls_map[currTLS].num_pages; i++) {
    tls_protect(tls_map[currTLS].pages[i]);
  }

  return 0;
}

int tls_read(unsigned int offset, unsigned int length, char *buffer) {
  int currTLS = -1;
  for (currTLS = 0; currTLS < 128; currTLS++) {
    if (tls_map[currTLS].id == pthread_self()) {
      break;
    }
  }

  // thread does not yet have a tls
  if (currTLS == -1) {
    return -1;
  }

  if (tls_map[currTLS].size < offset + length) {
    return -1;
  }

  int i;
  for (i = 0; i < tls_map[currTLS].num_pages; i++) {
    tls_unprotect(tls_map[currTLS].pages[i]);
  }

  int cnt, idx;
  for (cnt = 0, idx = offset; idx < (offset + length); ++cnt, ++idx) {
    struct page *p;
    unsigned int pn, poff;
    pn = idx / page_size;
    poff = idx % page_size;
    p = tls_map[currTLS].pages[pn];
    char* src = ((char *) p->address) + poff;
    buffer[cnt] = *src;
  }

  for (i = 0; i < tls_map[currTLS].num_pages; i++) {
    tls_protect(tls_map[currTLS].pages[i]);
  }

  return 0;
}

int tls_destroy() {
  int currTLS = -1;
  for (currTLS = 0; currTLS < 128; currTLS++) {
    if (tls_map[currTLS].id == pthread_self()) {
      break;
    }
  }

  // thread does not yet have a tls
  if (currTLS == -1) {
    return -1;
  }

  tls_map[currTLS].id = -1;
  tls_map[currTLS].size = -1;
  tls_map[currTLS].num_pages = -1;
  tls_map[currTLS].pages = NULL;

  return 0;
}

int tls_clone(pthread_t tid) {
  return 0;
}
