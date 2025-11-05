#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_CLIENTS 10
int current_client = 0;


int main(void) {
    // printf("main: %d\n", getpid());

    int server = fork();
    if (server == -1) {
        perror("fork");
        exit(1);
    } else if (server == 0) {
        if (execl("./server/server_main", "server_main", NULL) == -1) {
            perror("execl");
            exit(1);
        }
    }
    // printf("server: %d\n", server);

    int client;
    while((client = fork()) != 0 && current_client++ < MAX_CLIENTS); //printf("client: %d\n", client);
    if (client == -1) {
        perror("fork");
        exit(1);
    } else if (client == 0) {
        if (execl("./client/client_main", "client_main", NULL) == -1) {
            perror("execl");
            exit(1);
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (wait(NULL) == -1) {
            perror("waitpid");
            exit(1);        
        }
    }

    if (kill(server, SIGKILL) == -1) {
        perror("kill");
        exit(1);
    }
    return 0;
}