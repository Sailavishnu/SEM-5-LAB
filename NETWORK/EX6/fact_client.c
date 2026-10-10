
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock, n, num;
    char msg[1024];
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("Connect error");
        close(sock);
        return 1;
    }

    printf("Enter a number: ");
    if (scanf("%d", &num) != 1) {
        printf("Invalid input.\n");
        close(sock);
        return 1;
    }

    snprintf(msg, sizeof(msg), "%d", num);

    if (send(sock, msg, strlen(msg), 0) < 0) {
        perror("Send error");
        close(sock);
        return 1;
    }

    n = recv(sock, msg, sizeof(msg) - 1, 0);
    if (n <= 0) {
        printf("Server disconnected.\n");
        close(sock);
        return 1;
    }

    msg[n] = '\0';
    printf("Server response: %s\n", msg);

    close(sock);
    return 0;
}
