#include <stdio.h>
#include <stdbool.h>
#include <libc.h>

#define MAX_CONSOLE_INPUT 512
#define MAX_CONSOLE_TOKENS 32

char *args[MAX_CONSOLE_TOKENS];

int parser(char* input, char* delim) {
  int commands = 0;
  char **iter = args;

  char *ptr = strtok(input, delim); // returns pointer to the next token
  while (ptr != NULL) // as soon as ptr is null, we have reached the end of the line
  {
      if (ptr[0] != '>' && ptr[0] != '<' && ptr[0] != '|' && ptr[0] != '&') {
        commands++;
        *iter++ = ptr;
      }
      ptr = strtok(NULL, delim);
  }
  printf("outta the while loop\n");
  *iter = NULL; // need a null at the end to work properly with execvp
  return commands;
}

bool valid_input(char* input) {
  int ins = 0;
  int outs = 0;
  for (int i = 0; i < MAX_CONSOLE_INPUT - 1; i++) {
    if (input[i] == '<') {
      ins++;
      if (i == 0) {
        return false;
      } else if (input[i] == '&' && input[i+1] != '\n') {
        return false;
      }
    }
    if(input[i] == '>') {
      outs++;
      if (input[i+1] == '\n') {
        return false;
      }
    }
    if (input[i] == '&' && input[i+1] != '\n') {
      return false;
    }
  }
  if (ins > 1 || outs > 1) {
    return false;
  }
  return true;
}

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
        if (!valid_input(input)) {
          printf("Invalid input\n");
          continue;
        }
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

        int fin;

        int cmds = parser(input, " \n");

        printf("\n");
        for (int i = 0; i < cmds; i++) {
          printf("%s ", args[i]);
        }
        printf("\n");
        printf("commands: %d\n", cmds);

        // fork needed to not overrun the current program
        // ie, parent program is processing input and running the shell
        // the parent process creates child processes to actually execute the commands

        pid_t pid = fork();

        if (pid == 0) {
          // child
          char *nofile[MAX_CONSOLE_TOKENS];
          for (int i = 0; i < cmds - 1; i++) {
            nofile[i] = args[i];
          }

          if (file_out) {
            close(1);
            fin = open(args[cmds-1], O_WRONLY | O_CREAT);
            dup2(fin, 1);
          } else {
            nofile[cmds - 1] = args[cmds - 1];
          }

          if (execvp(args[0], nofile) < 0) {
                if (print) {printf("ERROR: Command could not be executed \n");}
          } else {
            if (print) {printf("Executed command successfully\n");}
          }
          exit(0);
        } else {
          // parent
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
