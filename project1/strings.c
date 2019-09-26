#include <stdio.h>
#include <stdbool.h>
#include <libc.h>
#include <unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>

#define MAX_CONSOLE_INPUT 512
#define MAX_CONSOLE_TOKENS 32

// char*** all_args;
//
// char *args[MAX_CONSOLE_TOKENS];
// // counts "separate commands" in args_array
// int args_index = 0;
//
// // each index is a separate command (ie, cat numbers.txt | sort would result in args_array[0] = "cat numbers.txt", args_array[1] = "sort")
// char** args_array[MAX_CONSOLE_TOKENS];
// char* out_file;
//
//
// void parser(char* input, char* delim) {
//   int commands = 0;
//   // tells if next command is an input/output file instead of an argument
//   bool next_out = false;
//
//   int args_index = 0;
//   int pipes_index = 0;
//
//   char **iter = all_args[pipes_index];
//
//   char *ptr = strtok(input, delim); // returns pointer to the next token
//   while (ptr != NULL) // as soon as ptr is null, we have reached the end of the line
//   {
//       if (next_out) { // grabs output file redirect name
//         out_file = ptr;
//         next_out = false;
//       } else if (ptr[0] == '>') { // flags next string as output file name and skips adding char to args
//         next_out = true;
//       } else if (ptr[0] == '|') {
//         // args_array[args_index] = args;
//         // *iter -= commands;
//         // pipes_index++;
//         // args_index++;
//         pipes_index++;
//         **iter = **all_args[pipes_index];
//       } else if (ptr[0] != '&' && ptr[0] != '<') { // if normal command, save to args and increment
//         *iter++ = ptr;
//         // all_args[pipes_index][args_index] = "hey";
//         // args_index++;
//       }
//       ptr = strtok(NULL, delim);
//   }
//   // printf("outta the while loop\n");
//   all_args[pipes_index][args_index] = NULL;
//   // *iter = NULL; // need a null at the end to work properly with execvp
//   // args_array[args_index] = args; // adds last set of commands to args array
//   // printf("args_array[0][0]: %s\n", args_array[0][0]);
//   // if (args_index >= 1)
//   //   printf("args_array[1][0]: %s\n", args_array[1][0]);
//   return;
// }

int main() {
  char input[10] = "123456789";
  char new[10];
  strcpy(new, input);
  input[3] = 'h';

  printf("input: %s\n", input);
  printf("new: %s\n", new);
  // parser(input, " ");
}
