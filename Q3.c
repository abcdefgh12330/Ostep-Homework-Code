#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int fd[2];
    pipe(fd);  // create pipe

    int rc = fork();

    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child process
        printf("hello\n");
        close(fd[0]);           // close read end
        write(fd[1], "x", 1);   // notify parent
        close(fd[1]);
    } else {
        // parent process
        char c;
        close(fd[1]);           // close write end
        read(fd[0], &c, 1);     // wait until child writes
        printf("goodbye\n");
        close(fd[0]);
    }

    return 0;
}