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
    struct sockaddr_in server_addr, client_addr;
    char buffer[SIZE];
    socklen_t addr_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Server waiting on port %d...\n", PORT);

    memset(buffer, 0, SIZE);
    int n = recvfrom(sockfd, buffer, SIZE - 1, 0,
                     (struct sockaddr *)&client_addr, &addr_len);
    if (n < 0) {
        perror("recvfrom");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    printf("Client connected\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        while (1) {
            memset(buffer, 0, SIZE);
            int n = recvfrom(sockfd, buffer, SIZE - 1, 0,
                             (struct sockaddr *)&client_addr, &addr_len);
            if (n < 0) {
                perror("recvfrom");
                continue;
            }
            buffer[n] = '\0';

            if (strcmp(buffer, "exit") == 0) {
                printf("\nClient left the chat.\n");
                break;
            }

            printf("\nClient: %s\n", buffer);
            fflush(stdout);
        }
    } else {
        while (1) {
            memset(buffer, 0, SIZE);
            printf("Server: ");
            fflush(stdout);

            if (fgets(buffer, SIZE, stdin) == NULL)
                break;

            buffer[strcspn(buffer, "\n")] = '\0';

            sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&client_addr, addr_len);

            if (strcmp(buffer, "exit") == 0) {
                printf("You left the chat.\n");
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
