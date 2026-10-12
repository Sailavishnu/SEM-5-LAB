#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

int main(void)
{
    int s, n;
    char b[SIZE];
    struct sockaddr_in addr = {0}, client = {0};
    socklen_t len = sizeof(client);
    pid_t pid;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return 1;
    }

    printf("UDP SERVER WAITING ON PORT %d...\n", PORT);
    fflush(stdout);

    n = recvfrom(s, b, SIZE - 1, 0,
                 (struct sockaddr *)&client, &len);

    if (n <= 0) {
        close(s);
        return 1;
    }

    b[n] = '\0';

    if (strcmp(b, "HELLO") != 0) {
        printf("Invalid connection request\n");
        close(s);
        return 1;
    }

    sendto(s, "CONNECTED", 9, 0,
           (struct sockaddr *)&client, len);

    printf("CLIENT CONNECTED\n");
    printf("Type EXIT to quit.\n\n");

    pid = fork();

    if (pid < 0) {
        perror("fork");
        close(s);
        return 1;
    }

    if (pid == 0) {
        while (1) {
            struct sockaddr_in from;
            socklen_t from_len = sizeof(from);

            n = recvfrom(s, b, SIZE - 1, 0,
                         (struct sockaddr *)&from, &from_len);
            if (n <= 0) break;

            b[n] = '\0';

            if (from.sin_addr.s_addr != client.sin_addr.s_addr ||
                from.sin_port != client.sin_port)
                continue;

            printf("\r\033[K");

            if (strcmp(b, "EXIT") == 0) {
                printf("Client left the chat.\n");
                kill(getppid(), SIGTERM);
                break;
            }

            printf("Client: %s\nServer> ", b);
            fflush(stdout);
        }

        close(s);
        _exit(0);
    }

    while (1) {
        printf("Server> ");
        fflush(stdout);

        if (!fgets(b, SIZE, stdin))
            break;

        b[strcspn(b, "\n")] = '\0';

        if (b[0] == '\0')
            continue;

        if (sendto(s, b, strlen(b), 0,
                   (struct sockaddr *)&client, len) < 0) {
            perror("sendto");
            break;
        }

        if (strcmp(b, "EXIT") == 0) {
            printf("You left the chat.\n");
            kill(pid, SIGTERM);
            break;
        }
    }

    close(s);
    return 0;
}