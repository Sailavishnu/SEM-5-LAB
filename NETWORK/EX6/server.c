#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFSIZE 1024

/* socket, bind, listen, accept -- returns the connected client socket */
int connectTCP(int *server_fd) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd, opt = 1;

    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0) { perror("socket"); exit(1); }
    setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(*server_fd, 5) < 0) { perror("listen"); exit(1); }
    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) { perror("accept"); exit(1); }
    printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

    return client_fd;
}

unsigned long long factorial(int n) {
    unsigned long long f = 1;
    for (int i = 2; i <= n; i++) f *= i;
    return f;
}

void handleEcho(int client_fd, const char *msg) {
    char response[BUFSIZE];
    printf("[Echo] Received from client: %s\n", msg);
    snprintf(response, BUFSIZE, "%s", msg);
    send(client_fd, response, strlen(response), 0);
}

void handleFactorial(int client_fd, const char *msg) {
    char response[BUFSIZE];
    int num = atoi(msg);
    printf("[Factorial] Received number: %d\n", num);

    if (num < 0)
        snprintf(response, BUFSIZE, "Factorial not defined for negative numbers");
    else if (num > 20)
        snprintf(response, BUFSIZE, "Number too large (max 20)");
    else
        snprintf(response, BUFSIZE, "Factorial of %d = %llu", num, factorial(num));

    send(client_fd, response, strlen(response), 0);
}

int main() {
    int server_fd, client_fd, n, choice;
    char buffer[BUFSIZE];
    const char *payload;

    client_fd = connectTCP(&server_fd);

    while (1) {
        n = recv(client_fd, buffer, BUFSIZE - 1, 0);
        if (n <= 0) {                      /* client closed connection */
            printf("Client disconnected.\n");
            break;
        }
        buffer[n] = '\0';

        /* request format: choice|data */
        choice = atoi(buffer);
        payload = strchr(buffer, '|');
        payload = payload ? payload + 1 : "";

        switch (choice) {
            case 1:
                handleEcho(client_fd, payload);
                break;
            case 2:
                handleFactorial(client_fd, payload);
                break;
            case 3:
                printf("Client requested exit. Shutting down.\n");
                goto done;
            default:
                send(client_fd, "Invalid option", 14, 0);
        }
    }

done:
    close(client_fd);
    close(server_fd);
    return 0;
}