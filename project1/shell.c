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

bool print;
bool file_out;

void executePipes() {
  int fd[2];
  char* pipeargs1[32];
  char* pipeargs2[32];

  if(pipe(fd)){
    perror("pipe");
    return;
  }
  switch(fork()){
      case -1:
          perror("fork");
          break;
      case 0:
          // child first arg
          pipeargs1[0] = "cat";
          pipeargs1[1] = "nums.txt";
          pipeargs1[2] = NULL;

          close(fd[0]);
          dup2(fd[1], 1);
          close(fd[1]);
          execvp(pipeargs1[0], pipeargs1);
          exit(0);
      default:
          // parent first arg
          pipeargs2[0] = "sort";
          pipeargs2[2] = NULL;
          close(fd[1]);
          dup2(fd[0], STDIN_FILENO);
          close(fd[0]);
          execvp(pipeargs2[0], pipeargs2);
          wait(NULL);
  }
  printf("END~\n");
}

void execute(char* fargv[], int fin, int fout) {
  pid_t pid = fork();

  if (pid < 0) {
    // error with forking
    if (print)
      printf("Fork could not be completed\n");
  }
  else if (pid == 0) { // child
    if (fin != 0) { // adjusts input
      dup2(fin, 0); // replaces stdin with fin
      close(fin);
    }
    if (fout != 1) { // adjusts output
      // close(1);
      dup2(fout, 1); // replaces stdout with fin
      close(fout);
    }

    printf("fin: %d\n", fin);
    printf("fout: %d\n", fout);
    int exec = execvp(fargv[0], fargv);
    if (exec < 0) {
      if (print)
        printf("ERROR: Could not execute command\n");
    }
    exit(0);
  }
  else { // parent process
    wait(NULL); // so child process finishes first
  } // checking pid for parent or child
}

int parser(char* input, char* delim) {
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
  return commands;
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
  char* fargv[MAX_CONSOLE_TOKENS];

  // checks for supressing output
  print = true;
  if (argc > 1) {
    print = (strcmp(argv[1],"-n") != 0);
  }

  // loops infinitely
  while (run)
  {
      // clears any previous input
      memset(input,'\0', MAX_CONSOLE_INPUT);
      memset(args, '\0', MAX_CONSOLE_TOKENS);
      memset(fargv, '\0', MAX_CONSOLE_TOKENS);
      file_out = false;

      if (print) { printf("my_shell$ "); }

      // grabs input from user
      if (fgets(input, sizeof(input), stdin) != NULL) {
        if (!valid_input(input)) {
          if (print) {printf("ERROR: Invalid input\n");}
          continue;
        }

        int cmds = parser(input, " \n"); // parses input into args

        // fork needed to not overrun the current program
        // ie, parent program is processing input and running the shell
        // the parent process creates child processes to actually execute the commands

        int index = 0; // index for each individual word

        bool has_pipe = false;
        for (int i = 0; i < cmds; i++) {
          if (args[i][0] == '|') {
            has_pipe = true;
          }
        }

        if (!has_pipe) {
          for (int i = 0; i < cmds; i++) {
            fargv[index] = args[i];
            index++;
          }

          int fout = 1;
          if (file_out) {
            fout = open(out_file, O_WRONLY | O_CREAT | O_APPEND);
          }
          execute(fargv, 0, fout);
        } else {
          // has pipe

          int fd[2];
          char* pipeargs1[32];
          char* pipeargs2[32];

          if(pipe(fd)){
            perror("pipe");
            return -1;
          }
          switch(fork()){
              case -1:
                  perror("fork");
                  break;
              case 0:
                  // child second arg
                  pipeargs1[0] = "sort";
                  pipeargs1[2] = NULL;
                  close(fd[1]);
                  dup2(fd[0], STDIN_FILENO);
                  close(fd[0]);
                  execvp(pipeargs1[0], pipeargs1);
                  exit(0);
              default:
                  // parent first arg
                  pipeargs2[0] = "cat";
                  pipeargs2[1] = "nums.txt";
                  pipeargs2[2] = NULL;
                  close(fd[0]);
                  dup2(fd[1], 1);
                  close(fd[1]);
                  execvp(pipeargs2[0], pipeargs2);
                  wait(NULL);
          }
          printf("END~\n");

        }
      } else {
        // if fgets returns null (from ctrl + d)
        run = false;
        printf("\n");
      }
  }
  return 0;
}
