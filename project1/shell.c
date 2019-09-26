#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_CONSOLE_INPUT 512
#define MAX_CONSOLE_TOKENS 32

char *args[MAX_CONSOLE_TOKENS];
char* out_file;

bool file_out;

void parser(char* input, char* delim) {
  int commands = 0;
  // tells if next command is an input/output file instead of an argument
  bool next_out = false;

  char **iter = args;

  char *ptr = strtok(input, delim); // returns pointer to the next token
  while (ptr != NULL) // as soon as ptr is null, we have reached the end of the line
  {
      if (next_out) { // grabs output file redirect name
        out_file = ptr;
        file_out = true;
        next_out = false;
      } else if (ptr[0] == '>') { // flags next string as output file name and skips adding char to args
        next_out = true;
      } else if (ptr[0] != '&' && ptr[0] != '<') { // if normal command, save to args and increment
        *iter++ = ptr;
        commands++;
      }
      ptr = strtok(NULL, delim);
  }
  *iter = NULL; // need a null at the end to work properly with execvp

  // args now holds all strings from command line
  // ie, args = {"cat", "hello.txt", "|", "sort", "numbers.txt"};
  return;
}

bool valid_input(char* input) {
  int ins = 0;
  int outs = 0;
  int i;
  for (i = 0; i < MAX_CONSOLE_INPUT - 1; i++) { // to max minus 1 to avoid seg faulting on and check
    if (input[i] == '<') {
      ins++;
      if (i == 0) { // shouldn't be at beginning of line
        return false;
      } else if (input[i+1] == '\n') { // also shouldn't be at end
        return false;
      }
    }
    if(input[i] == '>') {
      // same checks as ins
      outs++;
      if (i == 0) { // shouldn't be at beginning of line
        return false;
      } else if (input[i+1] == '\n') { // also shouldn't be at end
        return false;
      }
    }
    if (input[i] == '&' && input[i+1] != '\n') { // has to be at end of line
      return false;
    }
  }
  if (ins > 1 || outs > 1) {
    return false;
  }
  return true;
}

int main(int argc, char* argv[], char** envp) {
  bool run = true; // breaks after ctrl + d
  char input[MAX_CONSOLE_INPUT]; // command line input from user

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
      memset(args, '\0', MAX_CONSOLE_TOKENS);
      file_out = false;

      if (print) { printf("my_shell$ "); }

      // grabs input from user
      if (fgets(input, sizeof(input), stdin) != NULL) {
        if (!valid_input(input)) {
          if (print) {printf("ERROR: Invalid input\n");}
          continue;
        }

        parser(input, " \n"); // parses input into args

          // int pipefd[2];
          // char input_str[100];
          //
          // if (has_pipe) { // only do setup if using a pipe
          //   if (pipe(pipefd)==-1)
          //   {
          //       fprintf(stderr, "Pipe Failed" );
          //       return 1;
          //   }
          //   scanf("%s", input_str);
          // }

        // fork needed to not overrun the current program
        // ie, parent program is processing input and running the shell
        // the parent process creates child processes to actually execute the commands
        pid_t pid = fork();

        if (pid < 0) {
          // error with forking
          fprintf(stderr, "Fork could not be completed" );
          return 1;
        }
        else if (pid == 0) { // child
          if (file_out) { // can only occur for last argument
            close(1);
            open(out_file, O_WRONLY | O_CREAT | O_APPEND); // automatically goes to file id 1
          }

          if (execvp(args[0], args) < 0) {
                if (print) {printf("ERROR: Command could not be executed \n");}
          } else {
            if (print) {printf("Executed command successfully\n");}
          }
          exit(0);
        }
        else { // parent process
          wait(NULL); // so child process finishes first
        } // checking pid for parent or child
      } else {
        // if fgets returns null (from ctrl + d)
        run = false;
        printf("\n");
      }
  }
  return 0;
}
