#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]) {
  bool print = (strcmp(argv[1],"-n") != 0);
  while (1) {
    char input[1000];
    if (print) { printf("my_shell$ "); }
    fgets(input, 1000, stdin);
    printf("%s", input);
  }
  return 0;
}
