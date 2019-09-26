#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int fd[2];
    char buf[] = "ls";
    char* fargv[32];
    char* fargv1[32];

    if(pipe(fd)){
      perror("pipe");
      return -1;
    }
    switch(fork()){
        case -1:
            perror("fork");
            return -1;
        case 0:
            // child second arg
            fargv[0] = "sort";
            fargv[2] = NULL;
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            execvp(fargv[0], fargv);
            exit(0);
            // execl("./log", NULL);
        default:
            // parent first arg
            fargv1[0] = "cat";
            fargv1[1] = "nums.txt";
            fargv1[2] = NULL;
            close(fd[0]);
            dup2(fd[1], 1);
            // write(fd[1], buf, sizeof(buf));
            close(fd[1]);
            execvp(fargv1[0], fargv1);
            wait(NULL);
    }
    printf("END~\n");
    return 0;
}
