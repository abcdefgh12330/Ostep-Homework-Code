#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    int fd[2];
    char buffer[100];

    pipe(fd);

    if (fork() == 0) {
        // Child 1: writer
        close(fd[0]);               // close read end
        dup2(fd[1], STDOUT_FILENO); // stdout -> pipe
        close(fd[1]);

        printf("Hello from child 1!\n");
        fflush(stdout); // make sure data is sent
        exit(0);
    } else {
        // Parent reads
        close(fd[1]); // close write end
        read(fd[0], buffer, sizeof(buffer));
        printf("Parent got: %s", buffer);
        close(fd[0]);
    }

    return 0;
}