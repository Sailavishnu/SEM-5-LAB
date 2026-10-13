#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>

#define PORT 8082
#define SIZE 1024

int sockfd;
struct sockaddr_in server;
socklen_t len;

void cleanup(int sig) {
    char msg[] = "-1";
    sendto(sockfd, msg, strlen(msg), 0,
           (struct sockaddr *)&server, sizeof(server));
    printf("\nDisconnecting from server...\n");
    close(sockfd);
    exit(0);
}

int main() {
    int n;
    char domain[SIZE], ip[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    len = sizeof(server);

    signal(SIGINT, cleanup);

    /* ---- STEP 1: Send CONNECT request ---- */
    char *connect_msg = "CONNECT";
    sendto(sockfd, connect_msg, strlen(connect_msg), 0,
           (struct sockaddr *)&server, sizeof(server));

    /* ---- STEP 2: Wait for server response ---- */
    while (1) {
        n = recvfrom(sockfd, ip, SIZE - 1, 0,
                     (struct sockaddr *)&server, &len);
        if (n < 0) {
            perror("recvfrom");
            close(sockfd);
            return 1;
        }
        ip[n] = '\0';

        if (strncmp(ip, "CONNECTED", 9) == 0) {
            printf("Connected to server.\n");
            break;
        }

        if (strncmp(ip, "WAIT:", 5) == 0) {
            printf("%s\n", ip + 6);
            printf("Waiting for server... (do not close)\n");
            /* Keep waiting for next message from server */
            continue;
        }

        if (strncmp(ip, "QUEUE_FULL:", 11) == 0) {
            printf("%s\n", ip + 12);
            close(sockfd);
            return 1;
        }

        printf("Unexpected response: %s\n", ip);
    }

    /* ---- STEP 3: Now connected, ask for domain names ---- */
    while (1) {
        printf("Enter domain name (or -1 to exit): ");
        fflush(stdout);
        if (fgets(domain, SIZE, stdin) == NULL) {
            cleanup(0);
        }
        domain[strcspn(domain, "\n")] = '\0';

        if (strcmp(domain, "-1") == 0) {
            cleanup(0);
        }

        sendto(sockfd, domain, strlen(domain), 0,
               (struct sockaddr *)&server, sizeof(server));

        n = recvfrom(sockfd, ip, SIZE - 1, 0,
                     (struct sockaddr *)&server, &len);
        if (n < 0) {
            perror("recvfrom");
            close(sockfd);
            return 1;
        }
        ip[n] = '\0';

        printf("IP Address: %s\n", ip);
    }

    close(sockfd);
    return 0;
}
