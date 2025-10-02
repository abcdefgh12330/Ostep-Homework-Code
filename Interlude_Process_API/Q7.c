#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    printf("Parent: starting program (pid: %d)\n", getpid());
    fflush(stdout); // flush before fork to avoid duplicate buffer

    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // Child process
        printf("Child: closing STDOUT now...\n");
        fflush(stdout);
        close(STDOUT_FILENO);

        printf("Child: trying to print after closing STDOUT!\n");
        fflush(stdout); // this won't show anything
        _exit(0);
    } else {
        // Parent process
        printf("Parent: child pid = %d\n", rc);
    }

    return 0;
}