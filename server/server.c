#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // write, read, unlink
#include <signal.h> // signal, SIGUSR1
#include <sys/stat.h> // mkfifo
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_NONBLOCK
#include <errno.h> // errno
#include <time.h>

#include "../shared.h"
#include "./server.h"

int fifo_question_fd = 0; // FIFO for reading questions
int fifo_answer_fd = 0; // FIFO for writing answers

static void _create_fifo(const char *fifo_name) {
    if (mkfifo(fifo_name, 0666) == -1) {
        if (errno != EEXIST) {
            perror("_create_fifo : mkfifo");
            exit(1);
        }
    }
}

static void _delete_fifo(const char *fifo_name) {
    if (unlink(fifo_name) == -1) {
        perror("_delete_fifo : unlink");
        exit(1);
    }
}

static int _open_fifo(const char *fifo_name, int flag) {
    int fd = open(fifo_name, flag);
    if (fd == -1) {
        perror("_open_fifo : open");
        exit(1);
    }
    return fd;
}

static void _close_fifo(int fd) {
    if (close(fd) == -1) {
        perror("_close_fifo : close");
        exit(1);
    }
}

static void _other_signals_handler(int signum) {
    (void)signum;
    // printf("got the signal %d\n", signum);
    exit(1);
}

static void _override_signals() {
    for (int i = 1; i < _NSIG; i++) {
        if (i == SIGKILL)  continue;
        if (i == SIGSTOP)  continue;
        if (i == 32)       continue;
        if (i == 33)       continue;
        if (i == SIGWINCH) continue;
        signal(i, _other_signals_handler);
    }
    signal(SIGUSR1, SIGUSR1_handler);
}

void Server_init() {
    printf("Server running ...\n");
    _create_fifo(FIFO_QUESTION); // the server will be receiving from fifo_question
    _create_fifo(FIFO_ANSWER); // the server will be sending to fifo_answer

    // fifo_question_fd = _open_fifo(FIFO_QUESTION, O_RDONLY);
    fifo_question_fd = _open_fifo(FIFO_QUESTION, O_RDWR); // Open in read-write to make myself as a fake writer to avoid EOF when no clients are connected
    fifo_answer_fd = _open_fifo(FIFO_ANSWER, O_WRONLY);
    // fifo_answer_fd = _open_fifo(FIFO_ANSWER, O_RDWR); // Open in read-write to make myself as a fake reader to avoid SIGPIPE when no clients are connected (in reality this doesn't happen because the server writes only after reading a question from a client)

    _override_signals();
    srand(time(NULL) ^ getpid());
}

void Server_destroy() {
    _close_fifo(fifo_question_fd);
    _close_fifo(fifo_answer_fd);
    fifo_question_fd = 0;
    fifo_answer_fd = 0;
    _delete_fifo(FIFO_QUESTION);
    _delete_fifo(FIFO_ANSWER);
    printf("Server stopped\n");
}

Question Server_read_question() {
    Question question;
    if (read(fifo_question_fd, &question, sizeof(Question)) == -1) {
        perror("Server_read_question : read");
        exit(1);
    }
    return question;
}

Answer Server_generate_answer(Question question) {
    int *data = malloc(question.question * sizeof(int));
    if (data == NULL) {
        perror("Server_generate_answer : malloc");
        exit(1);
    }
    for (size_t i = 0; i < question.question; i++) {
        data[i] = rand() % ANSWER_SEED;
    }

    return (Answer){
        .server_pid = getpid(),
        .count = question.question,
        .data = data,
    };
}

 // the client_pid is just used to send the SIGUSR1 to it, i did this to eliminate the problem of gui step debugging process
void Server_write_answer(Answer answer, int client_pid) {
    if (write(fifo_answer_fd, &answer.server_pid, sizeof(answer.server_pid)) == -1) {
        perror("Server_write_answer : write");
        exit(1);
    }
    if (write(fifo_answer_fd, &answer.count, sizeof(answer.count)) == -1) {
        perror("Server_write_answer : write");
        exit(1);
    }
    if (write(fifo_answer_fd, answer.data, sizeof(*answer.data) * answer.count) == -1) {
        perror("Server_write_answer : write");
        exit(1);
    }

    kill(client_pid, SIGUSR1);
    pause();
}
