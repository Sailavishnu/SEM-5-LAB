#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock, n;
    char msg[1024];
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("Socket error"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection error"); close(sock); return 1;
    }

    n = recv(sock, msg, sizeof(msg) - 1, 0);   /* 220 banner */
    if (n > 0) { msg[n] = '\0'; printf("%s", msg); }

    while (1) {
        printf("ftp> ");
        if (fgets(msg, sizeof(msg), stdin) == NULL) break;
        if (strncmp(msg, "exit", 4) == 0) break;

        if (send(sock, msg, strlen(msg), 0) < 0) {
            perror("Send error"); break;
        }

        n = recv(sock, msg, sizeof(msg) - 1, 0);
        if (n <= 0) { printf("Server disconnected.\n"); break; }
        msg[n] = '\0';
        printf("%s", msg);
    }

    close(sock);
    return 0;
}