#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> // pause
#include <signal.h> // kill, SIGUSR1

#include "client.h"

int main(void) {
    Client_init();

    Question question = Client_generate_question();
    printf(
        "client %d: asked question %zu\n",
        question.client_pid,
        question.question
    );
    Client_write_question(question);
    pause();
    Answer answer = Client_read_answer();
    printf(
        "client %d: read answer: {server_pid = %d, count = %zu, data = ",
        question.client_pid,
        answer.server_pid,
        answer.count
    );
    for (size_t i = 0; i < answer.count; i++) {
        printf("%d ", answer.data[i]);
    }
    printf("}\n");
    kill(answer.server_pid, SIGUSR1);
    Answer_destroy(&answer);

    Client_destroy();
   return 0;
}