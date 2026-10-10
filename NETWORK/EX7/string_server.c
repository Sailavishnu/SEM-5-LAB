
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082

void process_request(char *req, char *res) {
    int choice, i, len;
    char s1[1024] = "", s2[1024] = "";
    char *p1, *p2;

    choice = atoi(req);
    p1 = strchr(req, '|');

    if (p1 == NULL) {
        strcpy(res, "Invalid request");
        return;
    }

    p1++;
    p2 = strchr(p1, '|');

    if (p2 != NULL) {
        *p2 = '\0';
        strcpy(s2, p2 + 1);
    }

    strcpy(s1, p1);

    switch (choice) {
        case 1:
            strcpy(res, s1);
            strcat(res, s2);
            break;

        case 2:
            len = strlen(s1);
            for (i = 0; i < len; i++)
                res[i] = s1[len - 1 - i];
            res[len] = '\0';
            break;

        case 3:
            for (i = 0; s1[i] != '\0'; i++)
                res[i] = tolower((unsigned char)s1[i]);
            res[i] = '\0';
            break;

        case 4:
            for (i = 0; s1[i] != '\0'; i++)
                res[i] = toupper((unsigned char)s1[i]);
            res[i] = '\0';
            break;

        default:
            strcpy(res, "Invalid choice");
    }
}

int main() {
    int sockfd, n;
    char req[1024], res[1024];
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("Bind error");
        close(sockfd);
        return 1;
    }

    printf("UDP server waiting...\n");

    while (1) {
        n = recvfrom(sockfd, req, sizeof(req) - 1, 0,
                     (struct sockaddr *)&client, &len);

        if (n < 0) {
            perror("Receive error");
            continue;
        }

        req[n] = '\0';
        printf("Request: %s\n", req);

        process_request(req, res);

        if (sendto(sockfd, res, strlen(res), 0,
                   (struct sockaddr *)&client, len) < 0)
            perror("Send error");
    }

    close(sockfd);
    return 0;
}
