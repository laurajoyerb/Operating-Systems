#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include "tls.h"

#define SIZE 8000


// can be used to test tls_clone
void *test(void *arg) {
    pthread_t t1 = *((pthread_t *) arg);

    pthread_t t2 = pthread_self();

    printf("Current threads: %ld, %ld\n", t2, t1);

    return NULL;
}

void *test_tls_create(void *arg) {
    if (tls_create(SIZE)) {
      printf("Failed to create tls for thread 1\n");
    }
    else {
      printf("Successfully created tls for thread 1\n");

      tls_print();

      if (tls_destroy()) {
        printf("Failed to destory tls for thread 1\n");
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
