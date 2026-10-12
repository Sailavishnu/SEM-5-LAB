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
    struct sockaddr_in addr = {0};
    socklen_t len = sizeof(addr);
    pid_t pid;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(s, "HELLO", 5, 0,
           (struct sockaddr *)&addr, len);

    n = recvfrom(s, b, SIZE - 1, 0, NULL, NULL);
    if (n <= 0) {
        printf("Server not responding\n");
        close(s);
        return 1;
    }

    b[n] = '\0';

    if (strcmp(b, "CONNECTED") != 0) {
        printf("Connection failed\n");
        close(s);
        return 1;
    }

    printf("UDP CLIENT CONNECTED\n");
    printf("Type EXIT to quit.\n\n");

    pid = fork();

    if (pid < 0) {
        perror("fork");
        close(s);
        return 1;
    }

    if (pid == 0) {
        while (1) {
            n = recvfrom(s, b, SIZE - 1, 0, NULL, NULL);
            if (n <= 0) break;

            b[n] = '\0';

            printf("\r\033[K");

            if (strcmp(b, "EXIT") == 0) {
                printf("Server left the chat.\n");
                kill(getppid(), SIGTERM);
                break;
            }

            printf("Server: %s\nClient> ", b);
            fflush(stdout);
        }

        close(s);
        _exit(0);
    }

    while (1) {
        printf("Client> ");
        fflush(stdout);

        if (!fgets(b, SIZE, stdin))
            break;

        b[strcspn(b, "\n")] = '\0';

        if (b[0] == '\0')
            continue;

        if (sendto(s, b, strlen(b), 0,
                   (struct sockaddr *)&addr, len) < 0) {
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