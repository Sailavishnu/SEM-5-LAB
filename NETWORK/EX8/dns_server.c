#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

int main() {
    int sockfd, n;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char domain[SIZE], ip[SIZE];

    char *names[] = {"google.com", "yahoo.com", "example.com", "github.com", "facebook.com", "twitter.com"};
    char *ips[] = {"142.250.0.1", "98.137.11.163", "93.184.216.34", "140.82.121.4", "157.240.0.35", "104.244.42.1"};

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&server, sizeof(server));

    printf("DNS Server waiting...\n");

    while (1) {
        n = recvfrom(sockfd, domain, SIZE - 1, 0,
                     (struct sockaddr *)&client, &len);
        if (n < 0)
            continue;

        domain[n] = '\0';
        printf("Requested domain: %s\n", domain);

        strcpy(ip, "Domain not found");

        for (int i = 0; i < 6; i++) {
            if (strcmp(domain, names[i]) == 0) {
                strcpy(ip, ips[i]);
                break;
            }
        }

        sendto(sockfd, ip, strlen(ip), 0,
               (struct sockaddr *)&client, len);
    }

    close(sockfd);
    return 0;
}
