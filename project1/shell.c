#include <stdio.h>
#include <stdbool.h>
#include <libc.h>

#define MAX_CONSOLE_INPUT 512
#define MAX_CONSOLE_TOKENS 32

char *args[MAX_CONSOLE_TOKENS];

char* out_file;

int parser(char* input, char* delim) {
  // counts actual commands (ie, not input/output file redirects)
  int commands = 0;

  // tells if next command is an input/output file instead of an argument
  bool next_in = false;
  bool next_out = false;

  char **iter = args;

  char *ptr = strtok(input, delim); // returns pointer to the next token
  while (ptr != NULL) // as soon as ptr is null, we have reached the end of the line
  {
      if (next_in) { // grabs input file args
        next_in = false;
        int fin = open(ptr, O_RDONLY);
        char *filecmds = (char *) calloc(100, sizeof(char));
        printf("opened file: %d\n", fin);
        read(fin, filecmds, 256);
        printf("Those bytes are as follows: %s\n", filecmds);
        close(fin);
        char* in_ptr = strtok(filecmds, delim);
        while (in_ptr != NULL) {
          if (next_out) {
            out_file = in_ptr;
            next_out = false;
          } else if (in_ptr[0] == '>') {
            next_out = true;
          } else {
            commands++;
            *iter++ = in_ptr;
          }
          in_ptr = strtok(NULL, delim);
        }
      } else if (next_out) { // grabs output file redirect name
        out_file = ptr;
        next_out = false;
      } else if (ptr[0] == '>') { // flags next string as output file name and skips adding char to args
        next_out = true;
      } else if (ptr[0] == '<') { // flags next string as input file name and skips adding char to args
        next_in = true;
      } else if (ptr[0] != '|' && ptr[0] != '&') { // if normal command, save to args and increment
        commands++;
        *iter++ = ptr;
      }
      ptr = strtok(NULL, delim);
  }
  // printf("outta the while loop\n");
  *iter = NULL; // need a null at the end to work properly with execvp
  return commands;
}

bool valid_input(char* input) {
  int ins = 0;
  int outs = 0;
  for (int i = 0; i < MAX_CONSOLE_INPUT - 1; i++) { // to max minus 1 to avoid seg faulting on and check
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
          if (print) {printf("ERROR: Invalid input\n");}
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

        int cmds = parser(input, " \n");

        // printf("\n");
        // for (int i = 0; i < cmds; i++) {
        //   printf("%s ", args[i]);
        // }
        // printf("\n");
        // printf("commands: %d\n", cmds);

        // fork needed to not overrun the current program
        // ie, parent program is processing input and running the shell
        // the parent process creates child processes to actually execute the commands

        pid_t pid = fork();

        if (pid == 0 && cmds > 0) {
          // child
          if (file_out) {
            close(1);
            int fout = open(out_file, O_WRONLY | O_CREAT);
            dup2(fout, 1);
          }

          if (execvp(args[0], args) < 0) {
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
