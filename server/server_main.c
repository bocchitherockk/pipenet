#include <stdio.h> // printf
#include <stdbool.h> // true
#include <unistd.h> // getpid

#include "../shared.h"
#include "server.h"

void print_read_question(int server_pid, Question question) {
    printf("server %d: read question: {client_pid = %d, question = %zu}\n",
        server_pid,
        question.client_pid,
        question.question
    );
}

void print_generated_answer(int client_pid, Answer answer) {
    printf("server %d: answer for client %d: {server_pid = %d, count = %zu, answers: ",
        answer.server_pid,
        client_pid,
        answer.server_pid,
        answer.count
    );
    for (size_t i = 0; i < answer.count; i++) {
        printf("%d ", answer.data[i]);
    }
    printf("}\n");
}

Question question = {0};
Answer answer = {0};

int main(void) {
    Server_init();

    while (true) {
        question = Server_read_question();
        print_read_question(getpid(), question);
        answer = Server_generate_answer(question);
        print_generated_answer(question.client_pid, answer);
        Server_write_answer(answer, question.client_pid);
        Answer_destroy(&answer);
    }

    Server_destroy();
   return 0;
}