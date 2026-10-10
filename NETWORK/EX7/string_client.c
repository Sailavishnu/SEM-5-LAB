
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082

int main() {
    int sockfd, choice, n;
    char s1[500], s2[500], req[1024], res[1024];
    struct sockaddr_in server;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        printf("\n1. Concatenation\n");
        printf("2. Reverse\n");
        printf("3. Lowercase\n");
        printf("4. Uppercase\n");
        printf("5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            close(sockfd);
            return 1;
        }
        getchar();

        if (choice == 5)
            break;

        if (choice < 1 || choice > 4) {
            printf("Invalid choice.\n");
            continue;
        }

        printf("Enter string: ");
        if (fgets(s1, sizeof(s1), stdin) == NULL)
            break;
        s1[strcspn(s1, "\n")] = '\0';

        s2[0] = '\0';

        if (choice == 1) {
            printf("Enter second string: ");
            if (fgets(s2, sizeof(s2), stdin) == NULL)
                break;
            s2[strcspn(s2, "\n")] = '\0';
        }

        snprintf(req, sizeof(req), "%d|%s|%s",
                 choice, s1, s2);

        if (sendto(sockfd, req, strlen(req), 0,
                   (struct sockaddr *)&server,
                   sizeof(server)) < 0) {
            perror("Send error");
            continue;
        }

        n = recvfrom(sockfd, res, sizeof(res) - 1, 0,
                     NULL, NULL);

        if (n < 0) {
            perror("Receive error");
            continue;
        }

        res[n] = '\0';
        printf("Result: %s\n", res);
    }

    close(sockfd);
    return 0;
}
