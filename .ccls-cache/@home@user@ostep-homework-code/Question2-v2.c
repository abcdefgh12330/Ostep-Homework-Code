// fork_write.c
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static void die(const char* msg) {
    perror(msg);
    exit(1);
}

// Ghi hết buffer (xử lý trường hợp write() ghi thiếu)
static void write_all(int fd, const char* buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            die("write");
        }
        off += (size_t)n;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s <path> [append] [reopen]\n"
            "  append : mở file với O_APPEND (ghi kiểu log, mỗi write sẽ tự nhảy cuối file)\n"
            "  reopen : tiến trình con tự open() lại file (không dùng chung open-file description)\n",
            argv[0]);
        return 2;
    }

    const char* path = argv[1];
    int use_append = 0;
    int child_reopen = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "append") == 0) use_append = 1;
        else if (strcmp(argv[i], "reopen") == 0) child_reopen = 1;
        else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 2;
        }
    }

    int flags = O_WRONLY | O_CREAT | O_TRUNC | (use_append ? O_APPEND : 0);
    int fd = open(path, flags, 0644);
    if (fd < 0) die("open");

    pid_t pid = fork();
    if (pid < 0) die("fork");

    // Để trộn thứ tự ghi dễ thấy hơn
    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    if (pid == 0) {
        // Child
        if (child_reopen) {
            // Đóng fd thừa và open lại (sẽ có open-file description riêng)
            close(fd);
            int flags2 = O_WRONLY | O_CREAT | (use_append ? O_APPEND : 0);
            fd = open(path, flags2, 0644);
            if (fd < 0) die("child open");
        }

        for (int i = 0; i < 100; i++) {
            char buf[128];
            int len = snprintf(buf, sizeof(buf), "CHILD  pid=%d  i=%03d\n", (int)getpid(), i);
            write_all(fd, buf, (size_t)len);
            // ngủ ngẫu nhiên chút để tạo tranh chấp
            sleep((useconds_t)(10 * (rand() % 5)));
        }
        close(fd);
        _exit(0);
    } else {
        // Parent
        for (int i = 0; i < 100; i++) {
            char buf[128];
            int len = snprintf(buf, sizeof(buf), "PARENT pid=%d  i=%03d\n", (int)getpid(), i);
            write_all(fd, buf, (size_t)len);
            sleep((useconds_t)(10 * (rand() % 5)));
        }
        close(fd);
        int status = 0;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
}