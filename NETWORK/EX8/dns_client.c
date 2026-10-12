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
    char domain[SIZE], ip[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    do {
        printf("Enter domain name (or -1 to exit): ");
        fgets(domain, SIZE, stdin);
        domain[strcspn(domain, "\n")] = '\0';

        if (strcmp(domain, "-1") == 0) {
            break;
        }

        sendto(sockfd, domain, strlen(domain), 0,
               (struct sockaddr *)&server, sizeof(server));

        n = recvfrom(sockfd, ip, SIZE - 1, 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom");
            close(sockfd);
            return 1;
        }

        ip[n] = '\0';
        printf("IP Address: %s\n", ip);
    } while (1);

    close(sockfd);
    return 0;
}
