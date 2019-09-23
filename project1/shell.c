#include <stdio.h>
#include <stdbool.h>
#include <libc.h>

#define MAX_CONSOLE_INPUT 512
#define MAX_CONSOLE_TOKENS 32

int main(int argc, char* argv[], char** envp) {
  bool run = true;
  char input[MAX_CONSOLE_INPUT];

  // checks for supressing output
  bool print = true;
  if (argc > 1) {
    print = (strcmp(argv[1],"-n") != 0);
  }

  // loops infinitely
  while (run)
  {
      // clears any previous input
      memset(input,'\0', MAX_CONSOLE_INPUT);

      if (print) { printf("my_shell$ "); }

      // special characters
      bool file_in = false;
      bool file_out = false;
      bool pipe = false;
      bool and = false;

      // grabs input from user
      if (fgets(input, sizeof(input), stdin) != NULL) {
        // flags for special characters
        for (int i = 0; i < sizeof(input); i++) {
          if (input[i] == '<') {
            file_in = true;
          }
          if (input[i] == '>') {
            file_out = true;
          }
          if (input[i] == '|') {
            pipe = true;
          }
          if (input[i] == '&') {
            and = true;
          }
        }
        printf("\n");
        char *args[MAX_CONSOLE_TOKENS];
        char **iter = args;
        char *ptr = strtok(input, " \n"); // returns pointer to the next token
        while (ptr != NULL) // as sooon as ptr is null, we have reached the end of the line
        {
            *iter++ = ptr;
            ptr = strtok(NULL, " \n");
        }
        *iter = NULL; // need a null at the end to work properly with execvp

        // fork needed to not overrun the current program
        // ie, parent program is processing input and running the shell
        // the parent process creates child processes to actually execute the commands
        pid_t pid = fork();

        if (pid == 0) {
          // child
          if (execvp(args[0], args) < 0) {
                if (print) {printf("ERROR: Command could not be executed \n");}
          } else {
            // parent
            if (print) {printf("Executed command successfully\n");}
          }
          exit(0);
        } else {
          wait(NULL);
        }
      } else {
        // if fgets returns null (from ctrl + d)
        run = false;
        printf("\n");
      }
  }
  return 0;
}
