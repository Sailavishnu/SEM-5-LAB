
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

int main() {
    int sockfd, n;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char message[SIZE], reply[SIZE];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        // Send message
        printf("Client: ");
        fgets(message, SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';

        sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        if (strcmp(message, "exit") == 0)
            break;

        // Receive reply
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
