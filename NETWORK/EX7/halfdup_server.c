
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

int main() {
    int sockfd, n;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char message[SIZE], reply[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("UDP server waiting...\n");

    while (1) {
        client_len = sizeof(client_addr);
        n = recvfrom(sockfd, message, SIZE - 1, 0,
                     (struct sockaddr *)&client_addr, &client_len);

        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        message[n] = '\0';
        printf("Client: %s\n", message);

        if (strcmp(message, "exit") == 0)
            break;

        printf("Server reply: ");
        fgets(reply, SIZE, stdin);
        reply[strcspn(reply, "\n")] = '\0';

        sendto(sockfd, reply, strlen(reply), 0,
               (struct sockaddr *)&client_addr, client_len);
    }

    close(sockfd);
    return 0;
}
