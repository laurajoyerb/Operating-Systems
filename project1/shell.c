#include <stdio.h>
#include <stdbool.h>
#include <libc.h>

int main(int argc, char* argv[]) {
  bool print = true;
  if (argc > 1) {
    print = (strcmp(argv[1],"-n") != 0);
  }
  while (1) {
    char input[1000];
    if (print) { printf("my_shell$ "); }
    fgets(input, 1000, stdin);
    printf("%s", input);
  }
  return 0;
}
