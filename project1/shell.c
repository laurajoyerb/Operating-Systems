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

bool background;

struct pipeCommand // makes storing arguments easier (UGH SEG FAULTS)
{
  const char **arg;
};

void execute(char* fargv[], int fin, int fout) {
  // fargv contains all args for a single, non piped command
  // fin and fout hold the file descriptors for where execvp should input and output
  signal(SIGCHLD, SIG_IGN); /* Silently (and portably) reap children. */
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

    int exec = execvp(fargv[0], fargv);
    if (exec < 0) {
      printf("ERROR: Could not execute command\n");
    }
    exit(0);
  }
  else { // parent process
    if (!background)
      wait(NULL);
  }
}

int forkProcess (int in, int out, struct pipeCommand *cmd) {
  signal(SIGCHLD, SIG_IGN); /* Silently (and portably) reap children. */
  pid_t pid = fork();

  if (pid == 0) {
    if (in != 0) {
      dup2 (in, 0);
      close (in);
    }

    if (out != 1) {
      dup2 (out, 1);
      close (out);
    }
    return execvp (cmd->arg [0], (char* const*)cmd->arg);
  } else if (pid > 0) {
    if (!background)
      wait(NULL);
  }
  return pid;
}

void executePipes(int n, struct pipeCommand *cmd) {
  int i;
  int in, fd [2];
  in = 0;

  for (i = 0; i < n - 1; ++i) {
    pipe (fd);

    // pass in input from previous pipe (or std in if first iteration), output is write end of pipe
    forkProcess (in, fd[1], cmd + i);
    close (fd[1]);

    // redirects the next input to be the read end of the current pipe
    in = fd [0];
  }

  // redirects stdin to read end of pipe to grab the last write
  if (in != 0)
    dup2 (in, 0);

  // last pipe command has to be executed separately, so this final process forks and completes it
  int fout = 1;
  if (file_out) {
    fout = open(out_file, O_RDWR | O_CREAT | O_APPEND);
  }
  execute((char**) cmd[i].arg, 0, fout);
}

