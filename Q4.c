// run_ls_exec_variants.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

// Try to detect platforms that have execvP()
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define HAS_EXECVP_CAPITAL 1
#endif

int run_variant(const char* which) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        // ---- CHILD: replace self with /bin/ls ----
        if (strcmp(which, "execl") == 0) {
            execl("/bin/ls", "ls", "-l", (char*)NULL);
        } else if (strcmp(which, "execle") == 0) {
            // Provide a custom environment instead of inheriting
            char *const envp[] = { "LANG=C", "DEMO_ENV=1", NULL };
            execle("/bin/ls", "ls", "-l", (char*)NULL, envp);
        } else if (strcmp(which, "execlp") == 0) {
            // Search PATH for "ls"
            execlp("ls", "ls", "-l", (char*)NULL);
        } else if (strcmp(which, "execv") == 0) {
            char *const argv_ls[] = { "ls", "-l", NULL };
            execv("/bin/ls", argv_ls);
        } else if (strcmp(which, "execvp") == 0) {
            char *const argv_ls[] = { "ls", "-l", NULL };
            execvp("ls", argv_ls);
        } else if (strcmp(which, "execvP") == 0) {
            char *const argv_ls[] = { "ls", "-l", NULL };
        #ifdef HAS_EXECVP_CAPITAL
            // Search using an explicit search path (not the environment PATH)
            execvP("ls", "/bin:/usr/bin", argv_ls);
        #else
            // Portable fallback when execvP() is unavailable:
            // emulate with PATH + execvp()
            setenv("PATH", "/bin:/usr/bin", 1);
            execvp("ls", argv_ls);
        #endif
        } else {
            fprintf(stderr, "Unknown variant: %s\n", which);
            _exit(2);
        }

        // If we get here, exec* failed
        perror("exec* failed");
        _exit(127);
    }

    // ---- PARENT: wait and report ----
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return 1; }
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code != 0) fprintf(stderr, "Child exited with %d\n", code);
        return code;
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "Child killed by signal %d\n", WTERMSIG(status));
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr,
            "Usage: %s <execl|execle|execlp|execv|execvp|execvP>\n", argv[0]);
        return 2;
    }
    return run_variant(argv[1]);
}