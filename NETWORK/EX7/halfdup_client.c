
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

int main() {
    int sockfd, n;
    struct sockaddr_in server;
    socklen_t addr_len = sizeof(server);
    char message[SIZE], reply[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        printf("Client: ");
        fgets(message, SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';

        sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&server, sizeof(server));

        if (strcmp(message, "exit") == 0)
            break;

        n = recvfrom(sockfd, reply, SIZE - 1, 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom");
            break;
        }

        reply[n] = '\0';
        printf("Server: %s\n", reply);
    }

    close(sockfd);
    return 0;
}