int parser(char* input, char* delim) {
  int commands = 0;
  // tells if next command is an input/output file instead of an argument
  bool next_out = false;

  char **iter = args;

  char *ptr = strtok(input, delim); // returns pointer to the next token
  while (ptr != NULL) // as soon as ptr is null, we have reached the end of the line
  {
      if (ptr[strlen(ptr) - 1] == '<' && strlen(ptr) > 1) { // catches inputs attached to first command
        ptr[strlen(ptr) - 1] = '\0';
      }

      if(ptr[0] == '|' && strlen(ptr) > 1) { // catches pipes on second command ie ls |sort
        *iter++ = "|";
        commands++;
        ptr[0] = '\0';
        ptr++;
      }

      char* badspaces = NULL;
      char* badpipespace = NULL;

      for (int i = 1; i < strlen(ptr) - 1; i++) {
        if(ptr[i] == '<') {
          badspaces = ptr;
          badspaces += i + 1;
          ptr[i] = '\0';
        }
        if (ptr[i] == '>') {
          out_file = ptr;
          out_file += i + 1;
          ptr[i] = '\0';
          file_out = true;
          next_out = false;
        }
        if (ptr[i] == '|') {
          badpipespace = ptr;
          badpipespace += i + 1;
          ptr[i] = '\0';
        }
      }

      if (ptr[strlen(ptr) - 1] == '>' && strlen(ptr) > 1) { // catches output redirects attached to first command
        next_out = true;
        ptr[strlen(ptr) - 1] = '\0';
        *iter++ = ptr;
        commands++;
      } else if (ptr[strlen(ptr) - 1] == '|' && strlen(ptr) > 1) { // catches pipes attached to first command ie ls| sort
        ptr[strlen(ptr) - 1] = '\0';
        *iter++ = ptr;
        commands++;
        ptr = "|";
        *iter++ = ptr;
        commands++;
      } else if (next_out) { // grabs output file redirect name
        out_file = ptr;
        file_out = true;
        next_out = false;
      } else if (ptr[0] == '>') { // flags next string as output file name and skips adding char to args
        if (ptr[1] != '\0') { // catches outputs attached to second command (output file name)
          out_file = ++ptr;
          file_out = true;
        } else {
          next_out = true;
        }
      } else if (ptr[0] == '<') { // flags next string as output file name and skips adding char to args
        if (ptr[1] != '\0') { // if no space between input and next character, save attached word as arg
          *iter++ = ++ptr;
          commands++;
        }
      } else if (ptr[0] != '&') { // if normal command, save to args and increment
        *iter++ = ptr;
        commands++;
      }

      if (badspaces != NULL) {
        *iter++ = badspaces;
        commands++;
        badspaces = NULL;
      } else if (badpipespace != NULL) {
        *iter++ = "|";
        commands++;
        *iter++ = badpipespace;
        commands++;
        badpipespace = NULL;
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
  bool has_pipe = false;
  for (i = 0; i < MAX_CONSOLE_INPUT - 1; i++) { // to max minus 1 to avoid seg faulting on and check
    if (input[i] == '|') {
      has_pipe = true;
      if (outs > 0) {
        return false;
      }
    }
    if (input[i] == '<') {
      ins++;
      if (i == 0) { // shouldn't be at beginning of line
        return false;
      } else if (input[i+1] == '\n') { // also shouldn't be at end
        return false;
      } else if (has_pipe) {
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
    if (input[i] == '&') { // has to be at end of line
      background = true;
      if (input[i + 1] != '\n')
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

  int stdinOrig = dup(0);
  int stdoutOrig = dup(1);

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
      background = false;

      // ensures that stdin and stdout are reopened before accepting new commands
      dup2(stdinOrig, 0);
      dup2(stdoutOrig, 1);

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
            fout = open(out_file, O_RDWR | O_CREAT | O_APPEND);
          }
          execute(fargv, 0, fout);
        } else {
          // has pipe
          index = 0;
          const char* pipeargs1[32];
          const char* pipeargs2[32];
          const char* pipeargs3[32];
          const char* pipeargs4[32];
          const char* pipeargs5[32];
          const char* pipeargs6[32];
          const char* pipeargs7[32];

          int pipeNum = 1;
          bool tooManyPipes = false;

          for (int i = 0; i < cmds && !tooManyPipes; i++) {
            if (args[i][0] != '|') {
              switch(pipeNum) {
                case 1:
                  pipeargs1[index] = args[i];
                  break;
                case 2:
                  pipeargs2[index] = args[i];
                  break;
                case 3:
                  pipeargs3[index] = args[i];
                  break;
                case 4:
                  pipeargs4[index] = args[i];
                  break;
                case 5:
                  pipeargs5[index] = args[i];
                  break;
                case 6:
                  pipeargs6[index] = args[i];
                  break;
                case 7:
                  pipeargs7[index] = args[i];
                  break;
              }
              index++;
            } else { // triggered by pipe character
              switch(pipeNum) {
                case 1:
                  pipeargs1[index] = NULL;
                  break;
                case 2:
                  pipeargs2[index] = NULL;
                  break;
                case 3:
                  pipeargs3[index] = NULL;
                  break;
                case 4:
                  pipeargs4[index] = NULL;
                  break;
                case 5:
                  pipeargs5[index] = NULL;
                  break;
                case 6:
                  pipeargs6[index] = NULL;
                  break;
                default: // more than 6 pipes or 7 connected arguments (not supported)
                  tooManyPipes = true;
              }
              pipeNum++;
              index = 0;
            }
          }

          if (tooManyPipes) {
            printf("ERROR: Could not execute command\n");
            continue;
          }
          // sets last pipeargs last element to NULL and execute args
          struct pipeCommand line[pipeNum];
          switch (pipeNum) {
            case 2:
              pipeargs2[index] = NULL;
              line[0].arg = pipeargs1;
              line[1].arg = pipeargs2;
              executePipes(2, line);
              break;
            case 3:
              pipeargs3[index] = NULL;
              line[0].arg = pipeargs1;
              line[1].arg = pipeargs2;
              line[2].arg = pipeargs3;
              executePipes(3, line);
              break;
            case 4:
              pipeargs4[index] = NULL;
              line[0].arg = pipeargs1;
              line[1].arg = pipeargs2;
              line[2].arg = pipeargs3;
              line[3].arg = pipeargs4;
              executePipes(4, line);
              break;
            case 5:
              pipeargs5[index] = NULL;
              line[0].arg = pipeargs1;
              line[1].arg = pipeargs2;
              line[2].arg = pipeargs3;
              line[3].arg = pipeargs4;
              line[4].arg = pipeargs5;
              executePipes(5, line);
              break;
            case 6:
              pipeargs6[index] = NULL;
              line[0].arg = pipeargs1;
              line[1].arg = pipeargs2;
              line[2].arg = pipeargs3;
              line[3].arg = pipeargs4;
              line[4].arg = pipeargs5;
              line[5].arg = pipeargs6;
              executePipes(6, line);
              break;
            case 7:
              pipeargs7[index] = NULL;
              line[0].arg = pipeargs1;
              line[1].arg = pipeargs2;
              line[2].arg = pipeargs3;
              line[3].arg = pipeargs4;
              line[4].arg = pipeargs5;
              line[5].arg = pipeargs6;
              line[6].arg = pipeargs7;
              executePipes(7, line);
              break;
          }
        }
      } else {
        // if fgets returns null (from ctrl + d)
        run = false;
        printf("\n");
      }
  }
  return 0;
}
