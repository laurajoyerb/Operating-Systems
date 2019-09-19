#include <stdio.h>
#include <stdbool.h>
#include <libc.h>

#define MAX_INPUT 1024
#define MAX_TOKENS 256

int main(int argc, char* argv[]) {
  char input[MAX_INPUT];

  bool print = true;
  if (argc > 1) {
    print = (strcmp(argv[1],"-n") != 0);
  }

  while (true)
  {
      if (print) { printf("my_shell$ "); }
      fgets(input, sizeof(input), stdin);

      char *args[MAX_TOKENS];
      char **next = args;
      char *ptr = strtok(input, " \n");
      while (ptr != NULL)
      {
          *next++ = ptr;
          ptr = strtok(NULL, " \n");
      }
      *next = NULL;

      pid_t pid = fork();

      if (pid == 0) {
        if (execvp(args[0], args) < 0) {
              if (print) {printf("ERROR: Could not execute command \n");}
        } else {
          if (print) {printf("executed command\n");}
        }
        exit(0);
      } else {
        wait(NULL);
      }
  }
  return 0;
}
