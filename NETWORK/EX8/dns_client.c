#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

int s;
struct sockaddr_in addr;
socklen_t len;

void cleanup(int sig)
{
    sendto(s, "-1", 2, 0,
           (struct sockaddr *)&addr, len);
    printf("\nDisconnected\n");
    close(s);
    exit(0);
}

int main(void)
{
    char b[SIZE];
    int n;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    len = sizeof(addr);

    signal(SIGINT, cleanup);

    sendto(s, "CONNECT", 7, 0,
           (struct sockaddr *)&addr, len);

    while (1) {
        n = recvfrom(s, b, SIZE - 1, 0, NULL, NULL);
        if (n <= 0) {
            printf("Server not responding\n");
            close(s);
            return 1;
        }
        b[n] = '\0';

        if (strcmp(b, "CONNECTED") == 0) {
            printf("Connected to server\n");
            break;
        }

        if (strcmp(b, "WAIT") == 0) {
            printf("This is an iterative server, only one client can connect at a time. Please wait...\n");
            printf("Waiting for server... (do not close)\n");
            continue;
        }

        if (strcmp(b, "QUEUE_FULL") == 0) {
            printf("Server busy. Queue full. Try again later.\n");
            close(s);
            return 1;
        }

        printf("Unexpected response: %s\n", b);
    }

    while (1) {
        printf("Enter domain name (or -1 to exit): ");
        fflush(stdout);
        if (fgets(b, SIZE, stdin) == NULL) {
            cleanup(0);
        }
        b[strcspn(b, "\n")] = '\0';

        if (strcmp(b, "-1") == 0) {
            cleanup(0);
        }

        sendto(s, b, strlen(b), 0,
               (struct sockaddr *)&addr, len);

        n = recvfrom(s, b, SIZE - 1, 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom");
            close(s);
            return 1;
        }
        b[n] = '\0';

        printf("IP Address: %s\n", b);
    }

    close(s);
    return 0;
}
