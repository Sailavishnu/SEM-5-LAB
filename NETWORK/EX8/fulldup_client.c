#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[SIZE];
    socklen_t addr_len = sizeof(server_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("UDP Client started. Type messages below.\n");
    printf("Type 'exit' to quit.\n\n");

    strcpy(buffer, "connected");
    sendto(sockfd, buffer, strlen(buffer), 0,
           (struct sockaddr *)&server_addr, addr_len);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        while (1) {
            memset(buffer, 0, SIZE);
            int n = recvfrom(sockfd, buffer, SIZE - 1, 0, NULL, NULL);
            if (n < 0) {
                perror("recvfrom");
                continue;
            }
            buffer[n] = '\0';

            if (strcmp(buffer, "exit") == 0) {
                printf("\nServer left the chat.\n");
                break;
            }

            printf("\nServer: %s\n", buffer);
            fflush(stdout);
        }
    } else {
        while (1) {
            memset(buffer, 0, SIZE);
            printf("Client: ");
            fflush(stdout);

            if (fgets(buffer, SIZE, stdin) == NULL)
                break;

            buffer[strcspn(buffer, "\n")] = '\0';

            sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&server_addr, addr_len);

            if (strcmp(buffer, "exit") == 0) {
                printf("You left the chat.\n");
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
