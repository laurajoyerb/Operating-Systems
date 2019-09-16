#include <stdio.h>
#include <stdbool.h>
#include <libc.h>

#define MAX_INPUT 1024
#define MAX_TOKENS 200

int main(int argc, char* argv[]) {
  bool print = true;
  if (argc > 1) {
    print = (strcmp(argv[1],"-n") != 0);
  }
  int tokenCount = 0;
  char input[MAX_INPUT];
  char* tokens[MAX_TOKENS];
  char* ptr = NULL;

  while (1) {
    if (print) { printf("my_shell$ "); }
    fgets(input, MAX_INPUT, stdin);
    tokenCount = 0;
    ptr = strtok(input, " ");
    while (ptr != NULL) {
      tokens[tokenCount] = ptr;
      tokenCount++;
      ptr = strtok(NULL, " ");
    }

    for (int i = 0; i < tokenCount; i++) {
      printf("%s\n", tokens[i]);
    }
  }
  return 0;
}
