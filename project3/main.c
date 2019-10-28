#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

#define THREAD_CNT 3
#define THREAD_TOTAL 6

// waste some time
void *count(void *arg) {
	unsigned long int c = (unsigned long int)arg;
	int i;
	for (i = 0; i < c; i++) {
		if ((i % 100000) == 0) {
			printf("tid: 0x%x Just counted to %d of %ld\n", (unsigned int)pthread_self(), i, c);
		}
	}
    return arg;
}

// void *string_function(void *arg) {
// 	unsigned long int c = 10000000*3;
// 	int i;
// 	for (i = 0; i < c; i++) {
// 		if ((i % 100000) == 0) {
// 			printf("tid: 0x%x Just counted to %d of %ld\n", (unsigned int)pthread_self(), i, c);
// 		}
// 	}
//     return arg;
// }

int main(int argc, char **argv) {
	pthread_t threads[THREAD_CNT];
	int i;
	unsigned long int cnt = 10000000;

    //create THRAD_CNT threads
	for(i = 0; i<THREAD_CNT; i++) {
		pthread_create(&threads[i], NULL, count, (void *)((i+1)*cnt));
	}

    //join all threads ... not important for proj2
		void* thread_value;

		for(i = 0; i<THREAD_CNT; i++) {
			pthread_join(threads[i], &thread_value);
		}
    // But we have to make sure that main does not return before
    // the threads are done... so count some more...
    count((void *)(cnt*(THREAD_CNT + 1)));
		// printf("Thread main value_ptr from pthread exit is: %p\n", *thread_values);
    return 0;
}

// int main(int argc, char **argv)
// {
//   // THREAD_TOTAL is 6 (3 count threads, 3 string_function threads)
//   pthread_t threads[THREAD_TOTAL];
//   int i;
//   unsigned long int cnt = 100000000;
//
//   // Added to check thread return values (only useful for count threads)
//   unsigned long int thread_args[THREAD_TOTAL];
//
//   // Added for checking return values
//   void *ret;
//   char *my_string = "Hello World!";
//
//   //create THREAD_CNT threads
//   for(i = 0; i<THREAD_CNT; i++)
//   {
//     pthread_create(&threads[i], NULL, count, (void *)((i+1)*cnt));
//     thread_args[i] = (i+1)*cnt;
//   }
//   for (i = THREAD_CNT; i < THREAD_TOTAL; i++)
//   {
//     pthread_create(&threads[i], NULL, string_function, (void *)(my_string));
//     thread_args[i] = 0;  // These elements are pointless for the string_function
//   }
//   printf("Threads have been made\n");
//
//   //join all threads
//   for(i = 0; i<THREAD_TOTAL; i++)  // Was THREAD_CNT
//   {
//     printf("Main is joining thread %i...\n", i+1);
//     if ( pthread_join(threads[i], &ret) >= 0 )
//     {
//       printf("Main finished joining thread %i!\n", i+1);
//
//       if ( i < 3 )
//       {
//         unsigned long int temp = (unsigned long int)ret;
//         printf("Thread %i returned: %ld\n", i+1, temp);
//         printf("Thread argument  : %ld\n", thread_args[i]);
//       }
//       else
//       {
//         char *temp = (char *)ret;
//         printf("Thread %i returned: '%s'\n", i+1, temp);
//         printf("Thread argument  : '%s'\n", my_string);
//       }
//     }
//     else
//     {
//       printf("Could not join thread %i\n", i+1);
//     }
//   }
//
//   // But we have to make sure that main does not return before
//   // the threads are done... so count some more...
//   //count((void *)(cnt*(THREAD_TOTAL + 1)));  // Was THREAD_CNT
//
//   printf("Thread test complete\n");
//
//   return 0;
// }
