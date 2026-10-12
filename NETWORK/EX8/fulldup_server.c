#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8082
#define SIZE 1024

int main(void)
{
    int s, n, running = 1;
    char b[SIZE];
    struct sockaddr_in addr = {0}, client = {0};
    socklen_t len = sizeof(client);
    fd_set readfds;

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

    printf("UDP Server waiting on port %d...\n", PORT);
    fflush(stdout);

    n = recvfrom(s, b, SIZE - 1, 0,
                 (struct sockaddr *)&client, &len);

    if (n <= 0) {
        perror("recvfrom");
        close(s);
        return 1;
    }

    b[n] = '\0';

    if (strcmp(b, "HELLO") != 0) {
        printf("Invalid connection request.\n");
        close(s);
        return 1;
    }

    sendto(s, "CONNECTED", 9, 0,
           (struct sockaddr *)&client, len);

    printf("Client connected. Type EXIT to quit.\n\n");
    printf("Server> ");
    fflush(stdout);

    while (running) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(s, &readfds);

        if (select(s + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(s, &readfds)) {
            struct sockaddr_in from = {0};
            socklen_t from_len = sizeof(from);

            n = recvfrom(s, b, SIZE - 1, 0,
                         (struct sockaddr *)&from, &from_len);

            if (n <= 0)
                continue;

            b[n] = '\0';

            if (from.sin_addr.s_addr != client.sin_addr.s_addr ||
                from.sin_port != client.sin_port)
                continue;

            printf("\r\033[K");

            if (!strcmp(b, "EXIT")) {
                printf("Client left the chat.\n");
                running = 0;
            } else {
                printf("Client: %s\nServer> ", b);
                fflush(stdout);
            }
        }

        if (running && FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(b, sizeof(b), stdin))
                strcpy(b, "EXIT");
            else
                b[strcspn(b, "\n")] = '\0';

            if (b[0] == '\0') {
                printf("Server> ");
                fflush(stdout);
                continue;
            }

            if (sendto(s, b, strlen(b), 0,
                       (struct sockaddr *)&client, len) < 0) {
                perror("sendto");
                break;
            }

            if (!strcmp(b, "EXIT")) {
                printf("You left the chat.\n");
                running = 0;
            } else {
                printf("Server> ");
                fflush(stdout);
            }
        }
    }

    close(s);
    return 0;
}