#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int write_pipe[2];
int  read_pipe[2];
/*
┌───────┐                           ┌───────┐
│       │<-----[0]┌───────┐[0]      │       │
│       │         │ read  │         │       │
│       │      [1]└───────┘[1]<-----│       │
│ front │                           │  gdb  │
│       │      [0]┌───────┐[0]----->│       │
│       │         │ write │         │       │
│       │----->[1]└───────┘[1]      │       │
└───────┘                           └───────┘
*/
FILE *to_gdb = NULL;

#define FG_RED "\x1B[31m"
#define RESET  "\x1B[0m"

// send a line (MI command) to gdb via the write FILE*
void send_cmd(const char *cmd) {
    fprintf(to_gdb, "%s\n", cmd);
    fflush(to_gdb);
}

// read available data from fd until a timeout of TIMEOUT_MS without data
char *read_message() {
    const int TIMEOUT_MS = 1000;
    const int BUFFER_SIZE = 1024 * 1024; // 1048576 // 1MB
    char *result = calloc(BUFFER_SIZE, sizeof(char));

    fd_set rfds;
    size_t total_read = 0;

    // convert ms to timeval
    struct timeval tv;
    tv.tv_sec = TIMEOUT_MS / 1000;
    tv.tv_usec = (TIMEOUT_MS % 1000) * 1000;
    
    // We'll repeatedly select+read until select times out
    while (true) {
        FD_ZERO(&rfds);
        FD_SET(read_pipe[0], &rfds);
        struct timeval tv_local = tv;
        
        int rv = select(read_pipe[0]+1, &rfds, NULL, NULL, &tv_local);
        if (rv == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        } else if (rv == 0) {
            // timeout, no more data
            break;
        }
        
        if (FD_ISSET(read_pipe[0], &rfds)) {
            size_t n = read(read_pipe[0], result+total_read, BUFFER_SIZE-1);
            if (n > 0) {
                result[n + total_read] = '\0';
                total_read += n;
                continue; // continue to try to read more available data
            } else if (n == 0) {
                // EOF (child likely exited)
                break;
            } else {
                if (errno == EINTR) continue;
                perror("read");
                break;
            }
        }
    }
    return result;
}

// this function is called after sending a command that would trigger the execution of a blocking line in the execuable. for example open("fifo", O_RDONLY) would block until something opens the fifo for writing. this function will block until a complete message is received from gdb.
char *read_message_blocking() {
    const int BUFFER_SIZE = 1024 * 1024; // 1MB
    char *result = calloc(BUFFER_SIZE, sizeof(char));
    size_t total_read = 0;

    while (true) {
        size_t n = read(read_pipe[0], result + total_read, BUFFER_SIZE - 1);
        if (n > 0) {
            total_read += n;
            result[total_read] = '\0';
            // Check if we have received a complete MI message (ends with ^done or ^error)
            if (strstr(result, "*stopped")) { // means gdb has stopped at the next breakpoint (in other words, the command has completed)
                break;
            }
        } else if (n == 0) {
            // EOF
            break;
        } else {
            if (errno == EINTR) continue;
            perror("read");
            break;
        }
    }
    return result;
}

int main(void) {
    if (pipe(write_pipe) == -1) {
        perror("pipe write_pipe");
        exit(1);
    }
    if (pipe(read_pipe) == -1) {
        perror("pipe read_pipe");
        exit(1);
    }

    int gdb_process = fork();
    if (gdb_process < 0) {
        perror("fork");
        exit(1);
    }

    if (gdb_process == 0) { // Child
        if (dup2(write_pipe[0]  , STDIN_FILENO)  == -1) { perror("dup2 stdin"); exit(1);  }
        if (dup2(read_pipe[1], STDOUT_FILENO) == -1) { perror("dup2 stdout"); exit(1); }
        if (dup2(read_pipe[1], STDERR_FILENO) == -1) { perror("dup2 stderr"); exit(1); }

        // Close unused fds in child
        close(write_pipe[0]);
        close(write_pipe[1]);
        close(read_pipe[0]);
        close(read_pipe[1]);

        execlp("gdb", "gdb", "--interpreter=mi2", NULL);
        perror("execvp gdb");
        exit(127);
    }

    // Parent
    // Close unused ends
    close(write_pipe[0]);
    close(read_pipe[1]);

    // Wrap the writable end with FILE* for easy fprintf
    to_gdb = fdopen(write_pipe[1], "w");
    if (!to_gdb) {
        perror("fdopen to_gdb");
        close(write_pipe[1]);
        close(read_pipe[0]);
        return 1;
    }

    // drain gdb startup banner
    read_message();

    char *commands[] = {
        "-file-exec-and-symbols ./main",
        "-break-insert main",
        "-exec-run", // this blocks at the fist line in main
        "-data-evaluate-expression b",
        "-exec-next", // this executes the first line and stops at the second
        "-data-evaluate-expression b",
        "-exec-next",
        "-data-evaluate-expression b",
        "-exec-next",
        "-data-evaluate-expression b",
        "-exec-next",
        "-data-evaluate-expression b",
        "-exec-next", // this should hit the open function call that would block
        "-data-evaluate-expression b",
        "-exec-next", // this will execute the printf
        "-data-evaluate-expression b",
        "-exec-next", // this is the return statement
        "-data-evaluate-expression b",
        "-exec-next", // this is after the return statement
        "-exec-next", // this is after the return statement
        "-exec-next", // this is after the return statement
        "-gdb-exit",
    };
    size_t commands_count = sizeof(commands) / sizeof(commands[0]);

    for (size_t i = 0; i < commands_count; i++) {
        printf(">>> %s\n", commands[i]);
        send_cmd(commands[i]);
        char *message;
        if (i == 13-1) {
            message = read_message_blocking();
        } else {
            message = read_message();
        }
        printf(FG_RED"%s"RESET"\n", message);
        free(message);
    }

    // Cleanup
    fclose(to_gdb); // i think this closes write_pipe[1] automatically
    close(read_pipe[0]);

    // wait for child process to finish
    if (waitpid(gdb_process, NULL, 0) == -1) {
        perror("waitpid");
        exit(1);
    }
    return 0;
}
