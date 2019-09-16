#include <stdio.h>

int main() {
  while (1) {
    char input[1000];
    printf("my_shell$ ");
    fgets(input, 1000, stdin);
    printf("%s", input);
  }
  return 0;
}
