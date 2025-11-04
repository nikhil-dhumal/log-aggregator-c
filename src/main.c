#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "config.h"
#include "http_server.h"

volatile int server_running = 1;

void interrupt_handler(int signum) {
    (void) signum;
    server_running = 0;
    printf("\nShutting down...\n");
}

int main(void)
{
    signal(SIGINT, interrupt_handler);

    if (start_server() == NULL) {
        fprintf(stderr, "ERROR: failed to start server\n");
        return 1;
    }

    printf("[server] Running at http://localhost:%s\n", SERVER_PORT);

    while (server_running) {
        sleep(1);
    }

    stop_server();
    printf("Server stopped\n");
    return 0;
}
