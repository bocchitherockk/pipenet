#ifndef SHARED_H
#define SHARED_H

#define FIFO_QUESTION "fifo_question"
#define FIFO_ANSWER "fifo_answer"

#define QUESTIONS_SEED 10
#define ANSWER_SEED 1000

extern int fifo_question_fd; // FIFO for receiving data(server) / sending data (client)
extern int fifo_answer_fd; // FIFO for receiving data(client) / sending data (server)


typedef struct Question {
    int client_pid;
    size_t question;
} Question;

typedef struct Answer {
    int server_pid;
    size_t count;
    int *data;
} Answer;

void Answer_destroy(Answer *answer);
void SIGUSR1_handler(int signum);


#endif // SHARED_H