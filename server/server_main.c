#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h> // kill, SIGUSR1
#include <unistd.h> // pause

#include "server.h"

int main(void) {
    Server_init();

    while (true) {
        Question question = Server_read_question();
        printf(
            "server %d: read question: {client_pid = %d, question = %zu}\n",
            getpid(),
            question.client_pid,
            question.question
        );
        Answer answer = Server_generate_answer(question);
        printf(
            "server %d: answer for client %d: {server_pid = %d, count = %zu, answers: ",
            getpid(),
            question.client_pid,
            answer.server_pid,
            answer.count
        );
        for (size_t i = 0; i < answer.count; i++) {
            printf("%d ", answer.data[i]);
        }
        printf("}\n");
        Server_write_answer(answer, question.client_pid);
        Answer_destroy(&answer);
    }

    Server_destroy();
   return 0;
}