#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include "tls.h"
#include <string.h>
#include <stdbool.h>

#define SIZE 8000

bool finished = false;
bool cloned = false;
bool written = false;


// can be used to test tls_clone
void *test(void *arg) {
    pthread_t t1 = *((pthread_t *) arg);

    pthread_t t2 = pthread_self();

    while (!finished) {
      /* code */
    }

    if (tls_clone(t1)) {
      printf("Failed to clone thread 1's TLS to thread 2's\n");
    } else {
      printf("Successfully cloned thread 1's TLS to thread 2's\n");
      cloned = true;
      tls_print();

      while(!written) {
      }

      char buf[100];
      tls_read(4, 6, buf);
    }

    printf("Current threads: %ld (%d), %ld (%d)\n", t2, 2, t1, 1);

    return NULL;
}

void *test_tls_create(void *arg) {
    if (tls_create(SIZE)) {
      printf("Failed to create tls for thread 1\n");
    }
    else {
      printf("Successfully created tls for thread 1\n");
      // finished = true;

      tls_print();
      unsigned int offset = 4;
      char* buffer = "hello";

      // while (!cloned) {
      //   /* code */
      // }

      if (tls_write(offset, strlen(buffer), buffer)) {
        printf("Failed to write to tls for thread 1\n");
      } else {
        printf("Successfully wrote to tls for thread 1\n");
        finished = true;
        written = true;

        char buffer[100];
        if (tls_read(offset, 6, buffer)) {
          printf("Failed to read from tls for thread 1\n");
        } else {
          printf("Successfully read from tls for thread 1\n");
        }
      }

      while (!cloned) {
        /* code */
      }

      if (tls_destroy()) {
        printf("Failed to destroy tls for thread 1\n");
      } else {
        printf("Successfully destroyed tls for thread 1\n");
      }

    }

    return NULL;
}

int main(int argc, char **argv) {
    pthread_t t1, t2;

    if (pthread_create(&t1, NULL, test_tls_create, NULL)) {
        printf("Unable to create thread 1");
        exit(1);
    }
    if (pthread_create(&t2, NULL, test, (void *) &t1)) {
        printf("Unable to create thread 2");
        exit(1);
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
