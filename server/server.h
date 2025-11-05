#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

#include "../shared.h"

void Server_init();
void Server_destroy();
Question Server_read_question();
Answer Server_generate_answer(Question question);
void Server_write_answer(Answer answer);

#endif // SERVER_H