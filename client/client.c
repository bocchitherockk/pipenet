#include <stdio.h> // perror
#include <stdlib.h> // exit, malloc
#include <unistd.h> // write, read, getpid, pause, close, access
#include <signal.h> // SIGUSR1, signal, kill
#include <fcntl.h> // O_RDONLY, O_WRONLY, open
#include <errno.h> // errno, ENOENT
#include <time.h>

#include "../shared.h"
#include "./client.h"

int fifo_question_fd = 0; // FIFO for sending data
int fifo_answer_fd = 0; // FIFO for receiving data


static int _open_fifo(const char *fifo_name, int flag) {
    while (access(fifo_name, flag) == -1 && errno == ENOENT); // wait until the server creates the FIFO
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

void Client_init() {
    fifo_question_fd = _open_fifo(FIFO_QUESTION, O_WRONLY);
    fifo_answer_fd = _open_fifo(FIFO_ANSWER, O_RDONLY);
    srand(time(NULL) ^ getpid());
    signal(SIGUSR1, SIGUSR1_handler);
}

void Client_destroy() {
    _close_fifo(fifo_question_fd);
    _close_fifo(fifo_answer_fd);
    fifo_question_fd = 0;
    fifo_answer_fd = 0;
}

Question Client_generate_question() {
    return (Question){
        .client_pid = getpid(),
        // note: we add 1 to avoid zero questions
        .question = (rand() % QUESTIONS_SEED) + 1,
    };
}

void Client_write_question(Question question) {
    if (write(fifo_question_fd, &question, sizeof(Question)) == -1) {
        perror("Client_write_question : write");
        exit(1);
    }
    pause();
}

Answer Client_read_answer() {
    int server_pid;
    size_t count;
    int *data;

    if (read(fifo_answer_fd, &server_pid, sizeof(server_pid)) == -1) {
        perror("Client_read_answer : read");
        exit(1);
    }

    if (read(fifo_answer_fd, &count, sizeof(count)) == -1) {
        perror("Client_read_answer : read");
        exit(1);
    }

    data = malloc(count * sizeof(int));
    if (data == NULL) {
        perror("Client_read_answer : malloc");
        exit(1);
    }

    if (read(fifo_answer_fd, data, count * sizeof(int)) == -1) {
        perror("Client_read_answer : read");
        exit(1);
    }

    kill(server_pid, SIGUSR1);
    return (Answer){
        .server_pid = server_pid,
        .count = count,
        .data = data,
    };
}