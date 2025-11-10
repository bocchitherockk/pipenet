#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <pthread.h>

typedef enum Process_Type {
    PROCESS_TYPE_SERVER = 1,
    PROCESS_TYPE_CLIENT = 2,
} Process_Type;

typedef enum Process_State {
    PROCESS_STATE_STOPPED   = 1,
    PROCESS_STATE_EXECUTING = 2,
    PROCESS_STATE_ENDED     = 3,
} Process_State;


typedef struct Process {
    int pid;
    bool is_set; // this is for the server
    Process_Type process_type;
    Process_State process_state;
    int write_pipe[2];
    int read_pipe[2];
    FILE *to_gdb;
    char source_file[100];
    char *source_code; // heap
    char *message; // heap
    char line_number[10];
    char line_content[1024];
    char question[100];
    char answer[100];
} Process;


typedef struct Processes {
    Process *items;
    size_t count;
    size_t capacity;
} Processes;

extern Process server;
extern Processes clients;

void Process_create(Process_Type process_type);
void Process_next(Process *process);

#endif // PROCESS_H