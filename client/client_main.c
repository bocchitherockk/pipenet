#include <stdio.h> // printf

#include "../shared.h"
#include "client.h"

void print_generated_question(Question question) {
    printf("client %d: asked question %zu\n",
        question.client_pid,
        question.question
    );
}

void print_read_answer(int client_pid, Answer answer) {
    printf("client %d: read answer: {server_pid = %d, count = %zu, data = ",
        client_pid,
        answer.server_pid,
        answer.count
    );
    for (size_t i = 0; i < answer.count; i++) {
        printf("%d ", answer.data[i]);
    }
    printf("}\n");
}

int main(void) {
    Client_init();

    Question question = Client_generate_question();
    print_generated_question(question);
    Client_write_question(question);
    Answer answer = Client_read_answer();
    print_read_answer(question.client_pid, answer);

    Answer_destroy(&answer);
    Client_destroy();
   return 0;
}