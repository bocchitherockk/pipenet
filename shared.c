#include <stdlib.h> // free

#include "./shared.h"

void Answer_destroy(Answer *answer) {
    if (answer->data != NULL) free(answer->data);
    answer->server_pid = 0;
    answer->count = 0;
    answer->data = NULL;
}

void SIGUSR1_handler(int signum) {
    (void)signum;
    // Do nothing
    // the server sends SIGUSR1 to the client to notify it that the answer is ready and can be read from the pipe
    // the client sends SIGUSR1 to the server to notify it that it has finished reading the answer so the server can proceed to answer the next question (this is done to avoid two clients reading their answers at the same time)
}